#include "GridSystem.h"

void GridSystem::updateHover(float worldX, float worldY) {
    mHoveredLane = inBattleground(worldX, worldY) ? Config::laneFromY(worldY) : -1;
}

bool GridSystem::toggleAt(float worldX, float worldY) {
    if (!inBattleground(worldX, worldY)) return false;
    int lane = Config::laneFromY(worldY);
    if (lane < 0) return false;
    if (isSelected(lane)) setSelected(lane, false);
    else setSelected(lane, true);
    return true;
}

void GridSystem::draw(Renderer2D& r, float time) const {
    for (int i = 0; i < Config::LANE_COUNT; ++i) {
        float yMin = Config::laneYMin(i);
        float yTop = yMin + Config::LANE_HEIGHT;
        bool sel = isSelected(i);
        bool hov = mHoveredLane == i;
        Vec3 base(0.06f, 0.07f, 0.1f);
        if (sel) base = base.mix(Vec3(0.1f, 0.5f, 0.45f), 0.55f + 0.15f * sinf(time * 8.0f));
        else if (hov) base = base.mix(Vec3(0.12f, 0.35f, 0.3f), 0.5f);
        r.pushQuad(Config::WORLD_W * 0.5f, (yMin + yTop) * 0.5f, Config::WORLD_W, Config::LANE_HEIGHT, base);
        r.pushQuad(Config::WORLD_W * 0.5f, yMin + 2.0f, Config::WORLD_W, 2.0f, Vec3(0.15f, 0.16f, 0.2f));
        r.pushQuad(Config::WORLD_W * 0.5f, yTop - 2.0f, Config::WORLD_W, 2.0f, Vec3(0.12f, 0.13f, 0.18f));
        for (int t = 0; t < 8; ++t) {
            float tx = 90.0f + t * 140.0f;
            Vec3 tile = Vec3(0.02f, 0.02f, 0.04f).mix(base, sel || hov ? 0.6f : 0.0f);
            r.pushQuad(tx, (yMin + yTop) * 0.5f, 120.0f, Config::LANE_HEIGHT - 8.0f, tile);
        }
        if (sel) {
            Vec3 edge(0.2f, 0.9f, 0.75f);
            r.pushQuad(Config::WORLD_W * 0.5f, yMin + 1.0f, Config::WORLD_W, 1.0f, edge);
            r.pushQuad(Config::WORLD_W * 0.5f, yTop - 1.0f, Config::WORLD_W, 1.0f, edge);
        }
    }
    (void)r;
    (void)time;
}
