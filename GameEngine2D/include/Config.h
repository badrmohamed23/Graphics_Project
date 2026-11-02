#pragma once

#include <cstdint>
#include <cmath>
#include <vector>

namespace Config {
constexpr float WORLD_W = 1280.0f;
constexpr float WORLD_H = 720.0f;

constexpr float UI_Y_TOP = 720.0f;
constexpr float UI_Y_BOT = 640.0f;
constexpr float GROUND_Y_BOT = 0.0f;
constexpr float GROUND_Y_TOP = 80.0f;

constexpr int LANE_COUNT = 4;
constexpr float LANE_HEIGHT = 120.0f;
constexpr float LANE_Y_MIN[LANE_COUNT] = { 80.0f, 220.0f, 360.0f, 500.0f };
constexpr float LANE_Y_CENTER[LANE_COUNT] = { 140.0f, 280.0f, 420.0f, 560.0f };

constexpr float X_HOME = 160.0f;
constexpr float X_PLAYER_SPAWN = 170.0f;
constexpr float X_ENEMY_SPAWN = 1260.0f;
constexpr float X_PLAYER_CAP = 1270.0f;

constexpr float SPEED_MARCH = 40.0f;
constexpr float SPEED_DEFENSE = 0.0f;
constexpr float SPEED_EASY = 35.0f;
constexpr float SPEED_MEDIUM = 35.0f;
constexpr float SPEED_HARD = 70.0f;

constexpr float PROJECTILE_SPEED = 120.0f;
constexpr float PROJECTILE_HALF_H = 15.0f;
constexpr float ATTACK_COOLDOWN = 1.0f;
constexpr float RANGED_RANGE = 400.0f;

constexpr float JUMP_H_MAX = 60.0f;
constexpr float JUMP_TIME = 0.8f;
constexpr float JUMP_DODGE_THRESHOLD = 35.0f;
constexpr float DUCK_HEIGHT_FACTOR = 0.5f;

constexpr float ENTITY_HALF_W = 30.0f;
constexpr float ENTITY_HALF_H = 45.0f;
constexpr float DUCK_TOP_OFFSET = 8.0f;

constexpr float LOW_PROJ_Y_MAX = 0.0f;
constexpr float HIGH_PROJ_Y_MIN = 10.0f;
constexpr float HIGH_PROJ_Y_MAX = 50.0f;

constexpr int START_COINS = 30;
constexpr int COST_MELEE = 10;
constexpr int COST_RANGED = 20;
constexpr int COST_DEFENSE = 30;
constexpr int DROP_EASY = 10;
constexpr int DROP_MEDIUM = 15;
constexpr int DROP_HARD = 25;

constexpr int HP_MELEE = 5;
constexpr int HP_RANGED = 5;
constexpr int HP_DEFENSE = 7;
constexpr int HP_EASY = 5;
constexpr int HP_MEDIUM = 5;
constexpr int HP_HARD = 7;

constexpr float WAVE_INTERVALS[3] = { 4.0f, 3.0f, 2.5f };
constexpr int WAVE_COUNTS[3] = { 3, 5, 10 };

constexpr float CUTSCENE_DURATION = 15.0f;

constexpr float MAGNET_RADIUS = 40.0f;
constexpr float COIN_SIZE = 18.0f;

inline float laneYCenter(int lane) {
    lane = lane < 0 ? 0 : (lane > LANE_COUNT - 1 ? LANE_COUNT - 1 : lane);
    return LANE_Y_CENTER[lane];
}

inline float laneYMin(int lane) {
    lane = lane < 0 ? 0 : (lane > LANE_COUNT - 1 ? LANE_COUNT - 1 : lane);
    return LANE_Y_MIN[lane];
}

inline int laneFromY(float y) {
    if (y < LANE_Y_MIN[0]) return -1;
    for (int i = LANE_COUNT - 1; i >= 0; --i) {
        if (y >= LANE_Y_MIN[i] && y <= LANE_Y_MIN[i] + LANE_HEIGHT) return i;
    }
    return -1;
}

inline float laneBottomY(int lane) {
    return laneYMin(lane);
}
}

struct AABB {
    float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;

    bool overlapsX(const AABB& o) const { return minX <= o.maxX && maxX >= o.minX; }
    bool overlapsY(const AABB& o) const { return minY <= o.maxY && maxY >= o.minY; }
    bool overlaps(const AABB& o) const { return overlapsX(o) && overlapsY(o); }
};

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
