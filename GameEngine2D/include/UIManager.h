#pragma once

#include "Config.h"
#include "Renderer2D.h"
#include "WalletManager.h"

struct UIRect {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    bool contains(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
    float cx() const { return x + w * 0.5f; }
    float cy() const { return y + h * 0.5f; }
};

UIRect uiCardRect(int index);
PlayerKind uiCardKind(int index);
int uiHoveredCard(float mx, float my);
void uiDrawToolbar(Renderer2D& r, const WalletManager& wallet, bool hasAttacker, int waveDisplay,
                   bool waveStarted, float time, float mx, float my, int selectedCardIndex);
void uiDrawButton(Renderer2D& r, const UIRect& rect, const char* label, bool hovered, Vec3 base);
void uiDrawHomeBase(Renderer2D& r, float time);
void uiDrawEnvironment(Renderer2D& r, float time);
