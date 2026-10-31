#pragma once

#include "Renderer2D.h"
#include "AudioEngine.h"

struct GLFWwindow;

enum class AppStateId { MainMenu, SoundSettings, Cutscene, Gameplay, Pause, Win, Lose };

struct InputState {
    float mouseX = 0.0f, mouseY = 0.0f;
    bool clickPressed = false;
    bool jumpPressed = false;
    bool duckHeld = false;
    bool escPressed = false;
    bool skipPressed = false;
    bool volUpPressed = false;
    bool volDownPressed = false;
    bool mutePressed = false;
};

class GameEngine;

class AppState {
public:
    virtual ~AppState() = default;
    virtual void enter(GameEngine&) {}
    virtual void exit(GameEngine&) {}
    virtual void update(GameEngine&, float dt, const InputState&) {}
    virtual void draw(GameEngine&, Renderer2D&) {}
};

class GameEngine {
public:
    InputState lastInput;
    bool init();
    void shutdown();
    void run();
    void switchTo(AppStateId id);
    AudioEngine audio;
    GLFWwindow* window() const { return mWindow; }
    AppState* state(AppStateId id) { return mStates[(int)id]; }
    float gameTime() const { return mGameTime; }

private:
    InputState pollInput();
    GLFWwindow* mWindow = nullptr;
    int mWindowW = 1280, mWindowH = 720;
    Renderer2D mRenderer;
    AppState* mStates[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    AppState* mActive = nullptr;
    AppStateId mPending = (AppStateId)-1;
    float mGameTime = 0.0f;
    bool mPrevMouse = false;
    bool mPrevJump = false;
    bool mPrevEsc = false;
    bool mPrevDuck = false;
    bool mPrevVolUp = false;
    bool mPrevVolDown = false;
    bool mPrevMute = false;
    float mOrtho[16];
};

