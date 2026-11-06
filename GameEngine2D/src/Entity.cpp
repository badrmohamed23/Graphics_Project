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

float PlayerEntity::rangedMuzzleY() const {
    return yCenter() - halfH + yOffset() + 45.0f * poseScale() + walkBob() * 0.5f;
}

void PlayerEntity::update(float dt, SimEvents& ev) {
    if (kind == PlayerKind::Defense) {
        shieldFlash = clamp01(shieldFlash - dt / 0.14f);
        return;
    }
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
        ev.emitProjectile(x + halfW + 12.0f, lane, 1.0f, rangedMuzzleY() - yCenter(),
                          Config::PROJECTILE_HALF_H, damage, false);
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
    ev.emitProjectile(x - halfW - 12.0f, lane, -1.0f, high ? Config::HIGH_SHOT_CENTER_Y : Config::LOW_SHOT_CENTER_Y,
                      high ? Config::HIGH_SHOT_HALF_H : Config::LOW_SHOT_HALF_H, damage, true);
    ev.emitSound(Sfx::Attack);
    cdTimer = high ? Config::ATTACK_COOLDOWN : Config::ATTACK_COOLDOWN_MEDIUM;
}

static constexpr float SHELL_HALF_HEIGHT = 4.0f;

void Projectile::draw(Renderer2D& r, float time) const {
    const float y = Config::laneYCenter(lane) + yRel;
    const float tip = x + dir * halfW;
    const float rear = x - dir * halfW;
    const float pulse = 0.93f + 0.07f * sinf(time * 16.0f);
    auto wake = [&](float cy, float h, float length, const Vec3& color) {
        r.pushTriangle(rear, cy - h, rear, cy + h, rear - dir * length, cy, color);
    };

    if (!fromEnemy) {
        // Railgun tracer: a narrow hot core and one straight wake.
        wake(y, 2.25f, 22.0f, Vec3(0.16f, 0.43f, 0.52f));
        const Vec3 blue = Vec3(0.21f, 0.78f, 0.92f) * pulse;
        r.pushQuad(x - dir * 1.5f, y, 2.0f * halfW - 3.0f, 5.0f, blue);
        r.pushTriangle(tip - dir * 3.0f, y - 2.5f, tip - dir * 3.0f, y + 2.5f, tip, y, blue);
        r.pushQuad(x + dir * 1.0f, y + 0.4f, 2.0f * halfW - 3.0f, 1.8f, Vec3(0.83f, 1.0f, 1.0f));
        return;
    }

    if (yRel > 0.0f) {
        // Plasma bolt: a rounded hot head and two short exhaust streams.
        wake(y + 3.0f, 1.6f, 13.0f, Vec3(0.46f, 0.15f, 0.33f));
        wake(y - 2.8f, 1.3f, 9.0f, Vec3(0.40f, 0.12f, 0.29f));
        const float noseRadius = 6.0f;
        const float noseX = tip - dir * noseRadius;
        const Vec3 plasma = Vec3(0.84f, 0.27f, 0.59f) * pulse;
        r.pushQuad(x - dir * 2.0f, y, 2.0f * halfW - 4.0f, 8.0f, plasma);
        r.pushCircle(noseX, y, noseRadius, plasma, 16);
        r.pushCircle(noseX + dir * 1.3f, y + 0.6f, 3.4f, Vec3(1.0f, 0.64f, 0.84f) * pulse, 12);
        r.pushQuad(x - dir * 3.5f, y + 1.8f, 6.0f, 1.3f, Vec3(1.0f, 0.46f, 0.73f) * pulse);
        return;
    }

    // The shell's upper edge matches the height a jumper has to clear.
    const float shellY = y + halfH - SHELL_HALF_HEIGHT;
    const float bevel = tip - dir * 3.0f;
    const Vec3 metal = Vec3(0.82f, 0.43f, 0.18f) * pulse;
    wake(shellY - 2.4f, 1.4f, 17.0f, Vec3(0.47f, 0.25f, 0.14f));
    r.pushQuad((rear + bevel) * 0.5f, shellY, 2.0f * halfW - 3.0f, 2.0f * SHELL_HALF_HEIGHT, metal);
    r.pushQuad((bevel + tip) * 0.5f, shellY, 3.0f, 4.0f, metal);
    r.pushTriangle(bevel, shellY + 4.0f, bevel, shellY + 2.0f, tip, shellY + 2.0f, metal);
    r.pushTriangle(bevel, shellY - 4.0f, tip, shellY - 2.0f, bevel, shellY - 2.0f, metal);
    r.pushQuad(rear + dir * 3.0f, shellY, 2.5f, 8.0f, Vec3(0.49f, 0.25f, 0.13f));
    r.pushQuad(x + dir * 1.8f, shellY + 1.8f, 8.0f, 1.8f, Vec3(1.0f, 0.72f, 0.39f));
}

void PlayerEntity::drawMelee(Renderer2D& r, float yc, float j, float time) const {
    float xf = x;
    float legSwing = (evade == EvadeState::Jump) ? 0.0f : sinf(walkPhase * 12.0f) * 6.0f;
    float bob = walkBob();
    float base = yc - halfH + j;
    float duckF = poseScale();
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
    const float gripX = xf + 30.0f + reach;
    const float angle = lerp(1.17f, -0.20f, strike);
    const float dx = cosf(angle), dy = sinf(angle);
    const float bladeX = gripX + dx * 5.0f, bladeY = armY + dy * 5.0f;
    const float tipX = bladeX + dx * 23.0f, tipY = bladeY + dy * 23.0f;
    r.pushTriangle(bladeX - dy * 4.0f, bladeY + dx * 4.0f, bladeX + dy * 3.0f, bladeY - dx * 3.0f,
                   tipX, tipY, Vec3(0.38f, 0.57f, 0.64f));
    r.pushTriangle(bladeX - dy * 4.0f, bladeY + dx * 4.0f, bladeX - dy, bladeY + dx,
                   tipX, tipY, Vec3(0.72f, 0.92f, 0.94f));
    r.pushQuad(bladeX, bladeY, 6.0f, 7.0f, Vec3(0.30f, 0.42f, 0.47f));
    r.pushQuad(gripX, armY, 7.0f, 6.0f, Vec3(0.52f, 0.62f, 0.66f));
    float shieldPad = bodyY - 6.0f * duckF;
    r.pushQuad(xf - 20.0f, shieldPad, 6.0f, 16.0f * duckF, dmgColor(damageTint(), Vec3(0.55f, 0.6f, 0.7f)));
    r.pushQuad(xf - 20.0f, shieldPad, 2.0f, 12.0f * duckF, Vec3(0.9f, 0.85f, 0.3f));
}

void PlayerEntity::drawRanged(Renderer2D& r, float yc, float j, float time) const {
    float legSwing = (evade == EvadeState::Jump) ? 0.0f : sinf(walkPhase * 12.0f) * 6.0f;
    float bob = walkBob();
    float base = yc - halfH + j;
    float duckF = poseScale();
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

    const float armY = rangedMuzzleY();
    const float flash = clamp01(1.0f - (Config::ATTACK_COOLDOWN - cdTimer) / 0.12f);
    const float gunX = x - 4.0f * flash;
    const Vec3 rail(0.46f, 0.56f, 0.60f);
    r.pushQuad(x + 17.0f, armY - 1.0f, 15.0f, 6.0f, dmgColor(damageTint(), suitDark));
    r.pushQuad(gunX + 31.0f, armY, 24.0f, 9.0f, Vec3(0.20f, 0.31f, 0.36f));
    r.pushQuad(gunX + 27.0f, armY - 5.0f, 6.0f, 7.0f, Vec3(0.15f, 0.22f, 0.25f));
    r.pushQuad(gunX + 45.0f, armY + 3.0f, 22.0f, 2.5f, rail);
    r.pushQuad(gunX + 45.0f, armY - 3.0f, 22.0f, 2.5f, rail * 0.68f);
    r.pushQuad(gunX + 44.0f, armY, 21.0f, 1.4f, Vec3(0.18f, 0.48f, 0.54f).mix(glow, flash));
    r.pushQuad(gunX + 55.0f, armY, 4.0f, 9.0f, rail);
    if (flash > 0.0f) r.pushQuad(gunX + 59.0f, armY, 6.0f, 3.0f * flash, glow * flash);
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

    const AABB bounds = box();
    const float fieldX = bounds.maxX;
    const float fieldHeight = bounds.maxY - bounds.minY;
    const float fieldY = (bounds.minY + bounds.maxY) * 0.5f;
    const Vec3 barrier = Vec3(0.24f, 0.91f, 0.63f) * (0.88f + 0.12f * sinf(time * 5.0f));
    r.pushQuad(fieldX - 9.0f, fieldY, 4.0f, fieldHeight - 6.0f, armorDark);
    r.pushQuad(fieldX - 1.0f, fieldY, 5.0f, fieldHeight, barrier * 0.24f);
    r.pushQuad(fieldX, fieldY, 1.8f, fieldHeight, barrier);
    for (float y : {bounds.minY + 3.0f, bounds.maxY - 3.0f}) {
        r.pushQuad(fieldX - 5.0f, y, 13.0f, 6.0f, Vec3(0.42f, 0.45f, 0.38f));
        r.pushQuad(fieldX, y, 4.0f, 2.0f, barrier * 1.2f);
    }
    if (shieldFlash > 0.0f) {
        const float spread = 4.0f + 9.0f * (1.0f - shieldFlash);
        const Vec3 spark = Vec3(0.80f, 1.0f, 0.89f) * shieldFlash;
        r.pushQuad(fieldX, shieldImpactY, 3.0f, 8.0f + spread, barrier.mix(spark, shieldFlash));
        r.pushQuad(fieldX + spread, shieldImpactY + spread * 0.36f, 5.0f, 1.5f, spark);
        r.pushQuad(fieldX + spread * 0.7f, shieldImpactY - spread * 0.4f, 4.0f, 1.5f, spark);
    }
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

    const float cannonY = yc + Config::HIGH_SHOT_CENTER_Y;
    r.pushQuad(x - 17.0f, (cannonY + cy) * 0.5f, 9.0f, cannonY - cy, shellDark);
    r.pushQuad(x - 24.0f, cannonY, 18.0f, 9.0f, shellDark);
    r.pushQuad(x - 25.0f, cannonY + 3.5f, 15.0f, 2.0f, shell);
    r.pushQuad(x - 33.0f, cannonY, 5.0f, 13.0f, shell * 0.78f);
    r.pushQuad(x - 36.0f, cannonY, 2.0f, 4.0f, eye * 0.7f);
    if (cdTimer > Config::ATTACK_COOLDOWN - 0.15f) {
        r.pushCircle(x - 38.0f, cannonY, 4.0f, Vec3(1.0f, 0.48f, 0.78f), 10);
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

    const float cannonY = yc + Config::LOW_SHOT_CENTER_Y + Config::LOW_SHOT_HALF_H - SHELL_HALF_HEIGHT;
    r.pushQuad(x - 20.0f, (cannonY + bodyY) * 0.5f, 9.0f, bodyY - cannonY, carapaceDark);
    r.pushQuad(x - 27.0f, cannonY, 21.0f, 8.0f, carapaceDark);
    r.pushQuad(x - 34.0f, cannonY, 6.0f, 12.0f, carapace * 0.82f);
    r.pushQuad(x - 38.0f, cannonY, 2.0f, 4.0f, eye * 0.58f);
    if (cdTimer > Config::ATTACK_COOLDOWN_MEDIUM - 0.15f) {
        r.pushQuad(x - 40.0f, cannonY, 6.0f, 3.0f, Vec3(1.0f, 0.69f, 0.30f));
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
