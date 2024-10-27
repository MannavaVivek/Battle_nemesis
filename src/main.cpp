#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include "map.hpp"
#include "unit.hpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;
const int TILE_SIZE = 64;

bool loadUnits(const std::string& filename, int player, SDL_Renderer* renderer, std::vector<Unit>& units);
std::vector<SDL_Point> calculateMovementRange(const Unit& unit, const Map& map);

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Turn-Based Strategy Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Map gameMap(SCREEN_WIDTH / TILE_SIZE, SCREEN_HEIGHT / TILE_SIZE, TILE_SIZE);
    if (!gameMap.loadMap("resources/layouts/map.txt", renderer)) {
        std::cerr << "Failed to load map." << std::endl;
    }

    std::vector<Unit> units;
    if (!loadUnits("resources/layouts/player1.txt", 1, renderer, units)) {
        std::cerr << "Failed to load units for player 1." << std::endl;
    }
    if (!loadUnits("resources/layouts/player2.txt", 2, renderer, units)) {
        std::cerr << "Failed to load units for player 2." << std::endl;
    }

    bool running = true;
    SDL_Event event;
    Unit* selectedUnit = nullptr;
    std::vector<SDL_Point> movementRange;
    SDL_Point destination = {-1, -1};
    bool animating = false;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x / TILE_SIZE;
                int mouseY = event.button.y / TILE_SIZE;

                if (!animating) {
                    bool validTileClicked = false;
                    
                    // Check if a unit was clicked
                    for (Unit& unit : units) {
                        if (unit.getX() == mouseX && unit.getY() == mouseY) {
                            selectedUnit = &unit;
                            movementRange = calculateMovementRange(unit, gameMap);
                            validTileClicked = true;
                            break;
                        }
                    }
                    // Check if a movement tile was clicked within the range
                    for (const SDL_Point& point : movementRange) {
                        if (point.x == mouseX && point.y == mouseY) {
                            destination = point;
                            animating = true;
                            validTileClicked = true;
                            break;
                        }
                    }

                    if (!validTileClicked) {
                        std::cout << "Invalid move: Tile clicked is outside movement range." << std::endl;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        gameMap.render(renderer);
        for (Unit& unit : units) {
            unit.render(renderer, TILE_SIZE);
        }

        if (selectedUnit) {
            for (const SDL_Point& point : movementRange) {
                Tile tile = gameMap.getTile(point.x, point.y);
                bool isPassable = false;
                switch (selectedUnit->getType()) {
                    case TANK: isPassable = tile.passableByTank; break;
                    case INFANTRY: isPassable = tile.passableByInfantry; break;
                    case BOAT: isPassable = tile.passableByBoat; break;
                    case HELICOPTER: isPassable = tile.passableByHelicopter; break;
                }

                SDL_SetRenderDrawColor(renderer, isPassable ? 0 : 255, isPassable ? 255 : 0, 0, 128);
                SDL_Rect rangeRect = { point.x * TILE_SIZE, point.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &rangeRect);
            }
        }

        SDL_RenderPresent(renderer);

        // Handle movement animation
        if (animating && selectedUnit) {
            animating = selectedUnit->animateMovement(destination.x, destination.y, TILE_SIZE);
            if (!animating) {
                selectedUnit->setPosition(destination.x, destination.y);
                destination = {-1, -1};
                selectedUnit = nullptr;
                movementRange.clear();
            }
            SDL_Delay(50);  // Slow down animation slightly
        }
    }

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

// BFS-based range calculation
std::vector<SDL_Point> calculateMovementRange(const Unit& unit, const Map& map) {
    int maxRange = unit.getMoveRange();
    std::vector<SDL_Point> rangePoints;
    std::queue<std::pair<int, int>> toVisit;
    toVisit.push({unit.getX(), unit.getY()});
    std::vector<std::vector<int>> visited(map.getWidth(), std::vector<int>(map.getHeight(), -1));
    visited[unit.getX()][unit.getY()] = 0;

    while (!toVisit.empty()) {
        auto [cx, cy] = toVisit.front();
        toVisit.pop();
        int distance = visited[cx][cy];

        if (distance >= maxRange) continue;

        std::vector<std::pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (const auto& [dx, dy] : directions) {
            int nx = cx + dx;
            int ny = cy + dy;

            if (nx >= 0 && ny >= 0 && nx < map.getWidth() && ny < map.getHeight() && visited[nx][ny] == -1) {
                Tile tile = map.getTile(nx, ny);
                bool passable = false;
                switch (unit.getType()) {
                    case TANK: passable = tile.passableByTank; break;
                    case INFANTRY: passable = tile.passableByInfantry; break;
                    case BOAT: passable = tile.passableByBoat; break;
                    case HELICOPTER: passable = tile.passableByHelicopter; break;
                }
                if (passable) {
                    visited[nx][ny] = distance + 1;
                    toVisit.push({nx, ny});
                    rangePoints.push_back({nx, ny});
                }
            }
        }
    }
    return rangePoints;
}

bool loadUnits(const std::string& filename, int player, SDL_Renderer* renderer, std::vector<Unit>& units) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open unit file: " << filename << std::endl;
        return false;
    }

    char unitTypeChar;
    int x, y, orientation;
    SDL_Texture* spriteSheet = nullptr;

    while (file >> unitTypeChar >> x >> y >> orientation) {
        UnitType type;
        switch (unitTypeChar) {
            case 'I':
                std::cout << "Loading infantry for player " << player << std::endl;
                spriteSheet = IMG_LoadTexture(renderer, player == 1 ? "resources/gfx/infantry_spritesheet_p1.png" : "resources/gfx/infantry_spritesheet_p2.png");
                type = INFANTRY;
                break;
            case 'T':
                std::cout << "Loading tank for player " << player << std::endl;
                spriteSheet = IMG_LoadTexture(renderer, player == 1 ? "resources/gfx/tank_spritesheet_p1.png" : "resources/gfx/tank_spritesheet_p2.png");
                type = TANK;
                break;
            case 'B':
                spriteSheet = IMG_LoadTexture(renderer, player == 1 ? "resources/gfx/boat_spritesheet_p1.png" : "resources/gfx/boat_spritesheet_p2.png");
                type = BOAT;
                break;
            case 'H':
                spriteSheet = IMG_LoadTexture(renderer, player == 1 ? "resources/gfx/helicopter_spritesheet_p1.png" : "resources/gfx/helicopter_spritesheet_p2.png");
                type = HELICOPTER;
                break;
            default:
                continue;
        }

        if (!spriteSheet) {
            std::cerr << "Failed to load spritesheet for " << unitTypeChar << " for player " << player << std::endl;
            continue;
        }

        units.emplace_back(type, x, y, player, orientation, spriteSheet);
    }

    return true;
}
