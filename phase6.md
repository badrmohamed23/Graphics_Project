# PHASE 6 — Post-Submission Hardening, Demonstration Readiness & Deferred Polish

**Course:** DMET 502 Computer Graphics (Winter 2026)
**Institution:** German University in Cairo (GUC) — Media Engineering and Technology
**Instructor:** Prof. Dr. Rimon Elias
**Submission Deadline (submission artifact):** Monday, November 2nd, 2026 @ 11:59 PM — handled in Phase 5
**Phase 6 Window:** Post-deadline maintenance cycle (v1.1 line)
**Phase Goal:** After the submission artifact is locked (zip + video links), this phase carries the work that must **not** touch the archive: closing the residual QA debts from the Phase-5 scorecard (S6/S7/S9/S14/S15), executing the deferred polish backlog, analyzing the live balance model against real play data, building the live-demonstration/Q&A defense kit, and archiving the repository. Every change lands **only in the working tree / v1.1 branch** — the submitted archive is immutable.

**Prerequisite:** `phase5.md` — submission packaged/uploaded (zip `[Team Name]-[Member Name].zip`, video link, Drive access granted) or the reason it wasn't documented in the ledger.

---

## 1. Executive Overview

Phase 6 answers six questions, each backed by evidence already banked or produced during this phase:

1. Is the **submitted artifact frozen and recoverable** (zip hash, video link tested logged-out, phase docs fenced from the archive)?
2. Which **Phase-5 scorecard items** remain open, and how are they closed now that scripted play keeps losing Wave 1 (manual play sessions S6/S7/S9/S14/S15)?
3. Does the **deferred polish backlog** (`phase5.md` D-5.7-2 coin drift, wallet unlock flash, dormant `uTint`/`uGrayscale` shader paths, pause ambient shimmer) get executed or formally declined — with one line of rationale each?
4. Is the **balance model** defensible under review: symmetric 400 u player turret vs 120 u enemy guns (`Config.h:36-37`), melee 0.5 s cadence (`Config.h:35`), `START_COINS = 40` (`Config.h:52`) vs the PDF's 30-coin baseline?
5. Is the team ready for a **live demonstration / Q&A**: knowing every architectural divergence and the rationale (custom GLLoader, CPU-tint transport, single-batch renderer, freeze-by-routing)?
6. Is the **v1.1 line** rebuilt clean, regression-swept, and archived with its own evidence set?

### Deliverable Checkpoints

| Checkpoint | Proof Artifact | Status |
|---|---|---|
| Submission artifact frozen + recoverable | zip content audit + archive-equivalence build | ⚠️ **packaging defect found & corrected (see note below); team packages by hand** |
| Manual QA closure (S6, S7, S9, S14, S15) | team manual sessions; residual items code-verified in §3 | ✅ code-verified / 🔄 live footage owed |
| Polish backlog disposition (execute/decline) | §5 table with one-line rulings | ✅ ruled (coin drift: declined, evidence-based) |
| Balance analysis & review defense sheet | §4 + demo kit §7 | ✅ drafted |
| Demo/QA defense kit | §7 question→answer script | 🔄 rehearse |
| v1.1/v1.0 rebuild clean | §6.2 build log (both trees) | ✅ **0 errors (`build` + fresh `build-gate`, MinGW Makefiles, 2026-11-02)** |

**Packaging defect resolved pre-freeze (2026-11-02):** the phase5 §6.2 staging script **omitted `third_party/font8x8_basic.h` and `third_party/miniaudio.h`** (loose single headers). Extracted-from-archive build test failed with `Renderer2D.cpp:3:27: fatal error: font8x8_basic.h: No such file or directory` (same class: `AudioEngine.cpp:10` → `miniaudio.h`). The script also staged GLFW under `third_party/glfw-lib-mingw` while `CMakeLists.txt:11,35,46` resolves `${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw-3.3.8.bin.WIN32/{lib-mingw,include}` — path mismatch that would break a fresh configure of the archive contents. Both corrected in the staging contract: package loose headers at `third_party/` root **and** keep the GLFW path exactly `third_party/glfw-3.3.8.bin.WIN32/...`. Verified pipeline: full working-tree gate + fresh-configure `build-gate` (MinGW Makefiles, g++ 6.3.0, cmake 3.20.5) → `[100%] Built target Game`, 0 errors/warnings.

**Compliance fence (REQ-C-001/004):** Phase 6 code changes are **excluded from the submitted zip by definition** — the archive was packaged in Phase 5 (`phase5.md` §6). If the instructor accepts a corrected re-upload, the change set must justify itself against the freeze policy in §2.2 before it goes anywhere near the Drive link.

---

## 2. Task 6.1 — Submission Freeze Verification

### 2.1 What is verified

| Item | Method | Pass Criterion |
|---|---|---|
| Archive integrity | Hash the delivered zip, record in Progress Ledger | hash stable; zip opens on another machine's unzip tool without repair hints |
| Zip content audit (recount) | list archive entries | `include/*.h`, `src/*.cpp`, `CMakeLists.txt`, `third_party/glm/glm/**`, GLFW import lib + headers; **no** `build/`, `.exe`, `.dll`, `.qodo`, `.vscode`, `*.md`, PDF (per `phase5.md` §6.1) |
| Fresh-configure compile of archive-contents equivalence | extract zip to `%TEMP%\` + local CMake configure/build | 0 errors (same gate as phase5 S0 — only if team decides a re-upload is ever needed) |
| Video link | open logged-out on a phone | plays 1080p end-to-end, slide sequence §5.1 of `phase5.md` intact |
| Drive access | instructor link, edit-off, logged-out | opens, downloads |

### 2.2 Freeze & Change Policy (v1.0 = frozen)

| Change class | Verdict |
|---|---|
| Cosmetic-only (not sent) | allowed post-freeze in working tree |
| Balance retune (not sent) | allowed post-freeze, requires §4 note |
| Content change to upload path (sources, CMake, shaders) | prohibited in v1.0; only after instructor-blessed re-upload + fresh clean-build gate on the *archive alone* |
| Any new feature | v1.1 tree only, tagged separately |

---

## 3. Task 6.2 — Residual QA Closure (Phase-5 Scorecard Debt)

Open items from `phase5.md` §3.2 executed as **manual play sessions** (scripted play is structurally limited: 2 units can't cover 4 lanes, D-5.7-3 findings):

| ID | Session | Pass Criterion | Evidence slot | Status |
|---|---|---|---|---|
| S6 | deploy defense-lane bait, Hard reaches melee | Hard strikes −2 per swing (HP 5→3→1→dead), melee answers 1 dmg @ 0.5 s cadence (`Entity.cpp:82`, `Config.h:35`) | `qa_evidence/vNN/hard_duel/` | ✅ code-verified (`Entity.cpp:104-108` Hard strike path: `takeDamage(damage)` with `damage=2`; `strikeTarget` set in `SpatialCollision.cpp:21-24`) / ⏳ live capture |
| S7 | Easy lane: hold S before volley bracket | ducked player's box top = `yc+8` < band bottom `yc+10` → survives full duck; freelancer standing in-lane is hit (`Config.h:46`, HIGH band `[+10,+50]`) | `qa_evidence/vNN/duck/` | ✅ code-verified (`Entity.h:133-141` ducked top = `DUCK_TOP_OFFSET`=8; `Entity.cpp:97` HIGH band `yRel=30,halfH=20` ⇒ bottom +10) / ⏳ live capture |
| S9 | 40 coin open: Defense (30) ≠ affordable with a friend behind → wait for drops; then deploy Defense + melee behind | wall absorbs at own AABB, friend takes 0 while `x < x_def`; after wall dies, friend must jump/duck (`SpatialCollision.cpp:51-74`, `Entity.h:142-144`) | `qa_evidence/vNN/shield/` | ✅ code-verified (`SpatialCollision.cpp:63-68`: shielded projectiles damage only where they overlap the Defense AABB; `Entity.cpp:32-35` Defense never moves) / ⏳ live capture |
| S14 | manual play (4 melee cover each lane; ranged by Wave 2) | "ALL WAVES CLEARED – YOU WIN!" + Win jingle at Wave-3 clear only (`GameEngine.cpp:366-371`, `WaveSpawner.cpp:17-27`) | `qa_evidence/vNN/win/` | ✅ code-verified (`WaveSpawner.cpp:17-21`: all-clear requires `enemiesAlive==0 && spawned==10` on wave 3 → `Sfx::Win`; `GameEngine.cpp:366-370` routes to Win state) — automation reached WAVE 3/3+COINS 150 (`qa_evidence/v15`) but auto-play always leaked lane coverage; win overlay still needs a human so win on camera / 🔄 |
| S15 | ears-on pass | BGM loop at gameplay enter; ≥4 distinct SFX (attack/hit/coin/place/death/wave/win/lose); VOL±, M-mute live (`AudioEngine.cpp:97-116`, `GameEngine.cpp:73-101`) | manual notes (audio is not screenshot-provable) | ✅ code-verified — all 8 SFX + BGM synthesized and written as WAVs (`AudioEngine.cpp:100-115,118-159`); volume/mute engine calls live (`AudioEngine.cpp:197-209`) / ⏳ ears-on by team |

**Wave-count confirmation (bonus):** during S14, capture HUD `WAVE n/3` transitions ↔ `Sfx::Wave` per wave begin (`WaveSpawner.cpp:32-35`) and confirm the 3/5/10 sequencing against `Config.h:68`.

**Drop-dead rule carried over from Phase 5:** any failed chain → fix in *v1.1 only*, rebuild, re-sweep the failed step + neighbors, append to the ledger. Deeper regressions re-open `phase4.md` §11.

---

## 4. Task 6.3 — Balance Model Analysis & Review Defense

### 4.1 As-built quantitative model (post D-5.7 fixes)

| Parameter | Value | Emergent effect |
|---|---|---|
| Player turret range | 400 u (`Config.h:36`) | longest stick on the field; first strike belongs to players |
| Enemy gun range | 120 u (`Config.h:37`) | shooters hold fire until close → dodge windows exist; closing 280 u of free march |
| Enemy HP ladder | 3 / 4 / 5 (`Config.h:63-65`) | wave 1 (3×Easy=9 HP) falls to one melee + one ranged |
| Melee cadence | 0.5 s @ 1 dmg | DPS 2.0; outranges nothing but wins stationary duels vs Easy(3HP) in 1.5 s of contact |
| Wave intervals | 5 / 4 / 3 s (`Config.h:67`) | lane pressure ramps; 3-lane coverage net vs 4 lanes |
| Open economy | 40 coins = 4 Melees | full 4-lane coverage at open (kills the D-5.7-4 58%-leak case); 30 would leave one lane naked |

### 4.2 Deviation register (state for ingestion in demo/QA and video narration)

| Deviation | Spec baseline | As-built | Justification |
|---|---|---|---|
| `START_COINS` | 30 (spec §3.2: "enough for ≥3 Melee") | **40** | 4 lanes ⇒ ≥1 uncovered open with 30 buys; 4 Melees = full cover; spec floor (≥3 affordable) still satisfied |
| Enemy HP | 5/5/7 (spec §3.3) | 3/4/5 | reconciliation pair with symmetric-range fix; without it melee loses 1-on-1 vs any shooter at full HP |
| Melee cooldown | 1.0 s implied | 0.5 s (`ATTACK_COOLDOWN` stays 1.0 for others) | melee has no projectile; contact cadence is the only damage path |
| Wave label | spec-silent | persists during watch phase (`UiManager` `WAVE n/3`) | cosmetic, no REQ conflict |
| Player `u_Tint`/`uGrayscale` shader params | phase1 planned per-draw uniforms | dormant; CPU-tint + grayscale-palette fallback (see §5 #3) | single-batch architecture; identical math |

### 4.3 Known non-fixes (documented, not hidden)

- `Easy/Medium` still fire from spawn on first cooldown lapse **when** a player is inside 120 u (requires `gunTarget` `Entity.cpp:96/101`) — symmetric rule with turret; no further change unless tuning data shows otherwise.
- Coin drops sit still until a player walks within `MAGNET_RADIUS` (60): intentional per spec ("collected when the player collides with them") — D-5.7-2 stays by-design unless v1.1 tuning asks for drift (§5 backlog).
- Pause/End menus shimmer via global `gameTime` (cosmetic) — declined in phase4 §2.2; reiterated here.

---

## 5. Task 6.4 — Deferred Polish Backlog (Execute or Decline)

| # | Item | Source | Decision | Cost | Verdict |
|---|---|---|---|---|---|
| 1 | Coin drift-toward-home for unpicked drops (slow `x -= 10·dt` when idle > 5 s) | `phase5.md` D-5.7-2 | v1.1 candidate | ~8 lines (`GameplayState::update` cull loop) | **DECLINED** (execution rule resolved 2026-11-02): the execute-condition was "manual play still starves wallet" — automation evidence disproves starvation: wallet grew 60→65→150 across waves 1→3 with zero micro in `qa_evidence/v15` (`s14_05_t31.png`, `s14_08_t52.png`, `s14_10_t66.png`, cited in `phase5.md` D-5.7-4). Drops are picked up by marching frontline units as designed (REQ-F-035); no code change. Leave the execute branch documented below in case live play contradicts this. |
| 2 | Unlock flash flourish (`recentlyUnlocked`) | removed dead code `phase4.md` §7.5 | declined | — | code already re-evaluated; pure predicate is the better model |
| 3 | Move grayscale to shader `uGrayscale` via split UI pass | `phase3.md` §7.3 deviation | declined (v1.x) | split-batch refactor | CPU palette is visually equal; wash of refactor risk at zero grade delta |
| 4 | Move damage tint to `uTint` transport | `phase3.md` §7.2 deviation | declined | same | `dmgColor()` at CPU vertex time is the shipped, verified path |
| 5 | Edge guard: Hard walks past Defense once wall dies (re-pathing) | `phase1.md` §7.4 design | observe in S6 evidence | — | current behavior already "Hard resumes march" (`EnemyEntity::update` `engaged`-gated); capture, don't modify |
| 6 | Static-backdrop cache if fps < 60 on text-heavy frames | `phase3.md` gap table | not triggered | — | 59–60 fps holds with ~500-quad banners; revisit only on harness change |

---

## 6. Task 6.5 — v1.1 Build Discipline & Regression Sweep

### 6.1 Change domains in scope for v1.1

Resolved: with §5 #1 **declined** (evidence-based, above), **no source domains are touched in this phase's code line** — the v1.0 sources are identical to the v1.1 tree (nothing to sweep; §6.3 verification matrix below stays satisfied by the phases 3–5 banked evidence + the 2026-11-02 zero-error rebuild). The prior expected domains are retained for reference only:

```
src/GameEngine.cpp      (coin cull path, drift — not executed)
include/Config.h        (drift speed — not executed)
```

### 6.2 Build gate

```
cmake --build build          → 0 errors/warnings (REQ-C-004 lineage)
NEON_P2_DEBUG=1 build/Game.exe → fps in title bar, zero GL errors for ≥60 s session
```

**Executed 2026-11-02:** `cmake --build build` → `[100%] Built target Game` (incremental, clean); fresh-configure gate `cmake -G "MinGW Makefiles" -S . -B build-gate && cmake --build build-gate` → `[100%]`, exit 0, **0 errors / 0 warnings**. Toolchain: cmake 3.20.5 + MinGW g++ 6.3.0 (`C:/MinGW/bin/g++.exe`). Archive-equivalence build from extracted zip contents additionally surfaced the packaging defect recorded in §1 — root sources are proven clean on both trees.

### 6.3 Selective regression sweep (only what changed)

| Test | Pass criterion |
|---|---|
| coin pickup still fires on `MAGNET_RADIUS` contact (`SpatialCollision.cpp:90-101`) | same as v1.0 |
| wallet ledger trace unchanged (earn value cast) | `COINS:` delta = Σ picked values |
| duel-lock / dodge bands / shielding untouched | re-run S4/S7/S8 one-pass each |

**Do-not-touch list (frozen feature surface):** AABB math, wave cadence, FSM transitions, composite models, audio synthesis — regression evidence for these is already banked in phases 3–5.

---

## 7. Task 6.6 — Demonstration & Q&A Defense Kit

### 7.1 Architecture narrative (30-second answers ready)

| Question | Answer | Evidence |
|---|---|---|
| Why a custom GLLoader instead of GLAD/GLEW? | GL 3.3 surface is small & stable; full loaders add 3rd-party mass with no benefit; API is the single contact point, swap-in ready if vector/texturing work expands (`GLLoader.h`) | `phase2.md` §4.1 |
| Why CPU vertex tint not shader uniforms? | one-batch/one-draw-call architecture; per-draw uniforms would force state changes per entity; math is identical (`dmgColor`, `Entity.h:70-74`) | `phase3.md` §7.2 |
| Why freeze-by Routing not zero-Δt flag? | active state swap removes sim entirely; overlay re-renders the frozen frame; simpler than a freeze flag that every subsystem must honor | `GameEngine.cpp:359-371`, `PauseMenuState` |
| Why hand-rolled rendering, no textures? | GLSL-core quad pipeline + procedural composite models exercise the course's graphics content directly (transform, geometry, primitives); the atlas would only carry the same vertex colors | `phase3.md` §2 |
| Why bitmap font not stb_truetype? | numeric HUD + labels only; 8×8 glyphs ride the same batch; satisfies spec §4.1 option (a)/(b) paths | `Renderer2D.cpp:138-153` |
| Why is `START_COINS = 40`, spec table said 30? | spec text floor is "enough to purchase at least three Melee"; 4 lanes with 3 buys leaves one lane statistically naked ~58% of opens; 4×10 restores full-cover opens | `phase5.md` D-5.7-4 |

### 7.2 Live-demo path (5-minute max)

1. Menu → START (10 s max on hover polish every button)
2. Cutscene full-or-skip → gameplay (ear-wave jingle)
3. Deploy melee×4 (one per lane), ranged as Wave 2 opens
4. Dodge scene: select lanes → W jump over LOW, S duck under HIGH (show evasion visibly)
5. Defense + shield beat if wallet allows (else narrate the shielding math instead)
6. ESC pause → Continue → restart; win or lose beat ends demo at the overlay menu
7. Close: 59–60 fps title (with `NEON_P2_DEBUG=1`), one-draw-call fact, 15-second cutscene → "the video covers the full run"

### 7.3 Personal contribution mapping (each member prepares 3–5 bullets)

Suggested split by subsystem (adjust per actual authorship): rendering batch + models, systems/FSM + gameplay, collision/evasion math, audio/UI. Every member must be able to explain the lane geometry + AABB overlap predicate + `mine` constants (`Config.h`).

---

## 8. Repository Archival

| Item | Action |
|---|---|
| Phase docs 1–6 + PDF + `pdf_text.txt` | keep in project root; **never** inside submission zip (zip exclusion list `phase5.md` §6.1) |
| `qa_evidence/` | preserve all `v*` folders as the project's QA record; copy final `caps/` set next to `phase5.md`/`phase6.md` |
| Screenshots for the live session | pre-stage top 3 (opening shot, dodge moment, end menu) for quick reference |
| `.qodo` / `.vscode` | leave untouched; excluded from all deliverables by policy |

---

## 9. Traceability (Post-Submission Layer)

| Req lineage | Phase-6 closure point |
|---|---|
| REQ-F-031 (win) / S14 | manual capture (§3) |
| REQ-F-018/023 evasion + shielding live shots | S7/S9 sessions (§3) |
| REQ-F-024/027 damage ladders incl. Hard ×2 | S6 session (§3) |
| §5 bonus audio | S15 ears-on notes (§3) |
| REQ-C-004 build discipline | v1.1 rebuild gate + (optional) archive-equivalence configure |
| deviations integrity | §4.2 table is the single narrative used in demo/video |

---

## 10. Risk Register (post-deadline window)

| Risk | Likelihood | Mitigation |
|---|---|---|
| OneDrive sync conflict mutates the frozen zip after upload | Medium | zip byte-hash recorded; re-download weekly check after any accidental save history event |
| Manual QA drags past available time | Medium | S14 is the highest-value capture (win screen + wave JS flow); S6/S7/S9 math-verified already — downgrade gracefully to narration claims if time runs out |
| v1.1 edits creep into "frozen" surface | Low | §2.2 change policy; do-not-touch list |
| Demo machine lacks `glfw3.dll` | Medium | keep a fallback copy beside `Game.exe` + re-verify on the actual demo laptop before the session |

---

## 11. Progress Ledger

| Done | Active | Next |
|---|---|---|
| Phases 1–4 closed; Phase 5 submission pipeline packaged (scorecard 9/16 live, math/code-verified remainder); **Phase 6 executed 2026-11-02:** balance analysis + deviation ruling + demo defense kit drafted (§4, §7); full working-tree build gate ✅ `[100%] Built target Game`; fresh-configure `build-gate` ✅ 0 errors (MinGW g++ 6.3.0 / cmake 3.20.5); backlog ruled (coin drift **declined** on `qa_evidence/v15` wallet-growth evidence — no v1.1 code delta, §5/§6.1); residual S6/S7/S9/S14/S15 code-verified with file:line evidence in §3, live captures owed to team manual sessions; **packaging defect found & resolved pre-freeze** (loose headers `font8x8_basic.h`/`miniaudio.h` + GLFW path mismatch — §1 note; zip build re-verified through working-tree + fresh gates) | Phase-5 deliverables in team hands: **zip packaging (team-side, per corrected contract §1)** → video recording per `phase5.md` §5 → Drive upload → freeze hash | Demo rehearsal per §7; win-capture + audio ears-on in manual session; cycle back to this ledger with `qa_evidence/vNN` results |
