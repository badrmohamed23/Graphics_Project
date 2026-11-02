# PHASE 2 — Environment Preparation & Graphics Pipeline Setup

**Course:** DMET 502 Computer Graphics (Winter 2026)
**Institution:** German University in Cairo (GUC) — Media Engineering and Technology
**Instructor:** Prof. Dr. Rimon Elias
**Submission Deadline:** Friday, November 6th, 2026 @ 11:59 PM
**Phase Goal:** Turn the Phase-1 blueprint into a runnable, verified OpenGL 3.3 Core pipeline: lock the toolchain, prove window creation + GL context + shader loading + batched quad rendering + ortho projection + frame-loop timing, and audit each piece against the real codebase before building gameplay logic on top.

**Prerequisite:** `phase1.md` (requirements traceability, coordinate model, FSM, entity schema, math, economy, directory architecture).

---

## 1. Executive Overview

Phase 2 is the bridge between the Phase-1 architecture and all later gameplay phases (Phase 3+ in the 10-phase roadmap: rendering, grid/UI, entities, combat, economy, cutscene/audio, QA). It answers four engineering questions with executable proof, not intentions:

1. Is the **build environment** reproducible (compiler + CMake + prebuilt GLFW + GLM + miniaudio + bitmap font)?
2. Is the **GL 3.3 Core context** acquisition reliable (window hints, viewport, blend state, function loading)?
3. Is the **graphics pipeline** correct (GLSL 330 core compile/link, VAO/VBO batching, ortho upload, tint/grayscale uniforms)?
4. Is the **frame loop** timing-correct (Δt measurement driving the state-machine `update/draw` split)?

Every item below carries an **As-Built** column referencing the actual files in `GameEngine2D/`, because a large part of this phase is *verification of what already exists* rather than green-field work.

### Deliverable Checkpoints

| Checkpoint | Proof Artifact | Status |
|---|---|---|
| Toolchain locked | `CMakeLists.txt` + `third_party/` tree | ✅ As-built |
| GL context acquired | `GameEngine::init()` window hints + viewport + blend | ✅ As-built |
| GL loader active | `GLLoader.h/.cpp` runtime resolution | ✅ As-built |
| Shader pipeline live | `Shader::compile()` error reporting + `shader_err.txt` log trap | ✅ As-built |
| Batch renderer smoke test | `Renderer2D::beginFrame/endFrame` with `uProj` upload | ✅ As-built |
| Frame loop timing | `run()` Δt → `AppState::update/draw` | ✅ As-built |
| Version probe (readable + graceful) | `GL_VERSION`/`GL_RENDERER` printed; unparsable strings (e.g. `'Intel'`) degrade gracefully, hard-fail only on numeric < 3.3 | ✅ Implemented |
| GL-error monitor | `checkGLError()` helper (post-setup + per-frame via env) | ✅ Implemented |
| Border-quad test tool | `NEON_P2_BORDER=1` draws 1-px frame around world | ✅ Implemented |
| FPS counter | `NEON_P2_DEBUG=1` shows live fps in window title | ✅ Implemented |
| Phase-1 verification windows | §10 test table | 🔄 Visual windows need eyes-on run (tools shipped) |

---

## 2. As-Built Toolchain Audit (Locked Environment)

### 2.1 Toolchain Matrix

| Layer | As-Built Choice | Location / Evidence | Notes |
|---|---|---|---|
| Language | C++17 (`CMAKE_CXX_STANDARD 17`) | `CMakeLists.txt:4-5` | Spec-compliant |
| Compiler | MinGW g++ (Windows) | `build/` Makefiles, `lib-mingw/GLFW` | `-Wno-narrowing` suppressed for GLM float literals |
| Build system | CMake ≥ 3.15 | `CMakeLists.txt:1` | Target `Game` → `build/Game.exe` (+ auto-copied `glfw3.dll`) |
| Windowing / Input | GLFW 3.3.8 prebuilt binary | `third_party/glfw-3.3.8.bin.WIN32` | Static import lib `libglfw3dll.a` + runtime `glfw3.dll` |
| Graphics API | OpenGL 3.3 Core (`GLFW_OPENGL_CORE_PROFILE`) | `GameEngine.cpp:434-436` | GLSL `#version 330 core` in all shader sources |
| GL Loader | **Custom minimal loader** (not GLAD/GLEW) | `GLLoader.h/.cpp` | Deliberate scope-cut: only declares constants + function typedefs the project actually uses; resolved at runtime |
| Math | GLM | `third_party/glm` | `glm::ortho`, vectors, matrices |
| UI glyph rendering | `font8x8_basic.h` bitmap font | `Renderer2D.cpp` | Renders numeric HUD/health values with quads (fulfills "numerical health" display option) |
| Audio (Bonus) | `miniaudio.h` single header | `third_party`, wrapped by `AudioEngine.h/.cpp` | BGM + SFX enum `Sfx { Attack, Hit, Coin, Death, Place, Wave, Win, Lose }` |

### 2.2 Link & Post-Build Contract

```
Target: Game
  sources   : GLLoader, Shader, Renderer2D, Entity, GridSystem, WaveSpawner,
              SpatialCollision, UIManager, AudioEngine, GameEngine, Main
  include   : include/, third_party/glm, third_party, GLFW include
  link      : libglfw3dll.a, opengl32, gdi32, user32, shell32, winmm, psapi
  post-build: copy_if_different glfw3.dll → output dir   (prevents load-time failure)
```

**Risk register:**

- `glfw3.dll` must always sit next to `Game.exe` (post-build command guarantees this).
- MinGW classifies some GLM narrowing as warnings; suppressed project-wide — acceptable, but any new float literal math must still compile clean per REQ-C-004 (zero compile errors).
- `**Shader-log hygiene (RESOLVED):** the stale `shader_err.txt` (a PowerShell script error, not shader diagnostics) has been deleted; debug output now flows unbuffered to the console (`setvbuf(stdout, NULL, _IONBF)` in `Main.cpp`).

---

## 3. Task 2.1 — Window Creation & GL 3.3 Core Context

### 3.1 As-Built Behavior (`GameEngine::init()`)

| Step | Setting | Value | Requirement Behind It |
|---|---|---|---|
| 1 | Context version | `GLFW_CONTEXT_VERSION_MAJOR/MINOR = 3.3` (§ constraints) | Modern OpenGL mandate, PDF §1 |
| 2 | Profile | `GLFW_OPENGL_CORE_PROFILE` | Core-profile only per stack lock |
| 3 | Resizable | `GLFW_FALSE` | Fixed game layout; keeps world↔screen mapping trivial + stable |
| 4 | Window size | 1280 × 720, title "NEON OUTPOST - DMET502 Project 1" (§ constraints) | Matches logical world space 1:1 |
| 5 | Viewport | `glViewport(0, 0, 1280, 720)` | Full-frame mapping |
| 6 | Blend | `glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)` | Enables alpha-stratified scene art (overlays, glow) |
| 7 | Projection | `glm::ortho(0, 1280, 0, 720, -1, 1)` | Phase-1 §3.2 exact bounds |

### 3.2 Verification Tests

| ID | Test | Pass Criterion | Status |
|---|---|---|---|
| V2.1-1 | Launch `build/Game.exe` | Window opens 1280×720, non-resizable, main menu renders | ✅ Implemented |
| V2.1-2 | `glGetString(GL_VERSION)` at debug spur | Readable `GL_VERSION`/`GL_RENDERER` print; non-numeric strings (e.g. `Intel`) skip parse gracefully; numeric <3.3 hard-fails init | ✅ Verified live: `Intel` / `Intel(R) UHD Graphics` |
| V2.1-3 | Resize stress (n/a) | Window cannot be resized → mapping can't drift | ✅ Guaranteed by hint |

---

## 4. Task 2.2 — OpenGL Function Loading Strategy

### 4.1 Design Rationale (Custom Loader vs GLAD/GLEW)

The Phase-1 blueprint listed GLAD, but the as-built `GLLoader.h` substitutes a **hand-rolled minimal loader**: macro GL constants (blend enums, array-buffer tokens, shader-stage tokens, compile/link status) + function-pointer typedefs resolved at startup via `wglGetProcAddress`/`glfwGetProcAddress`.

- **Why acceptable:** the project uses a small, stable GL surface (VAO/VBO, shaders, draw arrays, blend, clear, geterror). A full loader adds 3rd-party surface area without benefit here.
- **Trade-off risk:** any newly used GL call must get a typedef + resolve entry added. Checklist for reviewers: new GL call ⇒ GLLoader entry ⇒ compile clean.
- **Fallback decision:** if the call surface grows (e.g., texturing later), regenerate with GLAD and drop-in replace — GLLoader API is the only contact point (`Renderer2D`, `Shader` go through it).

### 4.2 Verification Tests

| ID | Test | Pass Criterion | Status |
|---|---|---|---|
| V2.2-1 | All resolved pointers non-null at init | debug assert on every resolved entry | ✅ Implemented |
| V2.2-2 | `glGetError()` polling in debug | `checkGLError("post-context-setup")` at init + per-frame `checkGLError("frame-end")` via `NEON_P2_DEBUG=1` | ✅ Implemented (0x0000 confirmed clean across a 5s live run) |

---

## 5. Task 2.3 — Shader Compilation & Error-Reporting Pipeline

### 5.1 As-Built Pipeline (`Shader::compile()`)

```
source string → glCreateShader → glShaderSource → glCompileShader
            → glGetShaderiv(COMPILE_STATUS)  ⟂ false → print info log → fail
link  stage: glCreateProgram → attach both → glLinkProgram
            → glGetProgramiv(LINK_STATUS)    ⟂ false → print info log → fail
use   stage: glUseProgram per frame; uniforms: Mat4 / Vec3 / Float setters
```

### 5.2 As-Built Shader Program (single world pass)

| Uniform | Type | Consumed By | Phase-1 Contract |
|---|---|---|---|
| `uProj` | `mat4` | vertex | `glm::ortho` upload per `beginFrame()` |
| `uTint` | `vec3` | fragment | Damage reddening: `(1, 1-f, 1-f)` mapping (Phase-1 §7.5) |
| `uGrayscale` | `float` | fragment | Locked-card grayscale mix (REQ-F-036/037) |
| attrib `aPos` `vec2` | — | vertex | Quad corners |
| attrib `aColor` `vec3` | — | fragment | Procedural-model vertex color (no texture atlas needed) |

Fragment logic (documented form, not submission code):

```
c      = vColor * uTint
lum    = luminance(c)                      // 0.299R + 0.587G + 0.114B
c      = mix(c, vec3(lum), uGrayscale)     // locked-card conversion
FragColor = vec4(c, 1.0)
```

### 5.3 Verification Tests

| ID | Test | Pass Criterion | Status |
|---|---|---|---|
| V2.3-1 | Intentional-bad-source compile probe in debug | Info log printed, `compile()` returns false, no crash | ✅ Implemented |
| V2.3-2 | Live program link | No GL error after `use()` | ✅ Implemented |
| V2.3-3 | Tint sweep | Uniform sequence (0 → 1) renders progressively redder quad | ✅ Implemented (drives REQ-F-020) |
| V2.3-4 | Grayscale sweep | `uGrayscale = 1` renders luminance-only | ✅ Implemented |

---

## 6. Task 2.4 — Renderer2D Batch Pipeline & Ortho Integration

### 6.1 As-Built Batch Design

```
CPU side (per frame)
  beginFrame(ortho4x4): clear vertex buffer, use program, upload uProj,
                        reset uTint = (1,1,1), uGrayscale = 0
  ...states push quads...
  endFrame():
     glBufferData(DYNAMIC_DRAW) with accumulated vertex array
     glDrawArrays(GL_TRIANGLES, 0, vertexCount / FLOATS_PER_VERTEX)
```

- **Layout:** `aPos` (2 floats) + `aColor` (3 floats) → 5 floats/vertex; quads expanded to 2 triangles on the CPU (documented elsewhere in `Renderer2D.cpp`).
- **One draw call** per frame (world pass; UI/glyph quads ride the same batch) — knowingly inverts the Phase-1 "separate UI shader" plan; accepted because there is no texture atlas and the uniform set is shared. Grade-neutral; revisit only if UI needs per-pass state that conflicts with the world pass.
- **Clear color** set by each state (`clearFrame(r,g,b)`) so menus/cutscene/gameplay can set the mood independently.

### 6.2 Damage Feedback Wiring (Phase-2 hook of REQ-F-020)

`Renderer2D::setDamageTint(f)` clamps `f ∈ [0,1]` and sets `uTint = (1, 1-f, 1-f)`. Engine code computes `f` per entity from `1 − HP_cur/HP_max` (Phase-1 §7.5) before pushing that entity's quads. Entity-correctness (monotonic reddening per hit) is verified in Phase-5 windows, but the transport layer is proven here.

### 6.3 Verification Tests

| ID | Test | Pass Criterion | Status |
|---|---|---|---|
| V2.4-1 | 1-px border quad at [0,0]–[1280,720] via the batch (`NEON_P2_BORDER=1`) | Fully visible (no clipping/flip errors) | ✅ Tool live — one eyes-on confirmation pending |
| V2.4-2 | Known quad → known color | Screen pixel matches pushed vertex color | ✅ implemented by visual smoke test |
| V2.4-3 | Thousands-of-quads stress (ball-park scene load) | sustained ≥ 60 fps, no dropped frames | 🔄 use `NEON_P2_DEBUG=1` title fps during Phase-3 load |
| V2.4-4 | Blend check | Semi-transparent overlay draws correctly over background | ✅ implemented (enabled globally) |

---

## 7. Task 2.5 — Frame Loop, Δt Measurement & State Routing

### 7.1 As-Built Loop (`GameEngine::run()`)

```
while (!windowShouldClose)
    glfwPollEvents()               → InputState { mouse, click, jump, duck, esc, skip, vol… }
    compute dt from wall clock
    if pause/freeze semantics active → dt = 0      // stage hook for REQ-F-032 (Phase-7/8 work)
    activeState->update(*this, dt, input)          // sim step
    activeState.draw(*this, renderer)              // render step — never mutates sim
    glfwSwapBuffers()
```

- **State table:** `AppStateId { MainMenu, SoundSettings, Cutscene, Gameplay, Pause, Win, Lose }` — the Phase-1 FSM minus the future `STATE_*` synonyms; transitions implemented by `switchTo()`.
- **Edge-trigger discipline:** pressed-input edges (click/jump/esc/volume) diffed against previous-frame state (`mPrevMouse`, `mPrevJump`, `mPrevEsc`, …) so key-repeat can't double-fire placement or pause.
- **Δt correctness rule:** all gameplay code consumes Δt additively (`x += v·dt`), never "per frame" — guarantees the 60→30 fps invariance of movement and cooldowns.

### 7.2 Verification Tests

| ID | Test | Pass Criterion | Status |
|---|---|---|---|
| V2.5-1 | Movement trace under variable load | melee Δx over T = SPEED_MELEE·T within 1% | 🔄 Phase-5 re-run |
| V2.5-2 | FPS-drop invariance | game logic speed identical at heavy scene load | 🔄 stress later |
| V2.5-3 | Edge test | one click = exactly one lane toggle (no bounce) | ✅ implemented via prev-state diffing |

---

## 8. Pipeline Data-Flow (As-Built)

```mermaid
flowchart LR
    GLFWev[GLFW events] --> GE[GameEngine.run]
    Clock[wall clock dt] --> GE
    GE -->|InputState + dt| ST[active AppState]
    ST -->|sim updates| GLS[GridSystem, Entities, WaveSpawner, Collision, UI, Wallet, Audio]
    ST -.quads.-> R[Renderer2D]
    ST -.clear color.-> R
    R -->|uProj, uTint, uGrayscale| SH[Shader GLSL 330 core]
    SH --> LO[GLLoader] --> GLC[GL 3.3 Core context]
    R --> SW[glfwSwapBuffers]
```

---

## 9. Regression & Build Discipline (REQ-C-002/004 Hooks)

1. **Zero-compile-error gate:** verified — `cmake --build build` completes clean (GameEngine, Main, full link).                Re-run before any commit.
2. **`glGetError` polling:** `checkGLError()` after context setup and per frame when `NEON_P2_DEBUG=1`.
3. **Log hygiene (done):** `shader_err.txt` removed; runtime diagnostics print unbuffered to console.
4. **Composite-model reminder:** Phase-2 proves quads pipeline; *scene quality* (composite primitives per REQ-C-002) is a Phase-3 measure — "single-primitive-only" components must not survive Phase-3 review.

---

## 10. Phase-1 → Phase-2 Traceability (Pipeline Layer)

| Req ID | Pipeline Hook Established In Phase 2 |
|---|---|
| REQ-F-020 | `uTint` damage-reddening uniform + clamped setter transport |
| REQ-F-021 | `font8x8` glyph pipeline (numeric HUD/health rendering path) |
| REQ-F-035 | HUD quads ride the same batch (coin total display path live) |
| REQ-F-036/037 | `uGrayscale` uniform + per-state clear color for card UI |
| REQ-F-038/039/040 | 7-state table wired fully into `AppState` polymorphism |
| REQ-F-004 | Fixed 1280×720 → `X_HOME` mapping cannot drift (non-resizable) |
| REQ-C-003 | Window/title chains to any screen-capture setup used for the video |

---

## 11. Progress Ledger

| Done | Active | Next |
|---|---|---|
| Phase-1 blueprint; Phase-2 environment + GL pipeline **as-built, compiled clean and live-verified** (window/context ✓, GL_VERSION probe ✓ — graceful `Intel` handling, loader ✓, `checkGLError` monitor ✓, shader errors handled ✓, batched ortho renderer ✓ with border-test tool, Δt loop ✓, 7-state FSM ✓, fps counter ✓, shader log cleaned ✓) | One eyes-on sweep: run `NEON_P2_BORDER=1` once and confirm the red 1-px world frame renders | Phase 3 — Core Rendering Engine & 2D Primitive Modeling: composite geometric primitives per unit (≥4 parts each, REQ-C-002), ≥5 environmental decor elements (REQ-F-034), Home-Base distinct model + health-bar/coin HUD authored on `font8x8` path |
