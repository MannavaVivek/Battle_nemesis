#include <SDL.h>
#include <vector>
#include <string>
#include "tile.hpp"

class Map {
public:
    Map(int width, int height, int tileSize);
    bool loadMap(const std::string& filename, SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer) const;
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Tile getTile(int x, int y) const { return tiles[y][x]; }

private:
    int width, height;
    int tileSize;
    std::vector<std::vector<Tile>> tiles;

    SDL_Texture* loadTexture(const std::string& filePath, SDL_Renderer* renderer);
};