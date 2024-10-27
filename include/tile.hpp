#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

enum TerrainType {
    GRASS,
    WATER,
    ROAD,
    MOUNTAIN
};

struct Tile {
    TerrainType type;
    bool passableByTank;
    bool passableByInfantry;
    bool passableByBoat;
    bool passableByHelicopter;
    SDL_Texture* texture;

    Tile(TerrainType type = GRASS, SDL_Texture* texture = nullptr);
    void render(SDL_Renderer* renderer, int x, int y, int tileSize) const;
};
