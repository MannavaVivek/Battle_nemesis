#include "unit.hpp"
#include <iostream>
#include <cmath>

Unit::Unit(UnitType type, int x, int y, int player, int orientation, SDL_Texture* texture)
    : type(type), x(x), y(y), player(player), orientation(orientation), texture(texture) {
    
    switch (type) {
        case INFANTRY: health = 50; attackDamage = 10; moveRange = 2; attackRange = 1; break;
        case TANK: health = 150; attackDamage = 20; moveRange = 4; attackRange = 2; break;
        case BOAT: health = 100; attackDamage = 10; moveRange = 2; attackRange = 1; break;
        case HELICOPTER: health = 100; attackDamage = 20; moveRange = 6; attackRange = 2; break;
    }
}

void Unit::render(SDL_Renderer* renderer, int tileSize) {
    SDL_Rect dstRect = { x * tileSize + (tileSize - 56) / 2, y * tileSize + (tileSize - 56) / 2, 56, 56 };
    SDL_Rect srcRect = { orientation * 56, 0, 56, 56 };
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}

void Unit::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

// Animate movement step-by-step, updating direction
bool Unit::animateMovement(int targetX, int targetY, int tileSize) {
    int deltaX = targetX - x;
    int deltaY = targetY - y;

    // Set orientation based on direction
    if (deltaX > 0) orientation = 0;      // right
    else if (deltaY > 0) orientation = 1; // down
    else if (deltaX < 0) orientation = 2; // left
    else if (deltaY < 0) orientation = 3; // up

    if (std::abs(deltaX) > 0) x += (deltaX > 0) ? 1 : -1;
    else if (std::abs(deltaY) > 0) y += (deltaY > 0) ? 1 : -1;

    return (x != targetX || y != targetY);
}

bool Unit::inAttackRange(int targetX, int targetY) const {
    return (std::abs(targetX - x) <= attackRange && std::abs(targetY - y) <= attackRange);
}