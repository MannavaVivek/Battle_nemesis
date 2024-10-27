#include "tile.hpp"

Tile::Tile(TerrainType type, SDL_Texture* texture)
    : type(type), texture(texture) {
    // Define movement properties based on terrain type
    passableByTank = (type == GRASS || type == ROAD);
    passableByInfantry = (type == GRASS || type == ROAD || type == MOUNTAIN);
    passableByBoat = (type == WATER);
    passableByHelicopter = true; // Helicopters can pass over any terrain
}

void Tile::render(SDL_Renderer* renderer, int x, int y, int tileSize) const {
    SDL_Rect destRect = { x * tileSize, y * tileSize, tileSize, tileSize };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
}
