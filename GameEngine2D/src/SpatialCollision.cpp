#include "SpatialCollision.h"

void SpatialCollision::resolve(std::vector<PlayerEntity*>& players, std::vector<EnemyEntity*>& enemies,
                              std::vector<Projectile>& projectiles, std::vector<Coin>& coins,
                              SimEvents& ev, bool& homeBreached) {
    for (EnemyEntity* e : enemies) {
        if (!e->alive) continue;
        e->engaged = false;
        e->strikeTarget = nullptr;
        AABB eb = e->box();
        PlayerEntity* best = nullptr;
        for (PlayerEntity* p : players) {
            if (!p->alive) continue;
            if (p->lane != e->lane) continue;
            AABB pb = p->box();
            if (!eb.overlaps(pb)) continue;
            if (!best || p->x > best->x) best = p;
        }
        if (best) {
            e->engaged = true;
            e->strikeTarget = best;
        }
        if (e->x - e->halfW <= Config::X_HOME) {
            homeBreached = true;
        }
    }

    for (PlayerEntity* p : players) {
        if (!p->alive || p->kind == PlayerKind::Defense) continue;
        if (p->kind == PlayerKind::Melee && p->cdTimer <= 0.0f) {
            AABB pb = p->box();
            EnemyEntity* target = nullptr;
            for (EnemyEntity* e : enemies) {
                if (!e->alive || e->lane != p->lane) continue;
                if (!pb.overlaps(e->box())) continue;
                if (!target || e->x < target->x) target = e;
            }
            if (target) target->takeDamage(1);
        }
    }

    for (Projectile& proj : projectiles) {
        if (!proj.alive) continue;
        if (proj.fromEnemy) {
            PlayerEntity* defShield = nullptr;
            for (PlayerEntity* p : players) {
                if (!p->alive || p->lane != proj.lane) continue;
                if (p->kind == PlayerKind::Defense && p->alive) defShield = p;
            }
            PlayerEntity* hit = nullptr;
            for (PlayerEntity* p : players) {
                if (!p->alive || p->lane != proj.lane) continue;
                if (!proj.box().overlaps(p->box())) continue;
                if (!hit || p->x >= hit->x) hit = p;
            }
            if (hit) {
                if (defShield && hit->isShieldedBy(defShield) && defShield->alive) {
                    if (defShield->box().overlaps(proj.box())) {
                        defShield->takeDamage(1);
                        proj.alive = false;
                        ev.emitSound(Sfx::Hit);
                    }
                } else {
                    hit->takeDamage(1);
                    proj.alive = false;
                    ev.emitSound(Sfx::Hit);
                }
            }
        } else {
            EnemyEntity* hit = nullptr;
            for (EnemyEntity* e : enemies) {
                if (!e->alive || e->lane != proj.lane) continue;
                if (!proj.box().overlaps(e->box())) continue;
                if (!hit || e->x <= hit->x) hit = e;
            }
            if (hit) {
                hit->takeDamage(1);
                proj.alive = false;
                ev.emitSound(Sfx::Hit);
            }
        }
    }

    for (PlayerEntity* p : players) {
        if (!p->alive) continue;
        AABB pb = p->box();
        for (Coin& c : coins) {
            if (!c.alive) continue;
            float dx = c.x - p->x;
            float dy = c.y - (p->yCenter() + p->yOffset());
            if (dx * dx + dy * dy <= Config::MAGNET_RADIUS * Config::MAGNET_RADIUS) {
                c.alive = false;
                ev.emitSound(Sfx::Coin);
            }
        }
    }
}
