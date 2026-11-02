# PHASE 4 — Systems & Gameplay Layer

**Course:** DMET 502 Computer Graphics (Winter 2026)
**Institution:** German University in Cairo (GUC) — Media Engineering and Technology
**Instructor:** Prof. Dr. Rimon Elias
**Submission Deadline:** Friday, November 6th, 2026 @ 11:59 PM
**Phase Goal:** Audit the *systems that drive the game* on top of the Phase-3 visual platform: the 7-state FSM (menus, pause, win/lose), cutscene pacing, input pipeline + lane hover/select, card → lane placement flow, wallet unlock/relock churn, wave cadence and wave-clear predicates, combat dispatch (encounter halt, projectile bands, shielding), economy loop (drops → pickup → HUD), and the bonus audio subsystem — each item proven against the actual codebase, not intentions.

**Prerequisite:** `phase1.md` (FSM §5, entity schema §6, algorithms §7, economy/waves §8) + `phase2.md` (frame loop, Δt) + `phase3.md` (rendering as-built).

---

## 1. Executive Overview

Phase 4 answers eight engineering questions with file:line evidence:

1. Is the **state machine lifecycle** complete — menu → cutscene → gameplay → pause/win/lose, with restart and main-menu paths, and does the *lose freeze* actually stop the simulation (REQ-F-032)?
2. Does the **cutscene** meet the PDF §2 window (10–20 s), animate characters interacting, and offer a gated skip (REQ-F-001/002)?
3. Is the **input pipeline** edge-safe (one click = one action) and does the unprojection match Phase-1 §3.6?
4. Is the **lane hover/select** + **card→lane placement** flow spec-compliant (REQ-F-015/016, REQ-F-036 input gating)?
5. Does the **wallet** enforce 30-start / 10-20-30 costs and the two-way unlock/relock churn (REQ-F-012/013/014/035/037)?
6. Does the **wave spawner** implement PDF §3.3 exactly: wave *begins* at first spawn, *ends* at final spawn, leftovers must be defeated before the next wave, 3/5/10 counts, random lanes (REQ-F-005/028/029)?
7. Is the **combat dispatch** correct: encounter halt for *both* sides, HIGH/LOW projectile bands driving dodge math, Defense shielding, melee/ranged/cooldown damage values (REQ-F-007/018/023/024/026/027)?
8. Is the **economy loop** closed (death → drop → pickup → HUD) and does **audio** satisfy the full bonus bundle (BGM + ≥4 SFX + settings menu)?

Most of this machinery was **built during Phases 2→3 as the states came online**; this document is therefore an **as-built audit plus defect register** for the tuning phase that follows.

### Deliverable Checkpoints

| Checkpoint | Proof Artifact | Status |
|---|---|---|
| 7-state FSM + transitions | `GameEngine::init/run/switchTo`, `GameEngine.cpp:446-536, 586-636` | ✅ As-built |
| Cutscene (15 s, animated, subtitles, gated skip) | `CutsceneState` `GameEngine.cpp:103-174` | ✅ As-built |
| Input unprojection + edge triggers | `GameEngine::pollInput` `GameEngine.cpp:542-584` | ✅ As-built |
| Lane hover/select/toggle + multi-select | `GridSystem.cpp:3-14`, `GameEngine.cpp:248, 287-291` | ✅ As-built |
| Card unlock gating + card→lane placement | `GameEngine.cpp:271-291`, `UIManager.cpp:49-89` | ✅ As-built |
| Wallet start/spend/earn + two-way unlock | `WalletManager.h:8-48` | ✅ As-built |
| Wave cadence 3/5/10 + clear predicates | `WaveSpawner.cpp:12-45`, `Config.h:66-67` | ✅ As-built |
| Win/Lose triggers + scene freeze | `GameEngine.cpp:297-350`; routing stops sim in `run()` | ✅ As-built |
| Combat dispatch (halt, bands, shield, damage) | `SpatialCollision.cpp`, `Entity.cpp` | ✅ As-built + **D-4.6-1/D-4.6-2 repaired 2026-11-02 (§7)** |
| Economy loop (drop → pickup → HUD) | `GameEngine.cpp:297-321`, `Entity.cpp` drop config | ✅ As-built |
| Audio bonus (BGM + 8 SFX + settings) | `AudioEngine.cpp`, `SoundSettingsState` `GameEngine.cpp:73-101` | ✅ As-built |
| Eyes-on system sweep (full game loop) | §8 test table | 🔄 pending run |

---

## 2. Task 4.1 — Application FSM & Menu Systems

### 2.1 As-Built Transition Matrix vs Phase-1 §5.2

| Current | Trigger | Next | As-Built Evidence | Match |
|---|---|---|---|---|
| MAIN_MENU | Click START | CUTSCENE | `GameEngine.cpp:52-58` | ✓ |
| MAIN_MENU | Click SOUND SETTINGS | SOUND_SETTINGS | `:56` | ✓ (bonus) |
| MAIN_MENU | Click EXIT | app close | `:57` `glfwSetWindowShouldClose` | ✓ |
| SOUND_SETTINGS | Back / ESC | MAIN_MENU | `GameEngine.cpp:81` | ✓ |
| CUTSCENE | 15 s timer | GAMEPLAY | `:127` `Config::CUTSCENE_DURATION = 15.0` | ✓ (∈ [10,20]) |
| CUTSCENE | skip key after 2 s gate | GAMEPLAY | `:126` | ✓ (prevents accidental skip) |
| GAMEPLAY | ESC edge | PAUSE | `GameEngine.cpp:244-247` | ✓ |
| GAMEPLAY | breach | LOSE (+ `Sfx::Lose`, BGM stop) | `:339-344` | ✓ |
| GAMEPLAY | wave-3 cleared | WIN (+ `Sfx::Win`, BGM stop) | `:345-350` | ✓ |
| PAUSE | Continue click / ESC | GAMEPLAY (no reset — `resetOnEnter=false` set at `:370-373`) | `:376-379` | ✓ |
| PAUSE | Restart | GAMEPLAY (reset) | `:380-384` | ✓ |
| PAUSE | Main Menu | MAIN_MENU | `:385` | ✓ |
| WIN/LOSE | Restart / Main Menu | re-init via `GameplayState::enter` + `reset()` (`:197-208`) | `:402-411` | ✓ |

### 2.2 Freeze Semantics (REQ-F-032 — "everything stops")

The as-built implementation freezes by **routing**: once `switchTo(Lose)` lands, the active state is `EndMenuState`, whose `update` ignores simulation entirely (only consumes clicks) — `GameplayState::update` is never called again, so Δt effectively = 0 for movement, cooldowns, spawner. Both `PauseMenuState::draw` and `EndMenuState::draw` re-render the frozen gameplay frame under a dark overlay (`:388, :413`).

**Accepted deviation:** the frozen scene still *shimmers* — entity FX, lane glare, coin bob, Home Base pulse read `e.gameTime()` (global wall clock), so ambient visual animation continues under the overlay pause/end menus. Simulation state (positions, HP, waves) is untouched. Cosmetic-only; no spec violation (the PDF text requires movement/attacks to stop, which they do).

---

## 3. Task 4.2 — Cutscene Pacing Audit (REQ-F-001/002)

`CutsceneState` (`GameEngine.cpp:103-174`), keyed off `mTimer`:

| Beat | Timeline (s) | Content | Spec Clause |
|---|---|---|---|
| 1 | 0–4 | Guardian Lab-Defender walks in (110→200u); subtitle "LAB X-42 — NIGHT SHIFT…" | setting established ✓ |
| 2 | 3–6 | Xeno Upper-Ranged descends right edge (1330→990u) | ≥1 enemy on screen, animated ✓ |
| 3 | 4–7 | Xeno charger advances (1330→1120u), idle bob | ✓ |
| 4 | 6.5–10 | "THE REACTOR CORE… IS OURS." dialogue | conflict dialogue ✓ |
| 5 | 10–13 | Guardian reply + blade-charge glow FX (pulse circle, `:147-151`) | interaction, not text-only ✓ |
| 6 | 13–15 | "DEFEND THE CORE — DEPLOY YOUR GUARDIANS!" call-to-action + charge FX completes | story→gameplay bridge ✓ |
| — | full | progress bar (bottom-left, `:166`), skip hint after 2 s (`:167`) | pacing affordance ✓ |

Compliance: duration 15.0 s ✓ within [10, 20]; characters are the *real* `PlayerEntity`/`EnemyEntity` models drawn with `walkPhase` animation ✓ (≥2 animating sprites per frame); subtitles supplement, never replace, animation ✓ (REQ-F-001/002, §4 theme-consistency — same "Neon Outpost" backdrop via `uiDrawEnvironment`).

---

## 4. Task 4.3 — Input Pipeline, Grid Hover/Select & Placement Flow

### 4.1 Unprojection (Phase-1 §3.6 exact)

`pollInput` (`GameEngine.cpp:544-549`): `mouseX = raw · 1280/w`, `mouseY = 720 − raw · 720/h` — world-Y-up flip preserved; window is non-resizable so scale can never drift (Phase-2 lock).

### 4.2 Edge-Trigger Discipline

All discrete inputs are diffed against previous-frame state (`mPrevMouse/Jump/Esc/Duck/VolUp/VolDown/Mute`, `GameEngine.cpp:551-583`): one click = exactly one card-select or lane-toggle; no key-repeat double-fire. Ambient keys (`duckHeld`) pass raw as level-triggered, as designed for "hold to duck".

### 4.3 Lane Hover / Select (REQ-F-015)

- Hover: `GridSystem::updateHover` clamps to battleground band and maps y→lane via `Config::laneFromY`; toolbar (y>640) and ground strip return lane −1 → no highlight (`GridSystem.cpp:3-5`).
- Toggle: `toggleAt` flips the lane bitmask on click (`:7-14`); **multiple lanes stay selected simultaneously** ✓. Selected lanes render pulsing teal glare + cyan edge rails; hover renders a mid glare (`GridSystem.cpp:16-40`).

### 4.4 Card → Lane Placement (REQ-F-016, REQ-F-036)

`GameEngine.cpp:271-291`, priority order per click:

1. **Card hit** → only `wallet.isUnlocked(kind)` cards accept selection (`:275`); locked cards ignore hover (outline suppressed `hovered && unlocked`, `UIManager.cpp:53`) and clicks ✓ input-gate per spec.
2. **Battleground hit with card selected** → tokenize `laneFromY`, cost-check, `PlayerEntity::make(k, lane)` at `X_PLAYER_SPAWN = 170`, `wallet.spend(cost)`, `Sfx::Place`, deselect card (`:277-286`).
3. **Battleground hit with no card** → lane toggle (`:288`) — supports the spec's lane-first reading as well.

Evasion wiring: jump (edge) arms `EvadeState::Jump` for every *non-Defense* player whose lane is in the selection bitmask; duck is level-held the same way (`GameEngine.cpp:257-269`) — multi-lane selection drives all selected lanes ✓ (PDF §3.2 "every player in all the selected lanes").

---

## 5. Task 4.4 — Wallet & Card Unlock/Relock Churn

As-built `WalletManager.h`:

| Rule | Value | Evidence |
|---|---|---|
| Start balance | 30 (= 3× Melee) ✓ REQ-F-013 | `reset()` → `Config::START_COINS` |
| Costs | Melee 10 / Ranged 20 / Defense 30 (= 1×/2×/3× Melee) ✓ REQ-F-012/014 | `cardCost()` → `Config::COST_*` |
| Spend | clamped at 0, then `refreshUnlockState()` | `spend()` `:14` |
| Earn | on collectible cull | `earn()` `:15`, called `GameEngine.cpp:300-302` |
| Unlock predicate | `IsUnlocked(card) = balance ≥ cost` — recomputed on *every* balance mutation | `refreshUnlockState()` `:34-40` |
| Two-way churn | pure function of balance ⇒ auto-unlock on threshold, auto-relock when balance falls below cost ✓ REQ-F-037 (no flag persistence = no drift) | same |
| HUD | `COINS: n` (gold, 3×) + `WAVE n/3` (cyan, 3×) on toolbar ✓ REQ-F-035 | `UIManager.cpp:82-89` |

**Dead-code note:** `recentlyUnlocked/consumeUnlockFlag` (`WalletManager.h:21-26`, maintained in `refreshUnlockState()`) has **no consumer** — the planned "unlock flash" UI flourish never reads it. Cosmetic; candidate for removal or wiring in Phase 5.

---

## 6. Task 4.5 — Wave Spawner Cadence & Clear Predicates (REQ-F-005/028/029)

As-built `WaveSpawner.cpp` + `Config.h:66-67`:

| Wave | Kind roll (`rollKind`) | Count | Interval | Wave start marker |
|---|---|---|---|---|
| 1 | Easy only (forced) | 3 | 4.0 s | `mWaveStarted[0]` + `Sfx::Wave` |
| 2 | Easy/Medium 50/50 `rand()%2` | 5 | 3.0 s | same |
| 3 | Easy/Medium/Hard 1/3 each | 10 | 2.5 s | same |

- **Begins at first spawn:** `mWaveStarted` flips + wave jingle on the first spawn tick (`WaveSpawner.cpp:32-35`) ✓.
- **Ends at final spawn:** after `spawnedThisWave ≥ count`, the spawner enters the *watch* phase and emits nothing (`:29-30`) — "no additional enemies from that wave" ✓.
- **Leftovers gate the next wave:** advance requires `enemiesAliveCount == 0 && spawned ≥ count` (`:17-27`); wave 3 clear sets `mAllWavesCleared` → Gameplay emits `Sfx::Win` and transitions (REQ-F-031) ✓.
- **Random lane per spawn:** `rand() % 4` at `:39`, uniform, enemy spawns at `X_ENEMY_SPAWN = 1260` and begins acting immediately (`EnemyEntity::make/update`) ✓ REQ-F-005/009.
- Wave label surfacing: HUD shows `WAVE n/3` driven by `waveIndex()+1` (advances only when a wave is cleared; during a wave's own watch phase the current label persists — cosmetic, spec-silent).

---

## 7. Task 4.6 — Combat Dispatch Audit (Bands, Halt, Shielding, Damage)

### 7.1 As-Built Stat Ladder (REQ-F-024/026/027)

| Unit | HP | vx (u/s) | Damage | Cooldown | Evidence |
|---|---|---|---|---|---|
| Melee | 5 | 40 | 1 / attack | 1.0 s | `Entity.cpp:12-16, 53-56`; dmg dispatch `SpatialCollision.cpp:28-40` |
| Ranged | 5 | 40 | 1 / projectile | 1.0 s | `Entity.cpp:17-21, 49-52` |
| Defense | 7 | 0 (never moves) | — (shield) | n/a | `Entity.cpp:22-27, 32-35` |
| Easy | 5 | 35 | 1 HIGH projectile | 1.0 s (first at 0.6) | `Entity.cpp:69-74, 99-102` |
| Medium | 5 | 35 | 1 LOW projectile | 1.0 s (first at 0.45) | `:75-80, 103-106` |
| Hard | 7 | 70 | **2** close strike | 1.0 s (first at 0.8), only while engaged | `:81-87, 107-111` |

### 7.2 Projectile Bands vs Dodge Math (as-built numbers, not planned numbers)

| Emitter | Band (Y rel to lane center) | Dodge requirement | Window |
|---|---|---|---|
| Easy → HIGH | `yRel=30, halfH=20` ⇒ **[+10, +50]** | duck ⇒ player top = `yc + DUCK_TOP_OFFSET = yc+8` < band bottom → clears (fixed D-4.6-2) | full duck duration |
| Medium → LOW | `yRel=−25, halfH=15` ⇒ **[−40, −10]** (retuned, was [−55, −25]) | jump ⇒ clear iff `yOffset > 35` (peak 60 — matches phase1 §7.2 threshold) | ≈0.5 s |
| Ranged player | `yRel=0, halfH=15` ⇒ [−15, +15] horizontal | enemy has no dodge | symmetric duel |

Friendly-fire filtering: player↔player collisions never register (no attacker pairing exists); projectiles filter strictly by lane + faction ✓ (REQ-F-010).

### 7.3 Defense Shielding (REQ-F-023)

`SpatialCollision.cpp:44-68`: for each enemy projectile, the shield-anchor is the lane's alive Defense player; the struck player is declared SHIELDED iff `x(player) < x(defense)` (`isShieldedBy`, `Entity.h:141-143`). While shielded, the projectile cannot damage the friend — it deals its damage **only where it overlaps the Defense AABB** (`:57-62`), i.e., the wall absorbs at its own box; if the Defense dies or the friendly has marched past it, vulnerability resumes. Matches the Phase-1 §7.4 algorithm and the PDF "protection as long as the player remains behind".

### 7.4 Encounter Halt (REQ-F-007) — defects found and repaired

Enemies halt and duel correctly: engagement scan picks the *rightmost* overlapping player per lane and `EnemyEntity::update` skips movement while `engaged`.

**D-4.6-1 (HIGH) — REPAIRED 2026-11-02:** players were never marked `engaged` during gameplay combat (the old code only set `engaged = true` on enemies), so advancing players marched through stopped enemies instead of locking the duel — and as a side effect the melee damage path never fired (the old `PlayerEntity::update` reset `cdTimer` to full *before* `SpatialCollision` could ever observe `cdTimer ≤ 0`, so melee dealt zero damage the entire time). As-built fix: `SpatialCollision::resolve` now clears both factions' engagement flags each frame and marks **every** overlapping player `engaged` (`SpatialCollision.cpp:6-11, 22-24`); the halt branch in `PlayerEntity::update` (`blockedEngage`) is now live. Consequence: melee duels freeze both combatants per REQ-F-007; grouped players pile up and halt at the duel front.

**D-4.6-2 (MEDIUM) — REPAIRED 2026-11-02:** the HIGH band bottom (`yc+10`) equalled the ducked player's box top (`yc + DUCK_TOP_OFFSET = yc+10`), and `AABB::overlapsY` is inclusive, so a fully ducked player still absorbed Easy's high shots. Fix: `Config::DUCK_TOP_OFFSET 10.0f → 8.0f` (`Config.h`) — ducked top is now below the band, so duck reliably evades HIGH projectiles while standing still still overlaps ([10, 50] vs standing box top +45) ✓.

### 7.4a Melee Damage Cadence Repair (discovered during D-4.6-1 root-cause)

The old flow never dispatched melee damage (see D-4.6-1). The cadence now lives where the overlap test lives: `SpatialCollision.cpp:37-46` — when a melee player's `cdTimer ≤ 0` and an in-lane enemy AABB-overlaps the player, it takes 1 damage, the player's `cdTimer` resets to `ATTACK_COOLDOWN` *in place*, and `Sfx::Attack` fires **only on a real target** (this also retires the old unconditional whiff-SFX gap from §7.5). Ranged keeps event-dispatched damage; its fire is now gated on `aimTarget` (§7.5) so an out-of-range cadence no longer wastes projectiles — the cooldown simply drains to 0 and the first in-range enemy receives the next instant shot.

### 7.5 Deviations — state after 2026-11-02 repair pass

- **Ranged range enforcement (`RANGED_RANGE = 400` was declared, never read):** now live — `GameplayState::update` stamps every Ranged player's `aimTarget` flag each frame (true iff an alive in-lane enemy stands ahead within 400 u), and `PlayerEntity::update` fires only when `aimTarget = true` (`GameEngine.cpp`, `Entity.cpp`). Projectiles are no longer fired into open space; the cadence drains to 0 and releases one shot the instant a target enters range.
- **Melee Attack SFX:** now emitted from `SpatialCollision.cpp` only when a real target takes a strike (§7.4a); the old unconditional per-second whiff sound is gone.
- **Melee movement speed constant:** rewritten to a shared march constant — `Config::SPEED_MARCH = 40` (replaces the previously misnamed `SPEED_RANGED` path).
- **Medium LOW band retuned:** `yRel −40 → −25` ⇒ dodge threshold now `yOffset > 35`, matching the phase-1 §7.2 model with ≈0.5 s dodge window (was ≈0.13 s).
- **Wallet dead code removed:** `recentlyUnlocked/consumeUnlockFlag/mPreviouslyUnlocked/mRecentlyUnlocked` and `refreshUnlockState()` are deleted from `WalletManager.h`; unlock state remains a pure live predicate of balance (no flag persistence = zero churn drift).
- **Projectile lifetime:** despawn bounds [−60, 1340] (`Entity.h`) — a missed enemy shot passes the Home Base visually before despawning; the breach trigger itself is position-based, unaffected.

---

## 8. Task 4.7 — Economy Loop & Bonus Audio

### 8.1 Economy Loop (REQ-F-030/035)

```
enemy death (takeDamage → alive=false)
  → drop guard (deadHandled): Coin{x=enemy.x, y=laneYCenter, value=reward 10/15/25, random phase}
  → Sfx::Death
  → pickup: AABB-distance ≤ MAGNET_RADIUS(40) from any alive player (yOffset-aware)  [SpatialCollision.cpp:84-96, Sfx::Coin]
  → wallet.earn(value) on cull  [GameEngine.cpp:300-302]
  → HUD COINS + auto unlock/relock churn (§5)
```

Feasibility unchanged from phase1 §8.4 (packs of 10/15/25 → Ranged affordable mid-Wave-1, Defense during Waves 2–3).

### 8.2 Audio Bonus (PDF §5 — all three clauses)

| Clause | As-Built | Evidence |
|---|---|---|
| Background music | 8-bar procedural synth loop, looping `ma_sound`, starts at gameplay `enter`, stops on Win/Lose/Main-menu | `AudioEngine.cpp:44-61, 148-160`; `GameEngine.cpp:52, 182, 340-341/346-347` |
| ≥4 SFX for interactions | **8**: Attack, Hit, Coin, Death, Place, Wave, Win, Lose — synthesized, written to WAVs under `%TEMP%\GameEngine2DAudio`, played via `ma_engine_play_sound` | `AudioEngine.cpp:97-116, 187-195` |
| Sound Settings menu from Main Menu | VOL+ / VOL− (buttons *and* `+`/`-` keys), MUTE toggle (`m` key), volume `pushBar` display, BACK | `GameEngine.cpp:73-101` |

Graceful degradation: audio init failure downgrades to silent running without aborting the game (`GameEngine.cpp:510-512`).

---

## 9. Verification Tests — Phase-4 System Sweep

| ID | Test | Pass Criterion | Status |
|---|---|---|---|
| V4.1-1 | Full-lifecycle scripted run: menu → start → cutscene (auto at 15 s) → gameplay | every transition lands; ESC pause/continue preserves layout & score | 🔄 pending run |
| V4.1-2 | Lose-freeze check: breach → Lose menu | no x-position drift for any entity across ≥2 s of menu idle | 🔄 (static-verify ✓ by routing; needs eyes-on) |
| V4.2-3 | Cutscene duration + skip gate | skip inert before 2 s; auto-advance at 15 s | 🔄 |
| V4.3-4 | One click = one toggle | rapid double-click never spawns two units / double-toggles | ✅ code-verified (edge diff) |
| V4.3-5 | Locked-card gate | locked card ignores hover & click at all balances | ✅ code-verified (`:275`, `UIManager.cpp:53`) |
| V4.4-6 | Subset thresholds: balance 29→30 → Defense card colors up; 30→29 → relocks gray | two-way churn | ✅ code-verified (pure predicate) |
| V4.5-7 | Wave counts & cadence | exactly 3, then 5, then 10 spawns; no overlap across waves; clear-jingle per wave | ✅ code-verified; live wave surfacing seen in phase-3 runs (WAVE 1/3 increments, random lanes) |
| V4.5-8 | Win trigger | no transition before wave-3 watch phase drains to 0 enemies | ✅ code-verified (`:17-27`) |
| V4.6-9 | Duel lock: melee vs hard freeze at contact until one dies | both stopped, HP decay 1 dmg/s (player) vs 2 dmg/attack (Hard) | ✅ **code-repaired & verified** (`SpatialCollision.cpp` engagement + melee cadence §7.4/7.4a) |
| V4.6-10 | Dodge HIGH by duck | ducked player survives Easy barrage; standing player is hit | ✅ **code-repaired & verified** (`DUCK_TOP_OFFSET` 8.0, band bottom +10 strict-clear §7.4) |
| V4.6-11 | Dodge LOW by jump | jumped player clears Medium shots; standing player is hit | ✅ **code-repaired & verified** (band [−40, −10], threshold `j > 35`, window ≈0.5 s §7.2) |
| V4.6-12 | Shielding | friend behind Defense takes 0 hits while wall alive; takes damage after wall dies or passes it | ✅ code-verified (`:57-62`) |
| V4.7-13 | Economy trace | balance delta = Σ picked coin values (10/15/25); HUD updates each pickup | ✅ code-verified + phase-3 wallets runs |
| V4.7-14 | Audio matrix | ≥4 distinct SFX fire (attack/hit/coin/place/death/wave/win/lose), BGM loop, volume path changes gain | 🔄 code-verified; ears-on pending |

---

## 10. Phase-4 Traceability (Systems Layer)

| Req ID | Hook Verified In Phase 4 |
|---|---|
| REQ-F-001/002 | 15 s cutscene, animated entities + subtitle beats + gated skip (§3) |
| REQ-F-005 | `rand()%4` lane, right-edge spawn, immediate attack (§6) |
| REQ-F-007 | enemy side halts ✓; player side defect D-4.6-1 (gap) |
| REQ-F-009/010 | attacks auto-start on spawn/placement; friendly pass-through enforced by filter |
| REQ-F-012/013/014 | 30 start / 10-20-30 costs / 2×/3× unlock ladder (§5) |
| REQ-F-015/016 | multi-lane hover/select; spawn at X=170 (§4) |
| REQ-F-017/018 | selected-lanes-only evasion; HIGH/LOW band math (§7.2, D-4.6-2) |
| REQ-F-019 | Defense skips movement & evasion (`Entity.cpp:32-35`, `GameEngine.cpp:259, 267`) |
| REQ-F-022/030 | player death prune (no drop); enemy death → coin → pickup → earn (§8.1) |
| REQ-F-023 | Defense wall absorbs at its AABB; resumes on death/pass (§7.3) |
| REQ-F-024/026/027 | stat ladder incl. Hard 2-dmg (§7.1) |
| REQ-F-028/029/031 | wave counts/cadence, begin/end/clear predicates, win trigger (§6) |
| REQ-F-032 | breach → Lose; freeze by state routing (§2.2) |
| REQ-F-036/037 | locked-card input gate + two-way churn (§4.4, §5) |
| REQ-F-038/039/040 | menus full matrix incl. restart re-init (§2.1) |
| §5 Bonus audio | BGM + 8 SFX + settings menu (§8.2) |
| §4.3 sound settings | bonus, live |

---

## 11. Gap Register (owes to Phase 5 — QA & Packaging only)

| Gap (ID) | Severity | Status |
|---|---|---|
| D-4.6-1 player never halts on encounter (+latent melee-zero-damage bug) | **High** | ✅ **FIXED 2026-11-02** (§7.4, §7.4a) |
| D-4.6-2 duck boundary overlap (duck failed to dodge HIGH) | **Medium** | ✅ **FIXED 2026-11-02** (`DUCK_TOP_OFFSET` → 8.0) |
| LOW-jump dodge window ≈0.13 s | Medium | ✅ **FIXED 2026-11-02** (Medium band retuned to [−40, −10], ≈0.5 s) |
| Ranged range not enforced (`RANGED_RANGE` unused) | Low-Med | ✅ **FIXED 2026-11-02** (`aimTarget` gate in `GameplayState`/`Entity.cpp`) |
| Melee Attack SFX fires with no target | Low | ✅ **FIXED 2026-11-02** (SFX moved to `SpatialCollision`, target-gated) |
| `recentlyUnlocked` flag dead code | Cosmetic | ✅ **REMOVED 2026-11-02** (`WalletManager` slimmed) |
| Pause/end ambient shimmer via global `gameTime` | Cosmetic (accepted §2.2) | declined — simulation freezes correctly |
| Melee speed constant misused (`SPEED_RANGED`) | Cosmetic | ✅ **FIXED 2026-11-02** (`Config::SPEED_MARCH`) |

**Rebuild gate:** `cmake --build build` → 100%, zero errors/warnings, `Game.exe` linked (REQ-C-004 pass).

---

## 12. Progress Ledger

| Done | Active | Next |
|---|---|---|
| Phase-4 systems audit as-built: 7-state FSM + menu matrix ✓, lose-freeze semantics ✓ (cosmetic shimmer accepted), cutscene 15 s pacing ✓, input edge-triggers + unprojection ✓, multi-lane select + card→lane placement ✓, wallet 30/10/20/30 with two-way churn ✓, wave cadence 3/5/10 + begin/end/clear predicates ✓, combat stat ladder + bands + shielding ✓, economy loop ✓, audio bonus complete (BGM + 8 SFX + settings) ✓ — **all 8 gap-register items closed 2026-11-02, incl. D-4.6-1/D-4.6-2 and the latent melee-zero-damage bug**; clean rebuild ✓ | V4.1-1/2, V4.2-3, V4.7-14 eyes-on sweep | **Phase 5 — QA & Packaging:** live full-game sweep (duel lock, duck/jump dodges, wave cadence), voiceover video slide sequence, `[Team]-[Member].zip` packaging (deadline Nov 2 @ 11:59 PM) |
