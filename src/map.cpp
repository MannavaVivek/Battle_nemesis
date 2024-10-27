#include "map.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Map::Map(int width, int height, int tileSize)
    : width(width), height(height), tileSize(tileSize) {
    tiles.resize(height, std::vector<Tile>(width));
}

SDL_Texture* Map::loadTexture(const std::string& filePath, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
    }
    return texture;
}

bool Map::loadMap(const std::string& filename, SDL_Renderer* renderer) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open map file: " << filename << std::endl;
        return false;
    }

    // Load textures
    SDL_Texture* grassTexture = loadTexture("resources/gfx/grass.png", renderer);
    SDL_Texture* waterTexture = loadTexture("resources/gfx/water.png", renderer);
    SDL_Texture* roadTexture = loadTexture("resources/gfx/road.png", renderer);
    SDL_Texture* mountainTexture = loadTexture("resources/gfx/mountain.png", renderer);

    // Parse file line by line
    std::string line;
    int y = 0;
    while (std::getline(file, line) && y < height) {
        std::istringstream stream(line);
        char tileChar;
        int x = 0;
        while (stream >> tileChar && x < width) {
            switch (tileChar) {
                case 'G': tiles[y][x] = Tile(GRASS, grassTexture); break;
                case 'W': tiles[y][x] = Tile(WATER, waterTexture); break;
                case 'R': tiles[y][x] = Tile(ROAD, roadTexture); break;
                case 'M': tiles[y][x] = Tile(MOUNTAIN, mountainTexture); break;
                default: tiles[y][x] = Tile(GRASS, grassTexture); break; // Default to grass
            }
            x++;
        }
        y++;
    }

    return true;
}

void Map::render(SDL_Renderer* renderer) const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tiles[y][x].render(renderer, x, y, tileSize);
        }
    }
}