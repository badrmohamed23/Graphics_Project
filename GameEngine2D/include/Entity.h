#pragma once

#include "Config.h"
#include "Renderer2D.h"

#include <functional>
#include <vector>
#include <string>

enum class Faction { None, Player, Enemy };
enum class PlayerKind { Melee, Ranged, Defense };
enum class EnemyKind { Easy, Medium, Hard };
enum class EvadeState { None, Jump };
enum class Sfx { Attack, Hit, Coin, Death, Place, Wave, Win, Lose, None };

struct SimEvents {
    struct PendingProjectile { float x; int lane; float dir; float yRel; float halfH; int damage; bool fromEnemy; };
    std::vector<PendingProjectile> projectiles;
    std::vector<Sfx> sounds;
    std::vector<std::pair<float, int>> coinDrops;
    void emitProjectile(float x, int lane, float dir, float yRel, float halfH, int damage, bool fromEnemy) {
        projectiles.push_back({ x, lane, dir, yRel, halfH, damage, fromEnemy });
    }
    void emitSound(Sfx s) { if (s != Sfx::None) sounds.push_back(s); }
    void emitCoinDrop(float x, int value) { coinDrops.push_back({ x, value }); }
    void clear() { projectiles.clear(); sounds.clear(); coinDrops.clear(); }
};

struct Coin {
    float x = 0.0f, y = 0.0f;
    int value = 10;
    float phase = 0.0f;
    bool alive = true;
};

struct Projectile {
    float x = 0.0f;
    int lane = 0;
    float dir = 1.0f;
    float yRel = 0.0f;
    float halfH = 15.0f;
    float halfW = 9.0f;
    int damage = 1;
    bool fromEnemy = true;
    bool alive = true;

    void update(float dt) {
        x += dir * Config::PROJECTILE_SPEED * dt;
        if (x < -60.0f || x > Config::WORLD_W + 60.0f) alive = false;
    }
    AABB box() const {
        float yc = Config::laneYCenter(lane);
        AABB b;
        b.minX = x - halfW; b.maxX = x + halfW;
        b.minY = yc + yRel - halfH; b.maxY = yc + yRel + halfH;
        return b;
    }
    void draw(Renderer2D& r, float time) const {
        float yc = Config::laneYCenter(lane);
        Vec3 c;
        if (!fromEnemy) c = Vec3(0.1f, 0.9f, 1.0f);
        else if (yRel > 25.0f) c = Vec3(1.0f, 0.35f, 0.95f);
        else c = Vec3(1.0f, 0.55f, 0.15f);
        c = c * (0.8f + 0.2f * sinf(time * 20.0f));
        r.pushQuad(x, yc + yRel, halfW * 2.4f, halfH * 0.8f, c);
        r.pushQuad(x - dir * halfW * 1.4f, yc + yRel, halfW * 0.8f, halfH * 1.4f, c * 0.8f);
    }
};

inline Vec3 dmgColor(float tintFactor, const Vec3& base) {
    static const Vec3 RED(0.75f, 0.05f, 0.05f);
    float f = tintFactor < 0.0f ? 0.0f : (tintFactor > 1.0f ? 1.0f : tintFactor) * 0.65f;
    return base.mix(RED, f);
}

inline Vec3 greyIf(bool locked, const Vec3& base) { return locked ? base.grayscale() : base; }

class Entity {
public:
    float x = 0.0f;
    int lane = 0;
    Faction faction = Faction::None;
    int hp = 1, maxHp = 1;
    float halfW = Config::ENTITY_HALF_W;
    float halfH = Config::ENTITY_HALF_H;
    int damage = 1;
    int reward = 0;
    bool alive = true;
    bool engaged = false;
    bool deadHandled = false;
    float cdTimer = 0.4f;
    float walkPhase = 0.0f;

    virtual ~Entity() = default;
    float yCenter() const { return Config::laneYCenter(lane); }
    virtual float yOffset() const { return 0.0f; }
    virtual AABB box() const {
        float yc = yCenter();
        AABB b;
        b.minX = x - halfW; b.maxX = x + halfW;
        b.minY = yc - halfH; b.maxY = yc + halfH;
        return b;
    }
    float damageTint() const {
        float f = 1.0f - (float)hp / (float)maxHp;
        return f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
    }
    void tickCooldown(float dt) { if (cdTimer > 0.0f) cdTimer -= dt; }
    void takeDamage(int dmg) {
        hp -= dmg;
        if (hp <= 0) { hp = 0; alive = false; }
    }
    virtual void update(float dt, SimEvents& ev) { (void)dt; (void)ev; }
    virtual void draw(Renderer2D& r, float time) const = 0;
};

class PlayerEntity : public Entity {
public:
    PlayerKind kind = PlayerKind::Melee;
    int cost = Config::COST_MELEE;
    EvadeState evade = EvadeState::None;
    bool duckHeld = false;
    float jumpT = 0.0f;
    bool aimTarget = false;

    float yOffset() const override {
        if (evade != EvadeState::Jump) return 0.0f;
        float t = jumpT / Config::JUMP_TIME;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return 4.0f * Config::JUMP_H_MAX * t * (1.0f - t);
    }
    AABB box() const override {
        float yc = yCenter();
        float j = yOffset();
        float top = (evade == EvadeState::None && duckHeld) ? Config::DUCK_TOP_OFFSET : halfH;
        AABB b;
        b.minX = x - halfW; b.maxX = x + halfW;
        b.minY = yc - halfH + j; b.maxY = yc + top + j;
        return b;
    }
    bool isShieldedBy(const PlayerEntity* def) const {
        return def && def->alive && def->kind == PlayerKind::Defense && this != def && this->x < def->x;
    }
    void update(float dt, SimEvents& ev) override;
    void draw(Renderer2D& r, float time) const override;
    static PlayerEntity* make(PlayerKind k, int lane);

private:
    void drawMelee(Renderer2D& r, float yc, float j, float time) const;
    void drawRanged(Renderer2D& r, float yc, float j, float time) const;
    void drawDefense(Renderer2D& r, float yc, float time) const;
};

class EnemyEntity : public Entity {
public:
    EnemyKind kind = EnemyKind::Easy;
    float speed = Config::SPEED_EASY;
    Entity* strikeTarget = nullptr;

    void update(float dt, SimEvents& ev) override;
    void draw(Renderer2D& r, float time) const override;
    static EnemyEntity* make(EnemyKind k, int lane);

private:
    void drawEasy(Renderer2D& r, float yc, float time) const;
    void drawMedium(Renderer2D& r, float yc, float time) const;
    void drawHard(Renderer2D& r, float yc, float time) const;
};
