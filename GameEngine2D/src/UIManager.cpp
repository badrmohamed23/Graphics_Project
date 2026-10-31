#include "UIManager.h"
#include <cstdio>
#include <cmath>

UIRect uiCardRect(int index) {
    return UIRect{ 40.0f + index * 150.0f, 652.0f, 130.0f, 62.0f };
}

PlayerKind uiCardKind(int index) {
    if (index == 0) return PlayerKind::Melee;
    if (index == 1) return PlayerKind::Ranged;
    return PlayerKind::Defense;
}

int uiCardIndexOf(PlayerKind k) {
    if (k == PlayerKind::Melee) return 0;
    if (k == PlayerKind::Ranged) return 1;
    return 2;
}

int uiHoveredCard(float mx, float my) {
    for (int i = 0; i < 3; ++i) {
        if (uiCardRect(i).contains(mx, my)) return i;
    }
    return -1;
}

static void cardMiniModel(Renderer2D& r, float cx, float cy, PlayerKind k, float time) {
    if (k == PlayerKind::Melee) {
        r.pushQuad(cx - 3.0f, cy - 6.0f, 10.0f, 18.0f, Vec3(0.25f, 0.5f, 0.8f));
        r.pushQuad(cx - 3.0f, cy + 8.0f, 7.0f, 7.0f, Vec3(0.16f, 0.33f, 0.55f));
        r.pushQuad(cx + 2.0f, cy + 9.0f, 4.0f, 3.0f, Vec3(0.0f, 1.0f, 1.0f));
        r.pushQuad(cx + 4.0f, cy - 3.0f, 8.0f, 4.0f, Vec3(0.75f, 0.8f, 0.85f));
        r.pushQuad(cx - 9.0f, cy - 6.0f, 4.0f, 10.0f, Vec3(0.55f, 0.6f, 0.7f));
    } else if (k == PlayerKind::Ranged) {
        r.pushQuad(cx - 3.0f, cy - 6.0f, 8.0f, 17.0f, Vec3(0.35f, 0.62f, 0.68f));
        r.pushQuad(cx - 3.0f, cy + 7.0f, 6.0f, 6.0f, Vec3(0.22f, 0.42f, 0.5f));
        r.pushQuad(cx + 2.0f, cy + 8.0f, 4.0f, 3.0f, Vec3(0.55f, 1.0f, 0.85f));
        r.pushQuad(cx + 3.0f, cy - 2.0f, 13.0f, 3.0f, Vec3(0.55f, 0.6f, 0.68f));
        r.pushQuad(cx + 14.0f, cy - 3.0f, 4.0f, 4.0f, Vec3(0.15f, 0.2f, 0.25f));
    } else {
        r.pushQuad(cx - 4.0f, cy - 5.0f, 14.0f, 18.0f, Vec3(0.45f, 0.42f, 0.3f));
        r.pushQuad(cx - 4.0f, cy - 9.0f, 14.0f, 5.0f, Vec3(0.3f, 0.28f, 0.2f));
        r.pushQuad(cx + 3.0f, cy - 2.0f, 4.0f, 4.0f, Vec3(1.0f, 0.9f, 0.4f));
        r.pushQuad(cx + 10.0f, cy - 1.0f, 3.0f, 12.0f + 2.0f * sinf(time * 4.0f), Vec3(0.2f, 0.85f, 0.55f));
    }
}

void uiDrawCard(Renderer2D& r, int index, bool unlocked, bool selected, bool hovered, float time) {
    float scaleFactor = 1.0f;
    UIRect rc = uiCardRect(index);
    r.pushQuadTL(rc.x, rc.y, rc.w, rc.h, Vec3(0.09f, 0.11f, 0.16f));
    if (selected) r.pushRectOutline(rc.x, rc.y, rc.w, rc.h, 3.0f, Vec3(0.2f, 1.0f, 0.8f));
    else if (hovered && unlocked) r.pushRectOutline(rc.x, rc.y, rc.w, rc.h, 2.0f, Vec3(0.4f, 0.8f, 0.7f));
    else r.pushRectOutline(rc.x, rc.y, rc.w, rc.h, 2.0f, Vec3(0.2f, 0.22f, 0.28f));

    Vec3 tintGlow(0.35f, 1.0f, 0.9f);
    Vec3 dark(0.2f, 0.22f, 0.28f);
    Vec3 fg = unlocked ? tintGlow * 1.2f : Vec3(0.42f, 0.44f, 0.5f);
    (void)scaleFactor;

    cardMiniModel(r, rc.x + 30.0f, rc.cy(), uiCardKind(index), time);

    char label[24];
    PlayerKind k = uiCardKind(index);
    const char* nameL = k == PlayerKind::Melee ? "MELEE" : (k == PlayerKind::Ranged ? "RANGED" : "DEFENSE");
    snprintf(label, 24, "%s", nameL);
    r.pushText(rc.x + 60.0f, rc.y + 16.0f, 2.0f, label, unlocked ? Vec3(0.5f, 0.9f, 0.85f) : Vec3(0.4f, 0.42f, 0.48f));
    char cost[24];
    snprintf(cost, 24, "%d COINS", WalletManager::cardCost(k));
    r.pushText(rc.x + 60.0f, rc.y + 38.0f, 2.0f, cost, unlocked ? Vec3(1.0f, 0.85f, 0.3f) : Vec3(0.45f, 0.42f, 0.38f));
}

void uiDrawToolbar(Renderer2D& r, WalletManager& wallet, int waveDisplay, float time, float mx, float my, int selectedCardIndex) {
    r.pushQuad(Config::WORLD_W * 0.5f, Config::UI_Y_BOT + (Config::UI_Y_TOP - Config::UI_Y_BOT) * 0.5f, Config::WORLD_W, Config::UI_Y_TOP - Config::UI_Y_BOT, Vec3(0.05f, 0.055f, 0.075f));
    r.pushQuad(Config::WORLD_W * 0.5f, Config::UI_Y_BOT, Config::WORLD_W, 2.0f, Vec3(0.15f, 0.6f, 0.5f));

    for (int i = 0; i < 3; ++i) {
        PlayerKind k = uiCardKind(i);
        bool unlocked = wallet.isUnlocked(k);
        uiDrawCard(r, i, unlocked, selectedCardIndex == i, uiHoveredCard(mx, my) == i, time);
    }

    char coins[32];
    snprintf(coins, 32, "COINS: %d", wallet.balance());
    r.pushText(980.0f, 668.0f, 3.0f, coins, Vec3(1.0f, 0.85f, 0.3f));

    char waveLine[32];
    snprintf(waveLine, 32, "WAVE %d/3", waveDisplay < 1 ? 1 : (waveDisplay > 3 ? 3 : waveDisplay));
    r.pushText(980.0f, 694.0f, 3.0f, waveLine, Vec3(0.4f, 0.9f, 1.0f));
}

void uiDrawButton(Renderer2D& r, const UIRect& rect, const char* label, bool hovered, Vec3 base) {
    Vec3 fill = hovered ? base * 1.5f : base;
    r.pushQuadTL(rect.x, rect.y, rect.w, rect.h, fill);
    r.pushRectOutline(rect.x, rect.y, rect.w, rect.h, 2.0f, hovered ? Vec3(0.3f, 1.0f, 0.85f) : Vec3(0.35f, 0.4f, 0.45f));
    float textScale = 2.5f;
    float tw = r.measureText(textScale, label);
    r.pushText(rect.cx() - tw * 0.5f, rect.cy() - 4.0f * textScale, textScale, label, Vec3(0.9f, 0.95f, 0.95f));
}

void uiDrawHomeBase(Renderer2D& r, float time) {
    float coreH = Config::LANE_Y_MIN[0] + Config::LANE_COUNT * Config::LANE_HEIGHT - 90.0f;
    float baseY = Config::LANE_Y_MIN[0];
    r.pushQuadTL(10.0f, baseY, 150.0f, coreH, Vec3(0.1f, 0.11f, 0.14f));
    r.pushQuadTL(14.0f, baseY + 8.0f, 46.0f, coreH - 16.0f, Vec3(0.14f, 0.15f, 0.2f));

    float pulse = 0.55f + 0.45f * sinf(time * 3.0f);
    Vec3 coreGlow(0.2f, 0.9f, 0.7f);
    float cy = baseY + coreH * 0.5f;
    r.pushCircle(37.0f, cy, 26.0f, coreGlow * pulse, 18);
    r.pushCircle(37.0f, cy, 14.0f, Vec3(1.0f, 1.0f, 1.0f) * (0.5f + 0.5f * pulse), 14);
    r.pushRectOutline(16.0f, cy - 30.0f, 42.0f, 60.0f, 3.0f, Vec3(0.3f, 0.32f, 0.38f));

    for (int i = 0; i < 4; ++i) {
        float y = baseY + 40.0f * (float)i + 20.0f;
        r.pushQuad(70.0f, y, 60.0f, 14.0f, Vec3(0.16f, 0.17f, 0.22f));
        r.pushQuad(70.0f, y, 4.0f, 10.0f, coreGlow * pulse);
    }

    float fenceX = Config::X_HOME;
    for (float fy = baseY; fy <= baseY + coreH; fy += 12.0f) {
        float bright = 0.4f + 0.4f * sinf(time * 6.0f + fy * 0.08f);
        r.pushQuad(fenceX, fy, 3.0f, 8.0f, Vec3(0.25f, 1.0f, 0.8f) * bright);
    }
    r.pushQuad(fenceX, baseY - 2.0f, 2.0f, coreH + 4.0f, Vec3(0.2f, 0.22f, 0.26f));
}

void uiDrawEnvironment(Renderer2D& r, float time) {
    r.pushQuad(Config::WORLD_W * 0.5f, Config::WORLD_H * 0.5f, Config::WORLD_W, Config::WORLD_H, Vec3(0.07f, 0.08f, 0.11f));
    for (int i = 0; i < 10; ++i) {
        float px = 60.0f + i * 130.0f;
        r.pushQuad(px, 200.0f, 8.0f, 560.0f, Vec3(0.05f, 0.055f, 0.075f));
    }

    r.pushQuadTL(0.0f, Config::GROUND_Y_BOT, Config::WORLD_W, Config::GROUND_Y_TOP - Config::GROUND_Y_BOT, Vec3(0.1f, 0.105f, 0.14f));
    for (int i = 0; i < 12; ++i) {
        float px = 50.0f + i * 110.0f;
        r.pushQuadTL(px, 14.0f, 60.0f, 8.0f, Vec3(0.16f, 0.17f, 0.24f));
        r.pushQuadTL(px + 24.0f, 30.0f, 36.0f, 5.0f, Vec3(0.12f, 0.13f, 0.18f));
    }

    float blink = 0.5f + 0.5f * sinf(time * 5.0f);
    for (int i = 0; i < 3; ++i) {
        float px = 250.0f + i * 120.0f;
        r.pushQuadTL(px, 92.0f, 60.0f, 40.0f, Vec3(0.12f, 0.13f, 0.18f));
        r.pushQuadTL(px + 8.0f, 96.0f, 44.0f, 24.0f, Vec3(0.1f, 0.7f, 0.6f) * (0.3f + 0.7f * blink));
    }

    for (int i = 0; i < 8; ++i) {
        float px = 120.0f + i * 160.0f;
        r.pushQuad(px, 634.0f, 90.0f, 6.0f, Vec3(0.2f, 0.22f, 0.28f));
        float glow = 0.5f + 0.5f * sinf(time * 2.0f + i);
        r.pushCircle(px + 45.0f, 626.0f, 5.0f, Vec3(0.9f, 0.75f, 0.3f) * glow, 10);
    }

    for (int i = 0; i < 2; ++i) {
        float px = 500.0f + i * 260.0f;
        r.pushQuadTL(px, 110.0f, 40.0f, 60.0f, Vec3(0.25f, 0.16f, 0.05f));
        r.pushQuadTL(px + 6.0f, 118.0f, 28.0f, 20.0f, Vec3(0.95f, 0.8f, 0.2f));
        r.pushQuadTL(px + 10.0f, 124.0f, 5.0f, 8.0f, Vec3(0.1f, 0.1f, 0.1f));
        r.pushTri(px + 22.0f, 146.0f, 12.0f, 10.0f, Vec3(0.1f, 0.1f, 0.1f));
    }

    for (int i = 0; i < 3; ++i) {
        float px = 800.0f + i * 150.0f;
        r.pushQuadTL(px, 96.0f, 70.0f, 44.0f, i == 1 ? Vec3(0.2f, 0.1f, 0.28f) : Vec3(0.28f, 0.14f, 0.06f));
        r.pushQuadTL(px + 6.0f, 104.0f, 20.0f, 8.0f, Vec3(0.85f, 0.7f, 0.2f));
        r.pushQuadTL(px + 34.0f, 104.0f, 20.0f, 8.0f, Vec3(0.85f, 0.7f, 0.2f));
    }

    for (int i = 0; i < 6; ++i) {
        float px = 180.0f + i * 210.0f;
        float sy = 70.0f;
        r.pushQuadTL(px, sy, 60.0f, 8.0f, Vec3(0.15f, 0.16f, 0.22f));
        r.pushQuadTL(px + 10.0f, sy - 14.0f, 8.0f, 14.0f, Vec3(0.15f, 0.16f, 0.22f));
        r.pushQuadTL(px + 40.0f, sy - 14.0f, 8.0f, 14.0f, Vec3(0.15f, 0.16f, 0.22f));
    }
}
