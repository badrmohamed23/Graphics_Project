#pragma once

#include "Entity.h"
#include <vector>

class SpatialCollision {
public:
    static void resolve(std::vector<PlayerEntity*>& players, std::vector<EnemyEntity*>& enemies,
                        std::vector<Projectile>& projectiles, std::vector<Coin>& coins,
                        SimEvents& ev);
};
