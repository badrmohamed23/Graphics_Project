#include "SpatialCollision.h"
#include <algorithm>

static bool inFront(const PlayerEntity* candidate, const PlayerEntity* current) {
    return !current || candidate->x > current->x ||
        (candidate->x == current->x && candidate->kind == PlayerKind::Defense);
}

void SpatialCollision::resolve(std::vector<PlayerEntity*>& players, std::vector<EnemyEntity*>& enemies,
                              std::vector<Projectile>& projectiles, std::vector<Coin>& coins,
                              SimEvents& ev) {
    for (PlayerEntity* p : players) p->engaged = false;
    for (EnemyEntity* e : enemies) {
        if (!e->alive) continue;
        e->engaged = false;
        AABB eb = e->box();
        PlayerEntity* best = nullptr;
        for (PlayerEntity* p : players) {
            if (!p->isActive()) continue;
            if (p->lane != e->lane) continue;
            AABB pb = p->box();
            if (!eb.overlaps(pb)) continue;
            p->engaged = true;
            if (inFront(p, best)) best = p;
        }
        if (best) {
            e->engaged = true;
            if (e->kind == EnemyKind::Hard && e->cdTimer <= 0.0f) {
                best->takeDamage(e->damage);
                e->cdTimer = Config::ATTACK_COOLDOWN;
                ev.emitSound(Sfx::Hit);
            }
        }
    }

    for (PlayerEntity* p : players) {
        if (!p->isActive() || p->kind == PlayerKind::Defense) continue;
        if (p->kind == PlayerKind::Melee && p->cdTimer <= 0.0f) {
            AABB pb = p->box();
            EnemyEntity* target = nullptr;
            for (EnemyEntity* e : enemies) {
                if (!e->alive || e->lane != p->lane) continue;
                if (!pb.overlaps(e->box())) continue;
                if (!target || e->x < target->x) target = e;
            }
            if (target) {
                target->takeDamage(p->damage);
                p->cdTimer = Config::ATTACK_COOLDOWN_MELEE;
                ev.emitSound(Sfx::Attack);
            }
        }
    }

    for (Projectile& proj : projectiles) {
        if (!proj.alive) continue;
        const AABB shot = proj.box();
        if (proj.fromEnemy) {
            PlayerEntity* hit = nullptr;
            // Incoming shots meet the front unit first, including a shield between
            // the shot and an ally walking behind it.
            for (PlayerEntity* p : players) {
                if (!p->isActive() || p->lane != proj.lane) continue;
                if (shot.overlaps(p->box()) && inFront(p, hit)) hit = p;
            }
            if (hit) {
                if (hit->kind == PlayerKind::Defense) {
                    const AABB field = hit->box();
                    hit->shieldImpactY = (std::max(shot.minY, field.minY) + std::min(shot.maxY, field.maxY)) * 0.5f;
                    hit->shieldFlash = 1.0f;
                }
                hit->takeDamage(proj.damage);
                proj.alive = false;
                ev.emitSound(Sfx::Hit);
            }
        } else {
            EnemyEntity* hit = nullptr;
            for (EnemyEntity* e : enemies) {
                if (!e->alive || e->lane != proj.lane) continue;
                if (!shot.overlaps(e->box())) continue;
                if (!hit || e->x <= hit->x) hit = e;
            }
            if (hit) {
                hit->takeDamage(proj.damage);
                proj.alive = false;
                ev.emitSound(Sfx::Hit);
                const AABB target = hit->box();
                const float impactX = std::clamp(shot.maxX, target.minX, target.maxX);
                const float impactY = std::clamp((shot.minY + shot.maxY) * 0.5f, target.minY, target.maxY);
                ev.railgunImpacts.push_back({impactX, impactY});
            }
        }
    }

    for (PlayerEntity* p : players) {
        if (!p->isActive()) continue;
        AABB pb = p->box();
        for (Coin& c : coins) {
            if (!c.alive || c.lane != p->lane) continue;
            const float cy = c.centerY();
            const float dx = c.x - std::clamp(c.x, pb.minX, pb.maxX);
            const float dy = cy - std::clamp(cy, pb.minY, pb.maxY);
            if (dx * dx + dy * dy <= Config::COIN_RADIUS * Config::COIN_RADIUS) {
                c.alive = false;
                ev.emitSound(Sfx::Coin);
            }
        }
    }
}
