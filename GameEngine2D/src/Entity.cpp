#include "Entity.h"
#include <cstdio>

PlayerEntity* PlayerEntity::make(PlayerKind k, int lane) {
    PlayerEntity* p = new PlayerEntity();
    p->kind = k;
    p->lane = lane;
    p->faction = Faction::Player;
    p->x = Config::X_PLAYER_SPAWN;
    p->walkPhase = 0.0f;
    switch (k) {
        case PlayerKind::Melee:
            p->hp = p->maxHp = Config::HP_MELEE;
            p->cost = Config::COST_MELEE;
            p->damage = 1;
            break;
        case PlayerKind::Ranged:
            p->hp = p->maxHp = Config::HP_RANGED;
            p->cost = Config::COST_RANGED;
            p->damage = 1;
            break;
        case PlayerKind::Defense:
            p->x = Config::X_DEFENSE_SPAWN;
            p->hp = p->maxHp = Config::HP_DEFENSE;
            p->cost = Config::COST_DEFENSE;
            p->damage = 0;
            break;
    }
    return p;
}

void PlayerEntity::update(float dt, SimEvents& ev) {
    if (kind == PlayerKind::Defense) return;
    tickCooldown(dt);
    if (evade == EvadeState::Jump) {
        jumpT += dt;
        if (jumpT >= Config::JUMP_TIME) {
            evade = EvadeState::None;
            jumpT = 0.0f;
        }
    }
    if (!engaged || !isActive()) {
        x += Config::SPEED_MARCH * dt;
        walkPhase += dt;
    }
    // The last part of the walk offscreen is visible but no longer playable.
    if (!isActive()) {
        if (x - halfW >= Config::WORLD_W) alive = false;
        return;
    }
    if (kind == PlayerKind::Ranged && cdTimer <= 0.0f) {
        ev.emitProjectile(x + halfW + 12.0f, lane, 1.0f, 0.0f, Config::PROJECTILE_HALF_H, damage, false);
        ev.emitSound(Sfx::Attack);
        cdTimer = Config::ATTACK_COOLDOWN;
    }
}

EnemyEntity* EnemyEntity::make(EnemyKind k, int lane) {
    EnemyEntity* e = new EnemyEntity();
    e->kind = k;
    e->lane = lane;
    e->faction = Faction::Enemy;
    e->x = Config::X_ENEMY_SPAWN;
    e->engaged = false;
    e->damage = 1;
    switch (k) {
        case EnemyKind::Easy:
            e->hp = e->maxHp = Config::HP_EASY;
            e->speed = Config::SPEED_EASY;
            e->reward = Config::DROP_EASY;
            break;
        case EnemyKind::Medium:
            e->hp = e->maxHp = Config::HP_MEDIUM;
            e->speed = Config::SPEED_MEDIUM;
            e->reward = Config::DROP_MEDIUM;
            break;
        case EnemyKind::Hard:
            e->hp = e->maxHp = Config::HP_HARD;
            e->speed = Config::SPEED_HARD;
            e->reward = Config::DROP_HARD;
            e->damage = 2;
            break;
    }
    return e;
}

void EnemyEntity::update(float dt, SimEvents& ev) {
    tickCooldown(dt);
    if (!engaged) {
        x -= speed * dt;
        walkPhase += dt;
    }
    if (cdTimer > 0.0f || kind == EnemyKind::Hard) return;
    const bool high = kind == EnemyKind::Easy;
    ev.emitProjectile(x - halfW - 12.0f, lane, -1.0f, high ? 30.0f : Config::LOW_SHOT_CENTER_Y,
                      high ? 20.0f : Config::LOW_SHOT_HALF_H, damage, true);
    ev.emitSound(Sfx::Attack);
    cdTimer = high ? Config::ATTACK_COOLDOWN : Config::ATTACK_COOLDOWN_MEDIUM;
}

void PlayerEntity::drawMelee(Renderer2D& r, float yc, float j, float time) const {
    (void)time;
    float xf = x;
    float legSwing = (evade == EvadeState::Jump) ? 0.0f : sinf(walkPhase * 12.0f) * 6.0f;
    float bob = (evade == EvadeState::Jump) ? 0.0f : fabsf(sinf(walkPhase * 12.0f)) * 3.0f;
    float base = yc - halfH + j;
    float duckF = (evade == EvadeState::None && duckHeld) ? 0.72f : 1.0f;
    float bodyTop = base + 78.0f * duckF + bob;
    float bodyY = (base + bodyTop) * 0.5f;

    float legDuck = (evade == EvadeState::None && duckHeld) ? 0.6f : 1.0f;
    r.pushQuad(xf - 12.0f, base + 10.0f * legDuck, 10.0f, 20.0f * legDuck + (evade == EvadeState::None && !duckHeld ? legSwing * 0.5f : 0.0f), dmgColor(damageTint(), Vec3(0.15f, 0.35f, 0.55f)));
    r.pushQuad(xf + 12.0f, base + 10.0f * legDuck, 10.0f, 20.0f * legDuck + (evade == EvadeState::None && !duckHeld ? -legSwing * 0.5f : 0.0f), dmgColor(damageTint(), Vec3(0.15f, 0.35f, 0.55f)));
    r.pushQuad(xf - 16.0f, base + 24.0f * legDuck, 6.0f, 6.0f, Vec3(0.9f, 0.75f, 0.2f));
    r.pushQuad(xf + 16.0f, base + 24.0f * legDuck, 6.0f, 6.0f, Vec3(0.9f, 0.75f, 0.2f));

    Vec3 suit(0.25f, 0.5f, 0.8f);
    Vec3 suitDark(0.16f, 0.33f, 0.55f);
    Vec3 visor(0.0f, 1.0f, 1.0f);
    r.pushQuad(xf, bodyY, 34.0f, 30.0f * duckF, dmgColor(damageTint(), suit));
    r.pushQuad(xf, bodyY - 10.0f * duckF, 36.0f, 10.0f * duckF, dmgColor(damageTint(), suitDark));
    r.pushQuad(xf + 2.0f, bodyY + 6.0f * duckF, 12.0f, 6.0f, visor * (0.7f + 0.3f * sinf(time * 6.0f)));
    r.pushQuad(xf, bodyTop - 9.0f * duckF, 20.0f, 14.0f * duckF, dmgColor(damageTint(), suitDark));
    r.pushQuad(xf + 6.0f, bodyTop - 8.0f * duckF, 9.0f, 6.0f * duckF, visor * (0.6f + 0.4f * sinf(time * 4.0f)));
    r.pushQuad(xf - 4.0f, bodyTop + 4.0f * duckF, 4.0f, 6.0f, Vec3(0.7f, 0.7f, 0.75f));

    float swing = (evade == EvadeState::Jump) ? 0.0f : sinf((walkPhase * 12.0f) + 1.2f) * 2.0f;
    const float strikeDuration = 0.18f;
    float strike = clamp01(1.0f - (Config::ATTACK_COOLDOWN_MELEE - cdTimer) / strikeDuration);
    float reach = 14.0f * strike;
    float armY = bodyY + 8.0f * duckF + swing + 6.0f * strike;
    r.pushQuad(xf + 20.0f + reach * 0.5f, armY, 16.0f + reach, 7.0f, dmgColor(damageTint(), suitDark));
    r.pushQuad(xf + 33.0f + reach, armY, 10.0f, 9.0f, Vec3(0.75f, 0.8f, 0.85f));
    r.pushQuad(xf + 43.0f + reach, armY, 6.0f, 12.0f, Vec3(0.4f, 0.45f, 0.5f));
    if (strike > 0.0f) {
        r.pushCircle(xf + 50.0f + reach, armY, 7.0f * strike, Vec3(0.4f, 1.0f, 1.0f), 10);
    }
    float shieldPad = bodyY - 6.0f * duckF;
    r.pushQuad(xf - 20.0f, shieldPad, 6.0f, 16.0f * duckF, dmgColor(damageTint(), Vec3(0.55f, 0.6f, 0.7f)));
    r.pushQuad(xf - 20.0f, shieldPad, 2.0f, 12.0f * duckF, Vec3(0.9f, 0.85f, 0.3f));
}

void PlayerEntity::drawRanged(Renderer2D& r, float yc, float j, float time) const {
    (void)time;
    float legSwing = (evade == EvadeState::Jump) ? 0.0f : sinf(walkPhase * 12.0f) * 6.0f;
    float bob = (evade == EvadeState::Jump) ? 0.0f : fabsf(sinf(walkPhase * 12.0f)) * 3.0f;
    float base = yc - halfH + j;
    float duckF = (evade == EvadeState::None && duckHeld) ? 0.72f : 1.0f;
    float bodyTop = base + 76.0f * duckF + bob;
    float bodyY = (base + bodyTop) * 0.5f;

    float legDuck = (evade == EvadeState::None && duckHeld) ? 0.6f : 1.0f;
    r.pushQuad(x - 12.0f, base + 10.0f * legDuck, 9.0f, 20.0f * legDuck + (evade == EvadeState::None && !duckHeld ? legSwing * 0.5f : 0.0f), dmgColor(damageTint(), Vec3(0.2f, 0.28f, 0.45f)));
    r.pushQuad(x + 12.0f, base + 10.0f * legDuck, 9.0f, 20.0f * legDuck + (evade == EvadeState::None && !duckHeld ? -legSwing * 0.5f : 0.0f), dmgColor(damageTint(), Vec3(0.2f, 0.28f, 0.45f)));

    Vec3 suit(0.35f, 0.62f, 0.68f);
    Vec3 suitDark(0.22f, 0.42f, 0.5f);
    Vec3 glow(0.55f, 1.0f, 0.85f);
    r.pushQuad(x, bodyY, 26.0f, 28.0f * duckF, dmgColor(damageTint(), suit));
    r.pushQuad(x, bodyY - 9.0f * duckF, 28.0f, 9.0f * duckF, dmgColor(damageTint(), suitDark));
    r.pushQuad(x + 1.0f, bodyY + 5.0f * duckF, 10.0f, 5.0f, glow * (0.65f + 0.35f * sinf(time * 7.0f)));
    r.pushQuad(x, bodyTop - 8.0f * duckF, 16.0f, 13.0f * duckF, dmgColor(damageTint(), suitDark));
    r.pushQuad(x + 5.0f, bodyTop - 7.0f * duckF, 8.0f, 5.0f * duckF, glow * (0.6f + 0.4f * sinf(time * 5.0f)));
    r.pushQuad(x - 3.0f, bodyTop + 3.0f * duckF, 4.0f, 9.0f, Vec3(0.8f, 0.8f, 0.85f));
    r.pushCircle(x - 3.0f, bodyTop + 9.0f * duckF, 3.0f, glow * (0.6f + 0.4f * sinf(time * 9.0f)), 8);

    float armY = bodyY + 7.0f * duckF;
    r.pushQuad(x + 18.0f, armY, 14.0f, 6.0f, dmgColor(damageTint(), suitDark));
    float recoil = (cdTimer > Config::ATTACK_COOLDOWN - 0.15f) ? 4.0f : 0.0f;
    r.pushQuad(x + 34.0f - recoil, armY, 26.0f, 6.0f, Vec3(0.55f, 0.6f, 0.68f));
    r.pushQuad(x + 30.0f - recoil, armY + 5.0f, 7.0f, 6.0f, Vec3(0.4f, 0.45f, 0.5f));
    r.pushQuad(x + 52.0f - recoil, armY, 8.0f, 8.0f, Vec3(0.15f, 0.2f, 0.25f));
    if (recoil > 0.0f) {
        r.pushCircle(x + 58.0f, armY, 6.0f, Vec3(0.5f, 1.0f, 0.9f), 10);
    }
}

void PlayerEntity::drawDefense(Renderer2D& r, float yc, float time) const {
    float base = yc - halfH;
    float bodyTop = base + 84.0f;
    Vec3 armor(0.45f, 0.42f, 0.3f);
    Vec3 armorDark(0.3f, 0.28f, 0.2f);
    Vec3 gold(0.85f, 0.7f, 0.25f);

    r.pushQuad(x - 20.0f, base + 8.0f, 10.0f, 16.0f, armorDark);
    r.pushQuad(x + 20.0f, base + 8.0f, 10.0f, 16.0f, armorDark);
    r.pushQuad(x, (base + bodyTop) * 0.5f, 44.0f, 44.0f, dmgColor(damageTint(), armor));
    r.pushQuad(x, (base + bodyTop) * 0.5f - 8.0f, 44.0f, 8.0f, armorDark);
    r.pushQuad(x, (base + bodyTop) * 0.5f + 10.0f, 30.0f, 6.0f, gold);
    r.pushQuad(x, (base + bodyTop) * 0.5f - 4.0f, 8.0f, 8.0f, Vec3(1.0f, 0.95f, 0.6f));
    r.pushQuad(x, bodyTop - 10.0f, 30.0f, 16.0f, dmgColor(damageTint(), armorDark));
    r.pushQuad(x + 4.0f, bodyTop - 10.0f, 10.0f, 6.0f, Vec3(1.0f, 0.9f, 0.4f));
    r.pushQuad(x, bodyTop + 6.0f, 10.0f, 5.0f, Vec3(0.6f, 0.58f, 0.5f));

    float phase = fmodf(time, 2.0f);
    float barrierH = 40.0f + 8.0f * sinf(phase * 3.14159f);
    Vec3 barrier(0.2f, 0.85f, 0.55f);
    r.pushQuad(x + halfW + 8.0f, yc, 5.0f, barrierH, barrier);
    r.pushQuad(x + halfW + 8.0f, yc + barrierH * 0.5f, 9.0f, 3.0f, barrier * 1.4f);
    r.pushQuad(x + halfW + 8.0f, yc - barrierH * 0.5f, 9.0f, 3.0f, barrier * 1.4f);
}

void PlayerEntity::draw(Renderer2D& r, float time) const {
    float yc = yCenter();
    float j = yOffset();
    if (kind == PlayerKind::Melee) drawMelee(r, yc, j, time);
    else if (kind == PlayerKind::Ranged) drawRanged(r, yc, j, time);
    else drawDefense(r, yc, time);
}

void EnemyEntity::drawEasy(Renderer2D& r, float yc, float time) const {
    float bob = sinf(time * 3.0f) * 5.0f;
    float cy = yc + bob;
    Vec3 shell(0.55f, 0.2f, 0.6f);
    Vec3 shellDark(0.4f, 0.12f, 0.45f);
    Vec3 eye(1.0f, 0.5f, 1.0f);

    r.pushQuad(x, cy - 26.0f, 44.0f, 8.0f, shellDark);
    r.pushQuad(x, cy, 40.0f, 44.0f, dmgColor(damageTint(), shell));
    r.pushTri(x + 20.0f, cy + 12.0f, 10.0f, 26.0f, dmgColor(damageTint(), shellDark));
    r.pushQuad(x - 4.0f, cy + 22.0f, 14.0f, 8.0f, shellDark);
    r.pushCircle(x + 2.0f, cy + 8.0f, 5.0f, eye * (0.6f + 0.4f * sinf(time * 5.0f)), 10);
    r.pushCircle(x - 12.0f, cy, 8.0f, dmgColor(damageTint(), Vec3(0.75f, 0.3f, 0.8f)), 12);

    float cannonY = cy + 26.0f;
    r.pushQuad(x - 16.0f, cannonY, 8.0f, 16.0f, shellDark);
    r.pushQuad(x - 22.0f, cannonY + 8.0f, 10.0f, 8.0f, Vec3(0.3f, 0.1f, 0.35f));
    if (cdTimer > Config::ATTACK_COOLDOWN - 0.15f) {
        r.pushCircle(x - 26.0f, cannonY + 10.0f, 6.0f, Vec3(1.0f, 0.4f, 1.0f), 10);
    }
}

void EnemyEntity::drawMedium(Renderer2D& r, float yc, float time) const {
    (void)time;
    float base = yc - halfH;
    float legPhase = walkPhase * 14.0f;
    Vec3 carapace(0.75f, 0.45f, 0.1f);
    Vec3 carapaceDark(0.55f, 0.3f, 0.06f);
    Vec3 eye(1.0f, 0.9f, 0.4f);

    for (int i = -2; i <= 2; ++i) {
        float lx = x + i * 11.0f;
        float lift = fabsf(sinf(legPhase + i * 1.1f)) * 6.0f;
        r.pushQuad(lx, base + lift + 4.0f, 4.0f, 16.0f, carapaceDark);
        r.pushQuad(lx + 3.0f, base + lift, 8.0f, 4.0f, carapaceDark);
    }
    float bodyY = base + 34.0f;
    r.pushQuad(x, bodyY, 52.0f, 22.0f, dmgColor(damageTint(), carapace));
    r.pushQuad(x, bodyY + 8.0f, 40.0f, 8.0f, carapaceDark);
    r.pushQuad(x - 8.0f, base + 44.0f, 24.0f, 10.0f, dmgColor(damageTint(), carapaceDark));
    r.pushQuad(x - 6.0f, base + 51.0f, 5.0f, 5.0f, eye);
    r.pushQuad(x + 4.0f, base + 51.0f, 5.0f, 5.0f, eye);

    float cannonY = base + 18.0f;
    r.pushQuad(x - 18.0f, cannonY, 16.0f, 7.0f, carapaceDark);
    r.pushQuad(x - 30.0f, cannonY - 4.0f, 10.0f, 10.0f, Vec3(0.4f, 0.22f, 0.05f));
    if (cdTimer > Config::ATTACK_COOLDOWN_MEDIUM - 0.15f) {
        r.pushCircle(x - 34.0f, cannonY - 2.0f, 6.0f, Vec3(1.0f, 0.6f, 0.2f), 10);
    }
}

void EnemyEntity::drawHard(Renderer2D& r, float yc, float time) const {
    (void)time;
    float base = yc - halfH;
    float legPhase = walkPhase * 20.0f;
    Vec3 shell(0.62f, 0.14f, 0.16f);
    Vec3 shellDark(0.42f, 0.08f, 0.1f);
    Vec3 spikeCol(0.85f, 0.25f, 0.25f);
    Vec3 eye(1.0f, 0.15f, 0.15f);

    for (int i = 0; i < 4; ++i) {
        float lx = x + (i % 2 == 0 ? -14.0f : 14.0f);
        float ly = base + 6.0f;
        float lift = fabsf(sinf(legPhase + i * 1.5f)) * 7.0f;
        r.pushQuad(lx + (i < 2 ? -6.0f : 6.0f), ly + lift, 6.0f, 20.0f, shellDark);
        r.pushTri(lx + (i < 2 ? -6.0f : 6.0f), ly + lift - 8.0f, 10.0f, 10.0f, shellDark);
    }
    float bodyY = base + 42.0f;
    r.pushQuad(x, bodyY, 50.0f, 36.0f, dmgColor(damageTint(), shell));
    r.pushQuad(x - 6.0f, bodyY + 10.0f, 30.0f, 10.0f, shellDark);
    for (int i = -1; i <= 1; ++i) {
        r.pushTri(x + i * 13.0f, bodyY + 26.0f, 9.0f, 14.0f, spikeCol);
    }
    r.pushQuad(x - 16.0f, bodyY + 16.0f, 8.0f, 6.0f, eye * (0.7f + 0.3f * sinf(time * 8.0f)));
    r.pushQuad(x + 2.0f, bodyY + 16.0f, 8.0f, 6.0f, eye * (0.7f + 0.3f * sinf(time * 8.0f)));

    float armY = bodyY + 2.0f;
    r.pushQuad(x - 30.0f, armY, 16.0f, 8.0f, dmgColor(damageTint(), shellDark));
    r.pushTri(x - 44.0f, armY, 12.0f, 16.0f, Vec3(0.9f, 0.9f, 0.95f));
    r.pushQuad(x + 30.0f, armY, 16.0f, 8.0f, dmgColor(damageTint(), shellDark));
    r.pushTri(x + 44.0f, armY, 12.0f, 16.0f, Vec3(0.9f, 0.9f, 0.95f));
    if (engaged && cdTimer > Config::ATTACK_COOLDOWN - 0.15f) {
        r.pushCircle(x - 40.0f, armY, 8.0f, Vec3(1.0f, 0.3f, 0.3f), 10);
    }
}

void EnemyEntity::draw(Renderer2D& r, float time) const {
    float yc = yCenter();
    if (kind == EnemyKind::Easy) drawEasy(r, yc, time);
    else if (kind == EnemyKind::Medium) drawMedium(r, yc, time);
    else drawHard(r, yc, time);
}
