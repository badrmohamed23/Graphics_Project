#include "UIManager.h"
#include <cstdio>
#include <cmath>

UIRect uiCardRect(int index) {
    return UIRect{ 40.0f + index * 200.0f, 652.0f, 180.0f, 62.0f };
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

static void cardMiniModel(Renderer2D& r, float cx, float cy, PlayerKind k, bool locked, float time) {
    auto quad = [&](float x, float y, float w, float h, const Vec3& color) {
        r.pushQuad(x, y, w, h, greyIf(locked, color));
    };
    if (k == PlayerKind::Melee) {
        quad(cx - 4.5f, cy - 9.0f, 15.0f, 27.0f, Vec3(0.25f, 0.5f, 0.8f));
        quad(cx - 4.5f, cy + 12.0f, 10.5f, 10.5f, Vec3(0.16f, 0.33f, 0.55f));
        quad(cx + 3.0f, cy + 13.5f, 6.0f, 4.5f, Vec3(0.0f, 1.0f, 1.0f));
        quad(cx + 8.0f, cy - 5.0f, 8.0f, 4.5f, Vec3(0.47f, 0.58f, 0.63f));
        r.pushTriangle(cx + 9.0f, cy - 4.0f, cx + 15.0f, cy - 3.0f, cx + 18.0f, cy + 12.0f,
                       greyIf(locked, Vec3(0.72f, 0.92f, 0.94f)));
        quad(cx - 13.5f, cy - 9.0f, 6.0f, 15.0f, Vec3(0.55f, 0.6f, 0.7f));
    } else if (k == PlayerKind::Ranged) {
        quad(cx - 4.5f, cy - 9.0f, 12.0f, 25.5f, Vec3(0.35f, 0.62f, 0.68f));
        quad(cx - 4.5f, cy + 10.5f, 9.0f, 9.0f, Vec3(0.22f, 0.42f, 0.5f));
        quad(cx + 3.0f, cy + 12.0f, 6.0f, 4.5f, Vec3(0.55f, 1.0f, 0.85f));
        quad(cx + 4.5f, cy - 3.0f, 19.5f, 4.5f, Vec3(0.55f, 0.6f, 0.68f));
        quad(cx + 21.0f, cy - 4.5f, 6.0f, 6.0f, Vec3(0.15f, 0.2f, 0.25f));
    } else {
        quad(cx - 6.0f, cy - 7.5f, 21.0f, 27.0f, Vec3(0.45f, 0.42f, 0.3f));
        quad(cx - 6.0f, cy - 13.5f, 21.0f, 7.5f, Vec3(0.3f, 0.28f, 0.2f));
        quad(cx + 4.5f, cy - 3.0f, 6.0f, 6.0f, Vec3(1.0f, 0.9f, 0.4f));
        quad(cx + 15.0f, cy - 1.5f, 4.5f, locked ? 18.0f : 18.0f + 3.0f * sinf(time * 4.0f),
             Vec3(0.2f, 0.85f, 0.55f));
    }
}

static void uiDrawCard(Renderer2D& r, int index, CardLock lock, int collected, bool selected, bool hovered, float time) {
    UIRect rc = uiCardRect(index);
    const bool locked = lock != CardLock::None;
    r.pushQuadTL(rc.x, rc.y, rc.w, rc.h, greyIf(locked, Vec3(0.09f, 0.11f, 0.16f)));
    Vec3 outline = Vec3(0.2f, 0.22f, 0.28f);
    if (!locked && selected) outline = Vec3(0.2f, 1.0f, 0.8f);
    else if (!locked && hovered) outline = Vec3(0.4f, 0.8f, 0.7f);
    r.pushRectOutline(rc.x, rc.y, rc.w, rc.h, !locked && selected ? 3.0f : 2.0f, greyIf(locked, outline));

    PlayerKind k = uiCardKind(index);
    cardMiniModel(r, rc.x + 30.0f, rc.cy(), k, locked, time);

    const char* label = k == PlayerKind::Melee ? "MELEE" : (k == PlayerKind::Ranged ? "RANGED" : "DEFENSE");
    r.pushText(rc.x + 58.0f, rc.y + 17.0f, 2.0f, label, locked ? Vec3(0.63f, 0.63f, 0.63f) : Vec3(0.5f, 0.9f, 0.85f));
    const int cost = WalletManager::cardCost(k);
    char price[24];
    snprintf(price, sizeof(price), "%d COINS", cost);
    r.pushText(rc.x + 58.0f, rc.y + 39.0f, 1.5f, price, locked ? Vec3(0.54f, 0.54f, 0.54f) : Vec3(1.0f, 0.85f, 0.3f));
    if (locked) {
        char reason[24];
        if (lock == CardLock::CollectCoins) snprintf(reason, sizeof(reason), "COLLECT %d MORE", cost - collected);
        else snprintf(reason, sizeof(reason), "%s", lock == CardLock::NeedAttacker ? "NEED ATTACKER" : "LOW BALANCE");
        r.pushText(rc.x + 58.0f, rc.y + 5.0f, 1.0f, reason, Vec3(0.68f, 0.68f, 0.68f));
    }
}

void uiDrawToolbar(Renderer2D& r, const WalletManager& wallet, bool hasAttacker, int waveDisplay,
                   bool waveStarted, float time, float mx, float my, int selectedCardIndex) {
    r.pushQuad(Config::WORLD_W * 0.5f, Config::UI_Y_BOT + (Config::UI_Y_TOP - Config::UI_Y_BOT) * 0.5f, Config::WORLD_W, Config::UI_Y_TOP - Config::UI_Y_BOT, Vec3(0.05f, 0.055f, 0.075f));
    r.pushQuad(Config::WORLD_W * 0.5f, Config::UI_Y_BOT, Config::WORLD_W, 2.0f, Vec3(0.15f, 0.6f, 0.5f));

    const int hoveredCard = uiHoveredCard(mx, my);
    for (int i = 0; i < 3; ++i) {
        PlayerKind k = uiCardKind(i);
        uiDrawCard(r, i, wallet.lockReason(k, hasAttacker), wallet.collected(), selectedCardIndex == i, hoveredCard == i, time);
    }

    r.pushText(634.0f, 695.0f, 1.35f, "CHOOSE A CARD, THEN A LANE", Vec3(0.65f, 0.90f, 0.86f));
    r.pushText(634.0f, 675.0f, 1.35f, "CLICK LANES TO MARK FOR DODGE", Vec3(0.45f, 0.66f, 0.69f));
    r.pushText(634.0f, 655.0f, 1.35f, "W / UP JUMP   S / DOWN DUCK", Vec3(0.45f, 0.66f, 0.69f));

    char coins[32];
    snprintf(coins, 32, "COINS: %d", wallet.balance());
    r.pushText(980.0f, 668.0f, 3.0f, coins, Vec3(1.0f, 0.85f, 0.3f));

    char collected[32];
    snprintf(collected, sizeof(collected), "COLLECTED: %d", wallet.collected());
    r.pushText(980.0f, 651.0f, 1.4f, collected, Vec3(0.65f, 0.68f, 0.72f));

    char waveLine[32];
    snprintf(waveLine, sizeof(waveLine), "%s %d/3", waveStarted ? "WAVE" : "NEXT", waveDisplay);
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
    float baseY = Config::LANE_Y_MIN[0];
    float coreH = Config::laneYMin(Config::LANE_COUNT - 1) + Config::LANE_HEIGHT - baseY;
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
    r.pushQuad(fenceX, cy, 5.0f, coreH, Vec3(0.2f, 0.22f, 0.26f));
    for (float fy = baseY; fy <= baseY + coreH; fy += 12.0f) {
        float bright = 0.4f + 0.4f * sinf(time * 6.0f + fy * 0.08f);
        r.pushQuad(fenceX, fy, 3.0f, 8.0f, Vec3(0.25f, 1.0f, 0.8f) * bright);
    }
}

static void drawServiceDeck(Renderer2D& r, float time) {
    const Vec3 recess(0.06f, 0.08f, 0.10f);
    const Vec3 housing(0.16f, 0.20f, 0.24f);
    const Vec3 steel(0.26f, 0.32f, 0.36f);
    const Vec3 rim(0.38f, 0.44f, 0.47f);
    const Vec3 coolant(0.25f, 0.59f, 0.54f);
    const Vec3 brass(0.65f, 0.53f, 0.31f);
    const Vec3 stencil(0.58f, 0.66f, 0.66f);
    const float width = Config::WORLD_W;

    // Equipment fits below y=62, leaving the lane's health strip clear.
    r.pushQuadTL(0.0f, Config::GROUND_Y_BOT, width, Config::GROUND_Y_TOP - Config::GROUND_Y_BOT, Vec3(0.09f, 0.11f, 0.14f));
    r.pushQuadTL(0.0f, 8.0f, width, 50.0f, recess);
    r.pushQuadTL(0.0f, 58.0f, width, 4.0f, housing);
    r.pushQuadTL(0.0f, 6.0f, width, 2.0f, steel);

    // The supply and return lines start under the reactor and run through the bays.
    r.pushQuadTL(10.0f, 8.0f, 144.0f, 54.0f, housing);
    r.pushQuadTL(22.0f, 13.0f, 120.0f, 42.0f, recess);
    for (float y : {18.0f, 32.0f}) {
        const float feedX = y == 18.0f ? 65.0f : 108.0f;
        const Vec3 color = y == 18.0f ? steel : coolant * 0.66f;
        r.pushQuadTL(feedX - 5.0f, y + 4.0f, 10.0f, Config::GROUND_Y_TOP - y - 4.0f, color);
        r.pushCircle(feedX, y + 4.0f, 5.0f, color, 12);
        r.pushQuadTL(feedX, y, 897.0f - feedX, 8.0f, color);
        r.pushQuadTL(feedX + 5.0f, y + 6.0f, 887.0f - feedX, 2.0f, color * 1.35f);
        for (float collar : {147.0f, 388.0f, 679.0f, 863.0f}) {
            r.pushQuadTL(collar, y - 2.0f, 6.0f, 12.0f, rim);
        }
    }

    // Reactor console with a recessed display, controls and cable raceway.
    r.pushQuadTL(195.0f, 9.0f, 162.0f, 5.0f, steel);
    r.pushQuadTL(203.0f, 14.0f, 144.0f, 26.0f, housing);
    r.pushQuadTL(201.0f, 38.0f, 152.0f, 5.0f, rim);
    r.pushQuadTL(215.0f, 29.0f, 86.0f, 30.0f, steel);
    r.pushQuadTL(220.0f, 34.0f, 76.0f, 20.0f, recess);
    r.pushText(225.0f, 41.0f, 1.0f, "X-42", coolant);
    const float live = 0.8f + 0.2f * sinf(time * 2.5f);
    r.pushQuadTL(264.0f, 40.0f, 25.0f * live, 3.0f, coolant);
    r.pushQuadTL(311.0f, 25.0f, 29.0f, 9.0f, recess);
    for (float button : {316.0f, 326.0f, 336.0f}) r.pushCircle(button, 30.0f, 2.5f, steel, 8);
    r.pushQuadTL(218.0f, 18.0f, 105.0f, 3.0f, steel);

    // Coolant accumulator and its pressure gauge share the feed line.
    r.pushQuadTL(423.0f, 9.0f, 234.0f, 6.0f, housing);
    r.pushQuadTL(446.0f, 15.0f, 12.0f, 13.0f, steel);
    r.pushQuadTL(563.0f, 15.0f, 12.0f, 13.0f, steel);
    r.pushQuadTL(448.0f, 21.0f, 126.0f, 32.0f, housing);
    r.pushCircle(448.0f, 37.0f, 16.0f, housing, 20);
    r.pushCircle(574.0f, 37.0f, 16.0f, housing, 20);
    r.pushQuadTL(449.0f, 47.0f, 124.0f, 3.0f, steel);
    for (float strap : {461.0f, 550.0f}) r.pushQuadTL(strap, 20.0f, 8.0f, 34.0f, steel);
    r.pushQuadTL(478.0f, 29.0f, 62.0f, 16.0f, recess);
    r.pushQuadTL(483.0f, 34.0f, 52.0f, 5.0f, coolant * live);
    r.pushQuadTL(586.0f, 32.0f, 48.0f, 10.0f, steel);
    r.pushCircle(622.0f, 40.0f, 15.0f, steel, 20);
    r.pushCircle(622.0f, 40.0f, 11.0f, recess, 20);
    for (float tick : {615.0f, 621.0f, 627.0f}) r.pushQuadTL(tick, 45.0f, 2.0f, 2.0f, rim);
    r.pushQuadTL(621.0f, 39.0f, 2.0f, 6.0f, brass);
    r.pushCircle(622.0f, 40.0f, 2.0f, brass, 8);

    // Warning paint belongs to the access hatch itself.
    r.pushQuadTL(715.0f, 8.0f, 122.0f, 47.0f, steel);
    r.pushQuadTL(719.0f, 15.0f, 114.0f, 36.0f, housing);
    r.pushRectOutline(724.0f, 20.0f, 104.0f, 26.0f, 2.0f, recess);
    r.pushQuadTL(765.0f, 38.0f, 22.0f, 4.0f, rim);
    r.pushText(739.0f, 25.0f, 1.0f, "ACCESS", stencil);
    for (float stripe = 720.0f; stripe < 831.0f; stripe += 14.0f) {
        r.pushQuadTL(stripe, 10.0f, 6.0f, 2.0f, brass);
        r.pushQuadTL(stripe + 2.0f, 12.0f, 6.0f, 2.0f, brass);
    }

    // Maintenance cases sit inside one anchored rack.
    r.pushQuadTL(905.0f, 8.0f, 328.0f, 8.0f, steel);
    r.pushQuadTL(916.0f, 16.0f, 7.0f, 42.0f, housing);
    r.pushQuadTL(1215.0f, 16.0f, 7.0f, 42.0f, housing);
    r.pushQuadTL(916.0f, 52.0f, 306.0f, 6.0f, steel);
    auto drawCase = [&](float x, float w, float h) {
        r.pushQuadTL(x, 16.0f, w, h, housing.mix(brass, 0.12f));
        r.pushQuadTL(x, 16.0f + h - 3.0f, w, 3.0f, steel);
        for (float rib : {x + 7.0f, x + w - 11.0f}) r.pushQuadTL(rib, 19.0f, 4.0f, h - 7.0f, steel);
        r.pushQuadTL(x + w * 0.5f - 8.0f, 21.0f + h * 0.5f, 16.0f, 4.0f, recess);
    };
    drawCase(938.0f, 104.0f, 27.0f);
    drawCase(1048.0f, 88.0f, 33.0f);
    drawCase(1142.0f, 58.0f, 22.0f);
}

void uiDrawEnvironment(Renderer2D& r, float time) {
    r.pushQuad(Config::WORLD_W * 0.5f, Config::WORLD_H * 0.5f, Config::WORLD_W, Config::WORLD_H, Vec3(0.07f, 0.08f, 0.11f));
    for (int i = 0; i < 10; ++i) {
        float px = 60.0f + i * 130.0f;
        r.pushQuad(px, 200.0f, 8.0f, 560.0f, Vec3(0.05f, 0.055f, 0.075f));
    }
    drawServiceDeck(r, time);

    for (int i = 0; i < 8; ++i) {
        float px = 120.0f + i * 160.0f;
        r.pushQuad(px, 634.0f, 90.0f, 8.0f, Vec3(0.23f, 0.27f, 0.33f));
        float glow = 0.5f + 0.5f * sinf(time * 2.0f + i);
        r.pushQuad(px, 630.0f, 64.0f, 4.0f, Vec3(0.65f, 0.75f, 0.56f) * (0.6f + 0.4f * glow));
        r.pushCircle(px + 42.0f, 630.0f, 3.5f, Vec3(0.9f, 0.75f, 0.3f) * glow, 10);
    }
}
