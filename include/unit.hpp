#pragma once

#include <SDL.h>

enum UnitType { INFANTRY, TANK, BOAT, HELICOPTER };

class Unit {
public:
    Unit(UnitType type, int x, int y, int player, int orientation, SDL_Texture* texture);

    void render(SDL_Renderer* renderer, int tileSize);
    
    bool animateMovement(int targetX, int targetY, int tileSize);

    int getX() const { return x; }
    int getY() const { return y; }
    int getPlayer() const { return player; }
    int getHealth() const { return health; }
    int getDamage() const { return damage; }
    int getMoveRange() const { return moveRange; }
    int getAttackRange() const { return attackRange; }
    UnitType getType() const { return type; }

    void setPosition(int x, int y);

private:
    UnitType type;
    int x, y;
    int player;
    int orientation;
    int health;
    int damage;
    int moveRange;
    int attackRange;
    SDL_Texture* texture;
};

