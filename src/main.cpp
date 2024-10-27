#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include "map.hpp"
#include "unit.hpp"

struct Explosion {
    SDL_Point position;
    int currentFrame = 0;
    bool active = false;
};

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;
const int TILE_SIZE = 64;
const int EXPLOSION_FRAME_WIDTH = 3600; 
const int EXPLOSION_FRAME_HEIGHT = 140;
const int EXPLOSION_TOTAL_FRAMES = 15;  

std::vector<Explosion> explosions;

bool loadUnits(const std::string& filename, int player, SDL_Renderer* renderer, std::vector<Unit>& units);
std::vector<SDL_Point> calculateMovementRange(const Unit& unit, const Map& map, const std::vector<Unit>& units);
std::vector<SDL_Point> calculateAttackRange(const Unit& unit, const Map& map);

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
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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

    SDL_Texture* explosionTexture = IMG_LoadTexture(renderer, "resources/gfx/explosion_spritesheet.png");
    if (!explosionTexture) {
        std::cerr << "Failed to load explosion spritesheet: " << IMG_GetError() << std::endl;
        return 1;
    }

    bool running = true;
    SDL_Event event;
    Unit* selectedUnit = nullptr;
    Unit* targetEnemy = nullptr;
    std::vector<SDL_Point> movementRange;
    std::vector<SDL_Point> attackRange;
    SDL_Point destination = {-1, -1};
    bool animating = false;
    int currentPlayer = 1;  // Track player turns (1 or 2)

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x / TILE_SIZE;
                int mouseY = event.button.y / TILE_SIZE;

                if (!animating) {
                    bool validTileClicked = false;
                    targetEnemy = nullptr;

                    // Selecting a new unit of the current player
                    for (Unit& unit : units) {
                        if (unit.getX() == mouseX && unit.getY() == mouseY && unit.getPlayer() == currentPlayer && unit.isAlive()) {
                            selectedUnit = &unit;
                            movementRange = calculateMovementRange(unit, gameMap, units);
                            attackRange = calculateAttackRange(unit, gameMap);
                            validTileClicked = true;
                            break;
                        }
                    }

                    if (selectedUnit) {
                        // Check for movement (green tiles) within range
                        for (const SDL_Point& point : movementRange) {
                            if (point.x == mouseX && point.y == mouseY) {
                                destination = point;
                                animating = true;
                                validTileClicked = true;
                                break;
                            }
                        }

                        // Check for attack (yellow tiles) on an enemy
                        for (const SDL_Point& point : attackRange) {
                            if (point.x == mouseX && point.y == mouseY) {
                                for (Unit& enemyUnit : units) {
                                    if (enemyUnit.getX() == mouseX && enemyUnit.getY() == mouseY && enemyUnit.getPlayer() != currentPlayer && enemyUnit.isAlive()) {
                                        destination = {selectedUnit->getX(), selectedUnit->getY()};  // Stay in place after attacking
                                        animating = true;
                                        targetEnemy = &enemyUnit;
                                        validTileClicked = true;
                                        break;
                                    }
                                }
                            }
                            if (targetEnemy) break;
                        }
                    }

                    if (!validTileClicked) {
                        std::cout << "Invalid action: Clicked outside of movement or attack range." << std::endl;
                    }
                }
            }
        }

        // Render game
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        gameMap.render(renderer);
        for (Unit& unit : units) {
            if (unit.isAlive()) {
                unit.render(renderer, TILE_SIZE);
            }
        }   

        // Render explosion animation
        for (Explosion& explosion : explosions) {
            if (explosion.active) {
                // explosions are in one single row, so we can calculate the frame position based on the current frame
                SDL_Rect srcRect = { (explosion.currentFrame % EXPLOSION_TOTAL_FRAMES) * EXPLOSION_FRAME_WIDTH, 0, EXPLOSION_FRAME_WIDTH, EXPLOSION_FRAME_HEIGHT };
                SDL_Rect dstRect = { explosion.position.x, explosion.position.y, TILE_SIZE, TILE_SIZE };
                SDL_RenderCopy(renderer, explosionTexture, &srcRect, &dstRect);
                explosion.currentFrame++;
                SDL_Delay(500);  // Explosion animation delay

                if (explosion.currentFrame >= EXPLOSION_TOTAL_FRAMES) {
                    explosion.active = false;
                }
            }
        }

        // Display movement and attack ranges
        if (selectedUnit) {
            // Green tiles for movement range
            for (const SDL_Point& point : movementRange) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 96);
                SDL_Rect rangeRect = { point.x * TILE_SIZE, point.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &rangeRect);
            }
            // Yellow tiles for attackable enemies
            for (const SDL_Point& point : attackRange) {
                for (Unit& enemyUnit : units) {
                    if (enemyUnit.getX() == point.x && enemyUnit.getY() == point.y && enemyUnit.getPlayer() != currentPlayer && enemyUnit.isAlive()) {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 96);
                        SDL_Rect attackRect = { point.x * TILE_SIZE, point.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                        SDL_RenderFillRect(renderer, &attackRect);
                    }
                }
            }
        }

        SDL_RenderPresent(renderer);

        // Movement and attack logic with animation
        if (animating && selectedUnit) {
            animating = selectedUnit->animateMovement(destination.x, destination.y, TILE_SIZE);
            
            if (!animating) {
                if (targetEnemy && selectedUnit->inAttackRange(targetEnemy->getX(), targetEnemy->getY())) {
                    targetEnemy->takeDamage(selectedUnit->getAttackDamage());
                    std::cout << "Enemy took " << selectedUnit->getAttackDamage() << " damage!" << std::endl;
                    if (!targetEnemy->isAlive()) {
                        std::cout << "Enemy defeated!" << std::endl;

                        // Trigger explosion animation at the defeated unit's location
                        Explosion explosion = { { targetEnemy->getX() * TILE_SIZE, targetEnemy->getY() * TILE_SIZE }, 0, true };
                        explosions.push_back(explosion);
                    } else {
                        std::cout << "Enemy health: " << targetEnemy->getHealth() << std::endl;
                    }
                } else {
                    selectedUnit->setPosition(destination.x, destination.y);
                }

                // End turn, reset selection, and switch players
                selectedUnit = nullptr;
                movementRange.clear();
                attackRange.clear();
                destination = {-1, -1};
                targetEnemy = nullptr;
                currentPlayer = (currentPlayer == 1) ? 2 : 1;  // Switch turns
            }
            SDL_Delay(50);  // Animation delay
        }
    }

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

// BFS-based range calculation
std::vector<SDL_Point> calculateMovementRange(const Unit& unit, const Map& map, const std::vector<Unit>& units) {
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

                                // Check if the tile is occupied by an enemy unit
                bool occupiedByEnemy = false;
                for (const Unit& enemyUnit : units) {
                    if (enemyUnit.getX() == nx && enemyUnit.getY() == ny && enemyUnit.getPlayer() != unit.getPlayer() && enemyUnit.isAlive()) {
                        occupiedByEnemy = true;
                        break;
                    }
                }

                if (passable && !occupiedByEnemy) {
                    visited[nx][ny] = distance + 1;
                    toVisit.push({nx, ny});
                    rangePoints.push_back({nx, ny});
                }

            }
        }
    }
    return rangePoints;
}

std::vector<SDL_Point> calculateAttackRange(const Unit& unit, const Map& map) {
    int maxRange = unit.getAttackRange();
    std::vector<SDL_Point> rangePoints;
    // need to return all tiles within range of the unit, including those on diagonals
    for (int dx = -maxRange; dx <= maxRange; dx++) {
        for (int dy = -maxRange; dy <= maxRange; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = unit.getX() + dx;
            int ny = unit.getY() + dy;
            if (nx >= 0 && ny >= 0 && nx < map.getWidth() && ny < map.getHeight()) {
                rangePoints.push_back({nx, ny});
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
                spriteSheet = IMG_LoadTexture(renderer, player == 1 ? "resources/gfx/infantry_spritesheet_p1.png" : "resources/gfx/infantry_spritesheet_p2.png");
                type = INFANTRY;
                break;
            case 'T':
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
