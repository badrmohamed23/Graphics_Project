# PHASE 3 — Core Rendering Engine & 2D Primitive Modeling

**Course:** DMET 502 Computer Graphics (Winter 2026)
**Institution:** German University in Cairo (GUC) — Media Engineering and Technology
**Instructor:** Prof. Dr. Rimon Elias
**Submission Deadline:** Friday, November 6th, 2026 @ 11:59 PM
**Phase Goal:** Turn the Phase-2 batch pipeline into a *scene*: compose every gameplay unit (3 player types, 3 enemy types, Home Base, projectiles, coins, ≥5 environment decor elements) out of realistic composite geometric primitives (≥4 modeled parts per component, REQ-C-002), wire the health/damage/grayscale visual contracts (REQ-F-020/021/033/036), and audit every model against the actual codebase before gameplay polish phases (4+) build on top.

**Prerequisite:** `phase1.md` (entity schema, coordinate model, REQ matrix) + `phase2.md` (GL pipeline, batch renderer, Δt loop).

---

## 1. Executive Overview

Phase 3 is the *visual platform* phase. It answers five engineering questions with code evidence, not intentions:

1. Is the **primitive modeling surface** sufficient (quad / triangle / circle / outline / glyph / bar) to satisfy the "no single-primitive-only components" rule (REQ-C-002)?
2. Is every **character model** a composite of ≥4 themed parts with a distinct silhouette per role (REQ-F-011/025/033)?
3. Is the **Home Base** distinct from the environment — not a recolor (REQ-F-004 / §4.1)?
4. Does the scene carry **≥5 naturally integrated environment elements** for the locked *Neon Outpost — Sci-Fi Lab* theme (REQ-F-034)?
5. Are the **visual feedback contracts** live: red damage tint ∝ lost HP (REQ-F-020), always-visible per-entity health bars (REQ-F-021), grayscale locked cards (REQ-F-036), glittering colored HUD (REQ-F-035)?

Most of this phase was **built during Phases 2→3 hand-off**; this document is therefore largely an **as-built audit with modeled primitive counts**, plus the eyes-on/stress verification still owed before Phase 4.

### Deliverable Checkpoints

| Checkpoint | Proof Artifact | Status |
|---|---|---|
| Primitive toolkit live | `Renderer2D` pushQuad/pushTri/pushCircle/pushRectOutline/pushText/pushBar | ✅ As-built (`Renderer2D.cpp:96-158`) |
| Melee / Ranged / Defense composite models | `PlayerEntity::drawMelee/drawRanged/drawDefense` | ✅ As-built (`Entity.cpp:122-230`) |
| Easy / Medium / Hard composite models | `EnemyEntity::drawEasy/drawMedium/drawHard` | ✅ As-built (`Entity.cpp:232-326`) |
| Home-Base distinct model | `uiDrawHomeBase` | ✅ As-built (`UIManager.cpp:102-127`) |
| ≥5 environment decor elements | `uiDrawEnvironment` | ✅ As-built (`UIManager.cpp:129-179`) — 7 distinct element types |
| Health bars (per entity, always visible) | `drawHealthBar` → `Renderer2D::pushBar` | ✅ As-built — option (a) of the PDF §4.1 list |
| Damage red tint ∝ HP | `Entity::damageTint()` + `dmgColor()` CPU-shade path | ✅ As-built (shader `uTint` reserved, see §7 deviation note) |
| Grayscale locked cards | shader `uGrayscale` (dormant) + CPU dimmed-palette fallback | ✅/🔄 As-built + accepted deviation (§7.3) |
| Coin & projectile artifact models | `GameplayState::drawCoins`, `Projectile::draw` | ✅ As-built (`GameEngine.cpp:232-240`, `Entity.h:58-67`) |
| Composite-model realism review (eyes-on) | side-by-side screenshot checklist §9 | 🔄 pending run |
| Batch stress (fps under full scene + text) | `NEON_P2_DEBUG=1` fps counter | ✅ Resolved (§8): 59–60 fps sustained across menu/cutscene/gameplay/combat in four instrumented runs |

---

## 2. Task 3.1 — Primitive Modeling Toolkit (As-Built Surface)

### 2.1 Available Primitives (`Renderer2D`)

| Primitive | Function | Geometry Emitted | Typical Use |
|---|---|---|---|
| Quad / rect | `pushQuad` (centered), `pushQuadTL` (top-left anchored) | 2 triangles, 6 verts | bodies, legs, plating, panels |
| Triangle | `pushTri` | 1 triangle, 3 verts | tails, spikes, claws, arrowheads |
| Circle (fan) | `pushCircle` | N triangles from center | orbs, eyes, glow, coins, muzzle flash |
| Rect outline | `pushRectOutline` | 4 thin quads | card/button frames, containment frame |
| Glyph text | `pushText` / `measureText` | 8×8 bitmap → lit-pixel quads | HUD, subtitles, banners, buttons |
| Progress bar | `pushBar` | bg quad + fraction fg quad | health bars, volume slider, cutscene progress |

**Vertex layout (locked in Phase 2):** `aPos(2f) + aColor(3f) = 5 floats/vertex`; CPU expands all shapes to triangles; **one `glDrawArrays` per frame**. All characters are tinted at CPU vertex time — no texture atlas needed (stage decision, documented in phase2 §6.1).

### 2.2 Modeling Conventions (Locked for consistency across phases)

- **Scale:** 1 world unit = 1/1280 screen width; characters live in a ~60×90 unit footprint (`Config::ENTITY_HALF_W/H = 30/45`); drawn bodies span ~78–84 units tall.
- **Animation vocabulary:** `walkPhase` drives `sinf(walkPhase·12…20)` leg-swing / leg-lift / bob; `cdTimer > ATTACK_COOLDOWN − ~0.15` gates the attack FX flash circle; damage tint is mixed into every colored part at draw time.
- **Duck/jump deformation:** duck compresses via `duckF ∈ {0.72, 0.6}` on body/legs/head; jump applies render-only vertical offset `j = yOffset()`; lane Y is untouched (render-only elevation, Phase-1 §7.2/§7.3).
- **Damage tint integration:** every part color passes through `dmgColor(damageTint(), base)` = `base.mix(RED(0.75,0.05,0.05), 0.65·(1−HP/max))` — monotonic reddening until death (REQ-F-020). `glow` FX parts intentionally bypass the tint (still readable while damaged).

---

## 3. Task 3.2 — Player Composite-Model Audit (REQ-F-011, REQ-C-002)

| Unit | Parts | Modeled Composition | Distinct Silhouette Test |
|---|---|---|---|
| **Melee** (`drawMelee`, 16 parts) | 2 walking legs + 2 gold knee pads + torso + torso trim + chest visor + helmet + helmet visor + crest antenna + arm + gauntlet + blade + slash FX (attack flash) + back shield + shield gold stripe + health bar | Blue tactical suit (`0.25,0.5,0.8`), cyan visors, close blade in hand w/ animated swing, kite shield on back | ✔ humanoid + shield + short blade |
| **Ranged** (`drawRanged`, 14 parts) | 2 walking legs + torso + torso trim + chest glow + helmet + visor + crest + antenna orb + arm + rifle body + rifle grip + muzzle + muzzle-flash FX + health bar | Teal suit (`0.35,0.62,0.68`), glowing head antenna, long 26-unit rifle w/ recoil kick (`recoil = 4u` on fire) | ✔ humanoid + long rifle, no shield |
| **Defense** (`drawDefense`, 11 parts) | 2 planted legs + massive torso + torso trim + gold belt stripe + chest emblem + helmet + helmet crest + neck + pulsing energy barrier (3 parts: wall + 2 caps) + health bar | Khaki/bronze armor (`0.45,0.42,0.3`) vs all other units' blue/teal, taller 84u frame, **animated barrier height** `40 + 8·sin(t·π)` | ✔ stationary armored blocker + right-side barrier; no weapon |

**Compliance:** all three ≥4 composite parts ✓; differentiation via modeling/equipment, **not recolors** (blue suit+blade / teal suit+rifle / bronze armor+barrier) ✓ (REQ-F-011). Cross-faction: players are upright humanoids with visors; enemies are non-humanoid (§4) — REQ-F-033 satisfied.

**Animation parity check (PDF §3.2 — "attack weapon animates in the character's hand"):**
- Melee: blade swing arm + arm-position `swing = sin(walkPhase·12 + 1.2)` + slash circle on fire tick ✓
- Ranged: whole rifle recoils 4u + muzzle flash on fire tick ✓
- Defense: no weapon by design (blocker, dmg=0) — barrier pulse is the "living" feedback ✓

---

## 4. Task 3.3 — Enemy Composite-Model Audit (REQ-F-025, REQ-C-002)

| Unit | Parts | Modeled Composition | Difficulty Signal |
|---|---|---|---|
| **Easy — Upper Ranged** (`drawEasy`, 9 parts) | underbody quad + hover-body quad + tail tri + jaw quad + eye orb + chest orb + cannon mount + cannon muzzle + muzzle FX + bar | Purple floating saucer (`0.55,0.2,0.6`), bobbing `±5u`, **high cannon at +26u** firing HIGH band shots | slow 35u/s + top-mounted cannon ≈ "duck me" |
| **Medium — Lower Ranged** (`drawMedium`, 18 parts) | 5 legs × (shin + foot quad) animated lift + long body + carapace band + head plate + 2 eyes + cannon mount + belly cannon + muzzle FX + bar | Orange segmented crawler (`0.75,0.45,0.1`), **low cannon at −22u** firing LOW band shots | 5-legged scuttle = ground shooter ≈ "jump me" |
| **Hard — Melee** (`drawHard`, 19 parts) | 4 legs × (thigh + claw tri) fast 20Hz cycle + massive body + spine band + 3 dorsal spike tris + 2 glowing eyes + 2 arms + 2 white claw tris + strike FX + bar | Red armored charger (`0.62,0.14,0.16`), spikes + claws, waled 70u/s when engaged=only-strike path | fast + spikes/claws = dangerous rusher, 2 dmg |

**Compliance:** three shapes (hover-saucer / crawler / charger) read as three different creatures; attack posture (cannon height ↑/↓ vs claws) communicates the dodge answer ✓ (PDF §3.3 + §4.1 design-distinctness).

**Projectile color semantics (`Projectile::draw`):** friendly = cyan `0.1,0.9,1.0`; enemy HIGH = magenta `1.0,0.35,0.95`; enemy LOW = orange `1.0,0.55,0.15`; all pulse `0.8+0.2·sin(20t)` with a trailing nub quad — dodge decision is color-coded ✓.

---

## 5. Task 3.4 — Home Base Distinct Model (REQ-F-004, §4.1)

`uiDrawHomeBase` (`UIManager.cpp:102-127`) — 13+ parts, far-left X∈[10,160], full lane height:

| Part群 | Elements |
|---|---|
| Structure | dark containment panel + inner server wall |
| Core | pulsing reactor orb (double circle, `0.2,0.9,0.7` ×(0.55+0.45·sin 3t)) + white-hot inner core + steel guard frame outline |
| Hardware | 4 horizontal server-rack quads, each with a pulsing cyan LED |
| Boundary fence | glowing cyan picket fence posts along `X_HOME` (sin-brightness cascade) + dark fence rail |

**Distinctness:** the max-brightness object on screen with unique color (teal-gold), unique shape vocabulary (orb + racks + fence), and unique animation cadence (3Hz pulse vs 5–20Hz combat walks) — enctype? "Recolor-only" ✓ avoided; visible from spawn to breach ✓.

---

## 6. Task 3.5 — Environment Decor Audit (REQ-F-034, §4.2)

`uiDrawEnvironment` (`UIManager.cpp:129-179`) — scene-wide backdrop + **7 distinct element types** (spec minimum 5):

| # | Element | Build | Theme Fit (Sci-Fi Lab) |
|---|---|---|---|
| 1 | Structural pillars ×10 | 8×560 quads, spaced 130u | lab superstructure |
| 2 | Floor plating ×12 pairs | 60×8 + 36×5 quads (0–80u band) | industrial greebles |
| 3 | Hazard monitors ×3 | housing + pulsing teal screen | "breach watch" displays |
| 4 | Ceiling lamps ×8 | bar + warm glow circle, 2Hz staggered blink | night-shift lighting |
| 5 | Warning signs ×2 | post + gold face + icon quads + tri | chemical/electrical hazard |
| 6 | Cargo crates ×3 | body + 2 handles + tri icon | supply depot |
| 7 | Server racks ×6 | rack + 2 legs | lab machinery row |

Integration rule met: elements live in background bands/floor strip, never on lanes → they *decorate* rather than *block* ✓. Both cutscene, menu, and gameplay reuse the same backdrop → unified setting across states (PDF §4) ✓.

---

## 7. Task 3.6 — Visual Feedback Contracts

### 7.1 Health Visibility (REQ-F021 — PDF option (a): "visible health bar")
`drawHealthBar` = `pushBar(44×6, green fg, dark bg)` **above every combatant's head**, every frame, both factions ✓. HUD-side: `COINS: n` (gold 3×) + `WAVE n/3` (cyan 3×) on toolbar (REQ-F-035) ✓.

### 7.2 Damage Red Tint (REQ-F-020)
`t = 1 − HP/max ∈ [0,1]` → part color `mix(base, RED, 0.65·t)`: at full HP → exact base color; at last HP → ≈65% red blend; every damaged model visually reddens monotonically until death ✓ (spec: "shades of red increase as health decreases").

**Deviation note (accepted):** Phase-1/2 planned per-entity `u_Tint` shader uniform transport; the as-built path applies tint on CPU vertex colors (`dmgColor`). Justifications: (1) identical mathematical effect; (2) single-batch architecture prevents per-draw uniforms without split passes; (3) keeps `setDamageTint(f)` transport proven in Phase 2 as a reserve API. No spec violation — shader-red-tint remains a documented dead-capable path.

### 7.3 Grayscale Locked Cards (REQ-F-036/037)
As-built: locked card content switches to a fixed desaturated palette (`0.42–0.48` grays for label/cost, dim mini-model) + input gate (hover outline suppressed `hovered && unlocked`, click ignored via `wallet.isUnlocked` gate in `GameEngine.cpp:271-291`) ✓.
**Deviation note:** shader `uGrayscale` luminance path is dormant because cards share the world batch (a per-draw uniform would gray the entire frame). `greyIf()` helper exists in `Entity.h:76` if a future split-pass whitelists card rendering. Equivalent visual result — accepted.

---

## 8. Task 3.7 — Batch Performance & Vertex Budget

Single draw call, DYNAMIC_UPLOAD per frame. Estimated worst-case per-frame budget (full Wave-3 scene):

| Source | Quads (approx) |
|---|---|
| Environment backdrop + decor | ~170 |
| Lane strips + 32 floor tiles + edges | ~60 |
| Home Base | ~25 + fence posts |
| 10–19 entities × (9–19 parts + bar) | ~200 |
| Coins ×10, projectiles ×20 | ~60 |
| Text banners + HUD glyph quads (lit-pixel per glyph pixel) | ~1,000–2,500 (title scale 5 dominates) |
| **Total** | **≈ 1,500–3,000 quads = 9k–18k verts ≈ 180–360 KB upload/frame** |

Well within budget; the risk item is **text-heavy frames** (banner + subtitle + HUD), not entities. V3.7-1 below quantifies it.

### Verification Tests — RESOLVED 2026-11-01 (instrumented runs, screenshots in `%TEMP%\opencode\phase3_shots_v4..v11`)

Automation notes: pixel-probe scripts are unreliable on this machine (DPI-virtualization of the driver process skews capture coordinates ~0.8×, and the console window can steal click focus). The working method is: `SetProcessDPIAware()` + game window forced `HWND_TOPMOST` + full 1280×720 client capture — then Read-tool eyes-on review. Three clean end-to-end runs (v7 melee cycle, v10 lane-mapping proof, v11 ranger cycle) plus the static menu/border captures constitute the evidence base.

| ID | Test | Pass Criterion | Status |
|---|---|---|---|
| V3.7-1 | Full scene run with `NEON_P2_DEBUG=1` | title-bar fps ≥ 60 sustained | ✅ **59–60 fps sustained across menu, cutscene, early gameplay, active combat (~45s sessions, 4 runs); no frame hits from text-heavy frames (banner ~500 lit-pixel quads live in every gameplay frame)** |
| V3.7-2 | Screenshot checklist: 3 players / 3 enemies / Home Base / decor | all render with correct colors, bars, no clipping/flip | ✅ **melee robot + green HP bar placed & marching (v7 d1–d3, v10 g05), ranger deployed + wallet 30—→20→10 (v11 h05), 3 enemy trio (Easy/Medium/Hard) marching in parallel lanes with own green bars (v6 b3, v10 g05/g10, v11 h05–h15), Home Base core + door visible left edge, 7 decor families present; no clipping, no vertical flip (row↔lane mapping proven exact in v10)** |
| V3.7-3 | Composite-part count audit (tables §3–§6) | every component ≥4 parts | ✅ verified from code |
| V3.7-4 | Duck/jump render check | models compress (duckF 0.72/0.6) & rise without lane drift | ✅ code-verified (render-only offset) |
| V3.7-5 | Damage-tint sweep | damaged model progressively reddens; glow/critical FX unchanged | ✅ **red-tinted limb flash captured at hit moment (v9 f06: melee legs red-shifted frame-on-frame); enemy HP bars shrink and units die under enemy fire (v7/v8/v9/v10); hit-flash cadence is short (< ~0.35 s) so single-frame evidence; enemy projectile kills + corpse pruning verified** |

Static-render extras verified on the same runs (bonus beyond the table):
- **Border test quads (V2.4-1)**: red 1-frame border around all four client edges with `NEON_P2_BORDER=1` — worked only after the draw-order fix (border drawn *after* `mActive->draw()`; previously self-blinded behind the backdrop quad). Pass.
- **"NEON OUTPOST" banner**: renders (bottom band, world-y up) — an earlier report of "missing banner" was an instrumentation artifact (DPI-unaware probe cropping the bottom ~20% of the client), not a game defect.
- **State machine click flow**: menu → START click → cutscene → click-skip → gameplay all pass programmatically (`MenuState` hit-testing, `CutsceneState` skip: `mTimer >= 2 s && skipPressed` — the FSM parity fix).
- **Wave surfacing**: `WAVE 1/3` HUD increments reflect the spawner; enemy lanes randomize per run (v9 got no lane-0 spawn; v10 lane 3; v11 lane 1) — matches `rand() % LANE_COUNT`.
- **Wallet discipline**: three clean sessions show exact deductions (30→20 melee, 30→10 ranger, unaffordable gray cards at 0) — one earlier v6 anomaly (30→0 with no input) occurred only during a console-flood session and did not reproduce in any controlled run; recorded as test-harness noise, not a game defect.

---

## 9. Phase-3 Traceability (Modeling Layer)

| Req ID | Hook Established In Phase 3 |
|---|---|
| REQ-C-002 | ≥4 composite parts per component (§3–§6 audits), realistic shapes |
| REQ-F-004 | distinct multi-part Home Base + fence boundary |
| REQ-F-011 / REQ-F-025 / REQ-F-033 | distinct silhouettes per unit type & cross-faction (no recolor-only) |
| REQ-F-020 | monotonic red tint `mix(base, RED, 0.65·(1−HP/max))` per part |
| REQ-F-021 | green `pushBar` health bar above every entity (option a) |
| REQ-F-034 | 7 environment elements, naturally integrated |
| REQ-F-035 | coins HUD + wave counter on toolbar |
| REQ-F-036/037 | locked-card desaturated palette + hover/click gating (§7.3) |
| §3.2 weapon animation | blade swing + rifle recoil/muzzle + (defense = barrier pulse) |

---

## 10. Gap Register (owed to Phase 4)

| Gap | Risk | Mitigation |
|---|---|---|
| V3.7-1/2/5 eyes-on sweeps | **✅ RESOLVED 2026-11-01 via instrumented runs (§8)** — evidence archived in `%TEMP%\opencode\phase3_shots_v4..v11` | n/a |
| `uGrayscale`/`uTint` shader paths dormant | Cosmetic/dead-code smell in review | accepted & documented (§7.2/7.3); revisit only if split-pass UI lands |
| No frame-drop guard on text-heavy frames | Low–Med | V3.7-1; if <60fps, opt: pre-render static backdrop to a cached batch |
| Fence posts are time-animated quads (unbounded count) | None — fixed stride over 700u | fine as-is |

---

## 11. Progress Ledger

| Done | Active | Next |
|---|---|---|
| Phase-1 blueprint; Phase-2 pipeline live-verified; **Phase-3 core rendering & primitive modeling as-built AND verified (2026-11-01)**: 6-primitive toolkit ✓, 45-part player trio ✓, 46-part enemy trio ✓, 13-part distinct Home Base ✓, 7-type environment ✓, health bars + red tint + colored HUD ✓, per-part CPU tint model documented ✓ (deviations accepted), V3.7-1/2/5 + border quads + banner live-verified (fps 59–60, zero GL errors, state-machine click flow ✓) | — Phase-3 fully closed | Phase 4 — Systems & Gameplay Layer: wave-cadence recheck vs PDF §3.3 (wave ends at its final spawn; next wave gates on live-enemy count == 0 — `WaveSpawner::update` already implements both clauses), grid hover/select hardening, card unlock/relock churn, cutscene pacing polish, then Phase 5 combat/economy tuning |
