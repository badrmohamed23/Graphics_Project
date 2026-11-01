#include "GameEngine.h"
#include "Config.h"
#include "Entity.h"
#include "GridSystem.h"
#include "WaveSpawner.h"
#include "WalletManager.h"
#include "SpatialCollision.h"
#include "UIManager.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#define PSAPI_VERSION 2
#include <windows.h>
#include <psapi.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
UIRect BTN_START = { 520.0f, 300.0f, 240.0f, 56.0f };
UIRect BTN_SOUND = { 520.0f, 380.0f, 240.0f, 56.0f };
UIRect BTN_EXIT = { 520.0f, 460.0f, 240.0f, 56.0f };
UIRect BTN_BACK = { 520.0f, 470.0f, 240.0f, 56.0f };
UIRect BTN_MUTE = { 480.0f, 390.0f, 160.0f, 56.0f };
UIRect BTN_VOLUP = { 680.0f, 330.0f, 100.0f, 56.0f };
UIRect BTN_VOLDOWN = { 500.0f, 330.0f, 100.0f, 56.0f };
UIRect BTN_RESUME = { 520.0f, 320.0f, 240.0f, 56.0f };
UIRect BTN_RESTART = { 520.0f, 400.0f, 240.0f, 56.0f };
UIRect BTN_TOMAIN = { 520.0f, 480.0f, 240.0f, 56.0f };

static void drawTitleBanner(Renderer2D& r, const char* title, float cy) {
    float s = 5.0f;
    float tw = r.measureText(s, title);
    r.pushText(Config::WORLD_W * 0.5f - tw * 0.5f, cy, s, title, Vec3(0.3f, 1.0f, 0.85f));
}

static void drawSubText(Renderer2D& r, const char* sub, float cy, float scale, Vec3 color) {
    float tw = r.measureText(scale, sub);
    r.pushText(Config::WORLD_W * 0.5f - tw * 0.5f, cy, scale, sub, color);
}

static void drawCenteredWorldText(Renderer2D& r, const char* text, float cy, float scale, Vec3 color) {
    float tw = r.measureText(scale, text);
    r.pushText(Config::WORLD_W * 0.5f - tw * 0.5f, cy, scale, text, color);
}

class MainMenuState : public AppState {
public:
    void enter(GameEngine& e) override { e.audio.stopBgm(); }
    void update(GameEngine& e, float dt, const InputState& in) override {
        (void)dt;
        if (!in.clickPressed) return;
        if (BTN_START.contains(in.mouseX, in.mouseY)) e.switchTo(AppStateId::Cutscene);
        else if (BTN_SOUND.contains(in.mouseX, in.mouseY)) e.switchTo(AppStateId::SoundSettings);
        else if (BTN_EXIT.contains(in.mouseX, in.mouseY)) glfwSetWindowShouldClose(e.window(), GLFW_TRUE);
    }
    void draw(GameEngine& e, Renderer2D& r) override {
        float t = e.gameTime();
        const InputState& in = e.lastInput;
        uiDrawEnvironment(r, t);
        drawTitleBanner(r, "NEON OUTPOST", 130.0f);
        drawSubText(r, "DMET 502 COMPUTER GRAPHICS - WINTER 2026", 235.0f, 2.5f, Vec3(0.5f, 0.55f, 0.6f));
        drawSubText(r, "DEFEND THE REACTOR CORE FROM THE XENO SWARM", 262.0f, 2.2f, Vec3(0.35f, 0.75f, 0.7f));
        uiDrawButton(r, BTN_START, "START GAME", BTN_START.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        uiDrawButton(r, BTN_SOUND, "SOUND SETTINGS", BTN_SOUND.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        uiDrawButton(r, BTN_EXIT, "EXIT", BTN_EXIT.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        drawSubText(r, "GUARD THE LAST REACTOR OF LAB X-42", 560.0f, 2.0f, Vec3(0.3f, 0.6f, 0.55f));
    }
};

class SoundSettingsState : public AppState {
public:
    void update(GameEngine& e, float dt, const InputState& in) override {
        (void)dt;
        if (in.clickPressed) {
            if (BTN_VOLUP.contains(in.mouseX, in.mouseY)) e.audio.stepVolume(0.1f);
            else if (BTN_VOLDOWN.contains(in.mouseX, in.mouseY)) e.audio.stepVolume(-0.1f);
            else if (BTN_MUTE.contains(in.mouseX, in.mouseY)) e.audio.toggleMute();
            else if (BTN_BACK.contains(in.mouseX, in.mouseY)) e.switchTo(AppStateId::MainMenu);
        }
        if (in.volUpPressed) e.audio.stepVolume(0.1f);
        if (in.volDownPressed) e.audio.stepVolume(-0.1f);
        if (in.mutePressed) e.audio.toggleMute();
    }
    void draw(GameEngine& e, Renderer2D& r) override {
        const InputState& in = e.lastInput;
        uiDrawEnvironment(r, e.gameTime());
        drawTitleBanner(r, "SOUND SETTINGS", 140.0f);
        drawSubText(r, "MASTER VOLUME", 300.0f, 3.0f, Vec3(0.6f, 0.65f, 0.7f));
        r.pushBar(BTN_VOLDOWN.x + BTN_VOLDOWN.w + 10.0f, BTN_VOLDOWN.y + 12.0f, 70.0f, 32.0f, e.audio.volume(), Vec3(0.2f, 0.9f, 0.7f), Vec3(0.1f, 0.12f, 0.18f));
        uiDrawButton(r, BTN_VOLUP, "VOL+", BTN_VOLUP.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        uiDrawButton(r, BTN_VOLDOWN, "VOL-", BTN_VOLDOWN.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        char volLabel[32];
        snprintf(volLabel, 32, "%s", e.audio.isMuted() ? "UNMUTE" : "MUTE");
        uiDrawButton(r, BTN_MUTE, volLabel, BTN_MUTE.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        uiDrawButton(r, BTN_BACK, "BACK", BTN_BACK.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        drawSubText(r, "[+ / -] VOLUME   [M] MUTE", 560.0f, 2.2f, Vec3(0.35f, 0.6f, 0.55f));
    }
};

class CutsceneState : public AppState {
public:
    void enter(GameEngine& e) override {
        (void)e;
        mTimer = 0.0f;
        mGuardian = PlayerEntity::make(PlayerKind::Melee, 2);
        mXenoUpper = EnemyEntity::make(EnemyKind::Easy, 2);
        mXenoLower = EnemyEntity::make(EnemyKind::Hard, 2);
        mGuardian->engaged = true;
        mXenoUpper->engaged = true;
        mXenoLower->engaged = true;
    }
    void exit(GameEngine& e) override {
        (void)e;
        delete mGuardian; mGuardian = nullptr;
        delete mXenoUpper; mXenoUpper = nullptr;
        delete mXenoLower; mXenoLower = nullptr;
    }
    void update(GameEngine& e, float dt, const InputState& in) override {
        (void)in;
        mTimer += dt;
        mGuardian->walkPhase += dt;
        mXenoUpper->walkPhase += dt;
        mXenoLower->walkPhase += dt;
        if (mTimer >= Config::CUTSCENE_DURATION) e.switchTo(AppStateId::Gameplay);
    }
    void draw(GameEngine& e, Renderer2D& r) override {
        float t = mTimer;
        uiDrawEnvironment(r, e.gameTime());

        float gX = 130.0f;
        if (t < 4.0f) gX = lerp(110.0f, 200.0f, t / 4.0f);
        else if (t < 10.0f) gX = 200.0f;
        else gX = lerp(200.0f, 300.0f, (t - 10.0f) / 5.0f);
        mGuardian->x = gX;
        float x1X = 1330.0f;
        if (t >= 3.0f && t < 6.0f) x1X = lerp(1330.0f, 990.0f, (t - 3.0f) / 3.0f);
        else if (t >= 6.0f) x1X = 990.0f + 8.0f * sinf(t * 2.0f);
        mXenoUpper->x = x1X;
        float x2X = 1330.0f;
        if (t >= 4.0f && t < 7.0f) x2X = lerp(1330.0f, 1120.0f, (t - 4.0f) / 3.0f);
        else if (t >= 7.0f) x2X = 1120.0f + 6.0f * sinf(t * 1.7f);
        mXenoLower->x = x2X;

        if (t >= 10.0f) {
            Vec3 charge(0.4f, 1.0f, 1.0f);
            float pulse = 0.5f + 0.5f * sinf(t * 14.0f);
            r.pushCircle(gX + 50.0f, Config::laneYCenter(2) + 22.0f, 12.0f + 6.0f * pulse, charge * pulse, 16);
        }

        mGuardian->draw(r, e.gameTime());
        mXenoUpper->draw(r, e.gameTime());
        mXenoLower->draw(r, e.gameTime());

        r.pushQuad(Config::WORLD_W * 0.5f, 52.0f, Config::WORLD_W, 84.0f, Vec3(0.03f, 0.035f, 0.05f));
        Vec3 subColor(0.85f, 0.92f, 0.95f);
        float subY = 30.0f;
        if (t < 3.5f) drawCenteredWorldText(r, "LAB X-42 - NIGHT SHIFT. THE CORE KEEPS THE CITY ALIVE.", subY, 2.4f, subColor);
        else if (t < 6.5f) drawCenteredWorldText(r, "UNIDENTIFIED SWARM BREACHING THE EAST WALL!", subY, 2.4f, Vec3(1.0f, 0.5f, 0.4f));
        else if (t < 10.0f) drawCenteredWorldText(r, "XENO: THE REACTOR CORE... IS OURS.", subY, 2.4f, Vec3(1.0f, 0.35f, 1.0f));
        else if (t < 13.0f) drawCenteredWorldText(r, "GUARDIAN: NOT WHILE I STAND HERE.", subY, 2.4f, Vec3(0.4f, 1.0f, 1.0f));
        else drawCenteredWorldText(r, "DEFEND THE CORE - DEPLOY YOUR GUARDIANS!", subY, 2.4f, Vec3(0.4f, 1.0f, 0.7f));

        r.pushBar(Config::WORLD_W * 0.5f - 100.0f, 8.0f, 200.0f, 5.0f, t / Config::CUTSCENE_DURATION, Vec3(0.25f, 0.9f, 0.7f), Vec3(0.1f, 0.12f, 0.15f));
        if (t >= 2.0f) drawSubText(r, "PRESS ANY KEY TO SKIP", 686.0f, 1.8f, Vec3(0.4f, 0.45f, 0.5f));
    }
private:
    float mTimer = 0.0f;
    PlayerEntity* mGuardian = nullptr;
    EnemyEntity* mXenoUpper = nullptr;
    EnemyEntity* mXenoLower = nullptr;
};

class GameplayState : public AppState {
public:
    bool resetOnEnter = true;
    void enter(GameEngine& e) override {
        if (resetOnEnter) reset();
        resetOnEnter = true;
        e.audio.playBgm();
    }
    void update(GameEngine& e, float dt, const InputState& in) override;
    void draw(GameEngine& e, Renderer2D& r) override;

    GridSystem grid;
    WaveSpawner spawner;
    WalletManager wallet;
    std::vector<PlayerEntity*> players;
    std::vector<EnemyEntity*> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Coin> coins;
    int selectedCard = -1;

private:
    void reset() {
        for (PlayerEntity* p : players) delete p;
        players.clear();
        for (EnemyEntity* en : enemies) delete en;
        enemies.clear();
        projectiles.clear();
        coins.clear();
        grid.reset();
        spawner.reset();
        wallet.reset();
        selectedCard = -1;
    }
    void pruneDead(SimEvents& ev) {
        for (size_t i = players.size(); i > 0; --i) {
            PlayerEntity* p = players[i - 1];
            if (!p->alive) {
                delete p;
                players.erase(players.begin() + (long)(i - 1));
            }
        }
        for (size_t i = enemies.size(); i > 0; --i) {
            EnemyEntity* en = enemies[i - 1];
            if (!en->alive) {
                delete en;
                enemies.erase(enemies.begin() + (long)(i - 1));
            }
        }
        for (size_t i = projectiles.size(); i > 0; --i) {
            if (!projectiles[i - 1].alive) projectiles.erase(projectiles.begin() + (long)(i - 1));
        }
        for (size_t i = coins.size(); i > 0; --i) {
            if (!coins[i - 1].alive) coins.erase(coins.begin() + (long)(i - 1));
        }
        (void)ev;
    }
    void drawCoins(Renderer2D& r, float time) {
        for (Coin& c : coins) {
            if (!c.alive) continue;
            float bob = sinf(time * 4.0f + c.phase) * 3.0f;
            Vec3 gold(1.0f, 0.82f, 0.25f);
            r.pushCircle(c.x, c.y + bob, 9.0f, gold, 14);
            r.pushCircle(c.x - 2.0f, c.y + bob + 2.0f, 4.0f, gold * 1.4f, 8);
        }
    }
};

void GameplayState::update(GameEngine& e, float dt, const InputState& in) {
    if (in.escPressed) {
        e.switchTo(AppStateId::Pause);
        return;
    }
    grid.updateHover(in.mouseX, in.mouseY);

    SimEvents ev;
    int enemiesAlive = 0;
    for (EnemyEntity* en : enemies) if (en->alive) ++enemiesAlive;
    spawner.update(dt, enemiesAlive, [this](EnemyKind kind, int lane) {
        enemies.push_back(EnemyEntity::make(kind, lane));
    }, ev);

    if (in.jumpPressed) {
        for (PlayerEntity* p : players) {
            if (p->kind == PlayerKind::Defense) continue;
            if (grid.isSelected(p->lane)) {
                p->evade = EvadeState::Jump;
                p->jumpT = 0.0f;
            }
        }
    }
    for (PlayerEntity* p : players) {
        if (p->kind == PlayerKind::Defense) continue;
        p->duckHeld = grid.isSelected(p->lane) && in.duckHeld;
    }

    if (in.clickPressed) {
        int hov = uiHoveredCard(in.mouseX, in.mouseY);
        if (hov >= 0) {
            PlayerKind k = uiCardKind(hov);
            if (wallet.isUnlocked(k)) selectedCard = hov;
        } else if (grid.inBattleground(in.mouseX, in.mouseY)) {
            if (selectedCard >= 0) {
                PlayerKind k = uiCardKind(selectedCard);
                int lane = Config::laneFromY(in.mouseY);
                int cost = WalletManager::cardCost(k);
                if (lane >= 0 && wallet.balance() >= cost) {
                    players.push_back(PlayerEntity::make(k, lane));
                    wallet.spend(cost);
                    ev.emitSound(Sfx::Place);
                    selectedCard = -1;
                }
            } else {
                grid.toggleAt(in.mouseX, in.mouseY);
            }
        }
    }

    for (PlayerEntity* p : players) p->update(dt, ev);
    for (EnemyEntity* en : enemies) en->update(dt, ev);
    for (Projectile& proj : projectiles) proj.update(dt);

    bool homeBreached = false;
    SpatialCollision::resolve(players, enemies, projectiles, coins, ev, homeBreached);

    for (Coin& c : coins) {
        if (!c.alive) wallet.earn(c.value);
    }

    for (EnemyEntity* en : enemies) {
        if (!en->alive && !en->deadHandled) {
            Coin drop;
            drop.x = en->x;
            drop.y = Config::laneYCenter(en->lane);
            drop.value = en->reward;
            drop.phase = (float)(rand() % 628) / 100.0f;
            coins.push_back(drop);
            ev.emitSound(Sfx::Death);
            en->deadHandled = true;
        }
    }
    for (PlayerEntity* p : players) {
        if (!p->alive && !p->deadHandled) {
            ev.emitSound(Sfx::Death);
            p->deadHandled = true;
        }
    }

    for (Sfx s : ev.sounds) e.audio.play(s);
    for (SimEvents::PendingProjectile& pp : ev.projectiles) {
        Projectile proj;
        proj.x = pp.x;
        proj.lane = pp.lane;
        proj.dir = pp.dir;
        proj.yRel = pp.yRel;
        proj.halfH = pp.halfH;
        proj.damage = pp.damage;
        proj.fromEnemy = pp.fromEnemy;
        projectiles.push_back(proj);
    }
    ev.clear();

    pruneDead(ev);

    if (homeBreached) {
        e.audio.play(Sfx::Lose);
        e.audio.stopBgm();
        e.switchTo(AppStateId::Lose);
        return;
    }
    if (spawner.allWavesCleared()) {
        e.audio.play(Sfx::Win);
        e.audio.stopBgm();
        e.switchTo(AppStateId::Win);
        return;
    }
    if (in.mutePressed) e.audio.toggleMute();
}

void GameplayState::draw(GameEngine& e, Renderer2D& r) {
    float t = e.gameTime();
    const InputState& in = e.lastInput;
    uiDrawEnvironment(r, t);
    grid.draw(r, t);
    uiDrawHomeBase(r, t);
    for (Projectile& proj : projectiles) proj.draw(r, t);
    for (EnemyEntity* en : enemies) en->draw(r, t);
    for (PlayerEntity* p : players) p->draw(r, t);
    drawCoins(r, t);
    uiDrawToolbar(r, wallet, spawner.waveIndex() + 1, t, in.mouseX, in.mouseY, selectedCard);
    drawSubText(r, "CLICK A CARD, THEN A LANE TO DEPLOY", 628.0f, 1.8f, Vec3(0.35f, 0.65f, 0.6f));
}

class PauseMenuState : public AppState {
public:
    void enter(GameEngine& e) override {
        GameplayState* gp = static_cast<GameplayState*>(e.state(AppStateId::Gameplay));
        gp->resetOnEnter = false;
    }
    void update(GameEngine& e, float dt, const InputState& in) override {
        (void)dt;
        if (in.escPressed || (in.clickPressed && BTN_RESUME.contains(in.mouseX, in.mouseY))) {
            e.switchTo(AppStateId::Gameplay);
            return;
        }
        if (in.clickPressed && BTN_RESTART.contains(in.mouseX, in.mouseY)) {
            static_cast<GameplayState*>(e.state(AppStateId::Gameplay))->resetOnEnter = true;
            e.switchTo(AppStateId::Gameplay);
            return;
        }
        if (in.clickPressed && BTN_TOMAIN.contains(in.mouseX, in.mouseY)) e.switchTo(AppStateId::MainMenu);
    }
    void draw(GameEngine& e, Renderer2D& r) override {
        e.state(AppStateId::Gameplay)->draw(e, r);
        r.pushQuad(Config::WORLD_W * 0.5f, Config::WORLD_H * 0.5f, Config::WORLD_W, Config::WORLD_H, Vec3(0.02f, 0.03f, 0.04f));
        drawTitleBanner(r, "PAUSED", 160.0f);
        const InputState& in = e.lastInput;
        uiDrawButton(r, BTN_RESUME, "CONTINUE", BTN_RESUME.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        uiDrawButton(r, BTN_RESTART, "RESTART", BTN_RESTART.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        uiDrawButton(r, BTN_TOMAIN, "MAIN MENU", BTN_TOMAIN.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
    }
};

class EndMenuState : public AppState {
public:
    explicit EndMenuState(bool win) : winMode(win) {}
    bool winMode = true;
    void update(GameEngine& e, float dt, const InputState& in) override {
        (void)dt;
        if (!in.clickPressed) return;
        if (BTN_RESTART.contains(in.mouseX, in.mouseY)) {
            static_cast<GameplayState*>(e.state(AppStateId::Gameplay))->resetOnEnter = true;
            e.switchTo(AppStateId::Gameplay);
        } else if (BTN_TOMAIN.contains(in.mouseX, in.mouseY)) {
            e.switchTo(AppStateId::MainMenu);
        }
    }
    void draw(GameEngine& e, Renderer2D& r) override {
        e.state(AppStateId::Gameplay)->draw(e, r);
        r.pushQuad(Config::WORLD_W * 0.5f, Config::WORLD_H * 0.5f, Config::WORLD_W, Config::WORLD_H, Vec3(0.02f, 0.03f, 0.04f));
        if (winMode) {
            drawTitleBanner(r, "ALL WAVES CLEARED - YOU WIN!", 180.0f);
            drawSubText(r, "THE REACTOR CORE STANDS INTACT", 250.0f, 2.5f, Vec3(0.4f, 0.9f, 0.75f));
        } else {
            drawTitleBanner(r, "HOME BREACHED - YOU LOSE", 180.0f);
            drawSubText(r, "THE XENO SWARM REACHED THE CORE", 250.0f, 2.5f, Vec3(0.9f, 0.4f, 0.4f));
        }
        const InputState& in = e.lastInput;
        uiDrawButton(r, BTN_RESTART, "RESTART", BTN_RESTART.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
        uiDrawButton(r, BTN_TOMAIN, "MAIN MENU", BTN_TOMAIN.contains(in.mouseX, in.mouseY), Vec3(0.08f, 0.28f, 0.26f));
    }
};

} // anonymous namespace

static gl::GLenum checkGLError(const char* where) {
    gl::GLenum err = gl::glGetError();
    if (err != 0) fprintf(stderr, "GL error 0x%04X at %s\n", (unsigned)err, where);
    return err;
}

static void drawBorderTestQuads(Renderer2D& r) {
    static bool announced = false;
    if (!announced) { printf("[P2] border-quad verification active (NEON_P2_BORDER)\n"); announced = true; }
    const Vec3 c(1.0f, 0.0f, 0.0f);
    r.pushQuad(640.0f, 0.5f, 1280.0f, 1.0f, c);
    r.pushQuad(640.0f, 719.5f, 1280.0f, 1.0f, c);
    r.pushQuad(0.5f, 360.0f, 1.0f, 720.0f, c);
    r.pushQuad(1279.5f, 360.0f, 1.0f, 720.0f, c);
}

bool GameEngine::init() {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW init failed\n");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    mWindow = glfwCreateWindow(1280, 720, "NEON OUTPOST - DMET502 Project 1", nullptr, nullptr);
    if (!mWindow) {
        fprintf(stderr, "Window creation failed\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(mWindow);
    printf("Loaded modules:\n");
    { HANDLE pr = GetCurrentProcess(); HMODULE hm[256]; DWORD cb = 0;
      if (EnumProcessModules(pr, hm, sizeof(hm), &cb))
          for (DWORD i = 0; i < cb / sizeof(HMODULE); ++i) {
              char name2[MAX_PATH];
              if (GetModuleFileNameA(hm[i], name2, MAX_PATH)) printf("  %s\n", name2);
          } }

    glfwSwapInterval(1);

    if (!gl::loadGLFunctions(reinterpret_cast<gl::GLProcLoader>(glfwGetProcAddress))) {
        fprintf(stderr, "OpenGL 3.3 Core not available\n");
        return false;
    }
    {
        const char* vs = (const char*)gl::glGetString(0x1F00);
        const char* rs = (const char*)gl::glGetString(0x1F01);
        printf("GL_VERSION: %s\n", vs ? vs : "?");
        printf("GL_RENDERER: %s\n", rs ? rs : "?");
        int verMajor = 0, verMinor = 0;
        if (vs) {
            const char* digitStart = vs;
            while (*digitStart && (*digitStart < '0' || *digitStart > '9')) ++digitStart;
            sscanf(digitStart, "%d.%d", &verMajor, &verMinor);
        }
        if (verMajor == 0 && verMinor == 0) {
            printf("GL_VERSION unparsable ('%s'); continuing — all pipeline entry points resolved\n", vs ? vs : "(null)");
        } else if (verMajor < 3 || (verMajor == 3 && verMinor < 3)) {
            fprintf(stderr, "OpenGL 3.3 Core context not available (raw GL_VERSION: '%s')\n", vs ? vs : "(null)");
            return false;
        }
    }
    int attribs[5];
    glfwGetFramebufferSize(mWindow, &attribs[0], &attribs[1]);
    (void)attribs;
    gl::glViewport(0, 0, 1280, 720);
    gl::glEnable(GL_BLEND);
    gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    checkGLError("post-context-setup");

    if (!mRenderer.init()) {
        fprintf(stderr, "Renderer init failed\n");
        return false;
    }

    glm::mat4 proj = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
    memcpy(mOrtho, glm::value_ptr(proj), sizeof(mOrtho));

    if (!audio.init()) {
        fprintf(stderr, "Audio unavailable, running silent\n");
    }

    mStates[(int)AppStateId::MainMenu] = new MainMenuState();
    mStates[(int)AppStateId::SoundSettings] = new SoundSettingsState();
    mStates[(int)AppStateId::Cutscene] = new CutsceneState();
    mStates[(int)AppStateId::Gameplay] = new GameplayState();
    mStates[(int)AppStateId::Pause] = new PauseMenuState();
    mStates[(int)AppStateId::Win] = new EndMenuState(true);
    mStates[(int)AppStateId::Lose] = new EndMenuState(false);
    mActive = mStates[(int)AppStateId::MainMenu];
    mActive->enter(*this);
    return true;
}

void GameEngine::shutdown() {
    for (AppState* s : mStates) delete s;
    mStates[0] = nullptr;
    mRenderer.shutdown();
    audio.shutdown();
    if (mWindow) {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
    }
    glfwTerminate();
}

void GameEngine::switchTo(AppStateId id) {
    mPending = id;
}

InputState GameEngine::pollInput() {
    InputState in;
    int w = 1280, h = 720;
    glfwGetWindowSize(mWindow, &w, &h);
    double mx = 0.0, my = 0.0;
    glfwGetCursorPos(mWindow, &mx, &my);
    in.mouseX = (float)mx * 1280.0f / (float)(w > 0 ? w : 1);
    in.mouseY = 720.0f - (float)my * 720.0f / (float)(h > 0 ? h : 1);

    bool mouse = glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    in.clickPressed = mouse && !mPrevMouse;
    mPrevMouse = mouse;

    bool jump = glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(mWindow, GLFW_KEY_UP) == GLFW_PRESS;
    in.jumpPressed = jump && !mPrevJump;
    mPrevJump = jump;

    bool duck = glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(mWindow, GLFW_KEY_DOWN) == GLFW_PRESS;
    in.duckHeld = duck;
    bool duckJust = duck && !mPrevDuck;
    mPrevDuck = duck;

    bool esc = glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    in.escPressed = esc && !mPrevEsc;
    mPrevEsc = esc;

    bool volUp = glfwGetKey(mWindow, GLFW_KEY_EQUAL) == GLFW_PRESS;
    in.volUpPressed = volUp && !mPrevVolUp;
    mPrevVolUp = volUp;

    bool volDown = glfwGetKey(mWindow, GLFW_KEY_MINUS) == GLFW_PRESS;
    in.volDownPressed = volDown && !mPrevVolDown;
    mPrevVolDown = volDown;

    bool mute = glfwGetKey(mWindow, GLFW_KEY_M) == GLFW_PRESS;
    in.mutePressed = mute && !mPrevMute;
    mPrevMute = mute;

    bool space = glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool enter = glfwGetKey(mWindow, GLFW_KEY_ENTER) == GLFW_PRESS;
    in.skipPressed = in.escPressed || space || enter || duckJust || in.jumpPressed || in.clickPressed || in.mutePressed;
    return in;
}

void GameEngine::run() {
    double prevTime = glfwGetTime();
    const bool p2Border = getenv("NEON_P2_BORDER") != nullptr;
    const bool p2Fps = getenv("NEON_P2_DEBUG") != nullptr;
    int fpsFrames = 0;
    double fpsWindowStart = prevTime;
    while (mWindow && !glfwWindowShouldClose(mWindow)) {
        glfwPollEvents();
        double now = glfwGetTime();
        float dt = (float)(now - prevTime);
        prevTime = now;
        if (dt > 0.05f) dt = 0.05f;
        if (dt < 0.0f) dt = 0.0f;
        mGameTime += dt;

        InputState in = pollInput();
        lastInput = in;

        if ((int)mPending >= 0) {
            AppState* next = mStates[(int)mPending];
            if (mActive && next) {
                mActive->exit(*this);
                mActive = next;
                mActive->enter(*this);
            }
            mPending = (AppStateId)-1;
        }

        mActive->update(*this, dt, in);

        mRenderer.setDamageTint(0.0f);
        mRenderer.setGrayscale(0.0f);
        mRenderer.clearFrame(0.07f, 0.08f, 0.11f);
        mRenderer.beginFrame(mOrtho);
        if (p2Border) drawBorderTestQuads(mRenderer);
        mActive->draw(*this, mRenderer);
        mRenderer.endFrame();
        if (p2Fps) checkGLError("frame-end");
        glfwSwapBuffers(mWindow);
        if (p2Fps) {
            ++fpsFrames;
            double elapsed = now - fpsWindowStart;
            if (elapsed >= 1.0) {
                char title[80];
                snprintf(title, sizeof(title), "NEON OUTPOST - DMET502 Project 1  [P2 %d fps]", (int)((double)fpsFrames / elapsed));
                glfwSetWindowTitle(mWindow, title);
                fpsFrames = 0;
                fpsWindowStart = now;
            }
        }
    }
}
