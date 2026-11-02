# PHASE 7 — Live Demonstration, Defense Execution & Project Closure

**Course:** DMET 502 Computer Graphics (Winter 2026)
**Institution:** German University in Cairo (GUC) — Media Engineering and Technology
**Instructor:** Prof. Dr. Rimon Elias
**Submission Artifact:** frozen post-Phase-5/6 (zip + video links uploaded to Drive) — no source changes permitted on this line
**Phase Window:** post-deadline → end of grading window
**Phase Goal:** Execute the graded interactive surface that no artifact can cover by itself — the **live demonstration and Q&A defense** — capture what actually gets asked/answered, run the **project post-mortem** (what the 5 repair passes taught about the simulated-density vs manual-testing gap), finalize the **repository state** (evidence organization, build-dir hygiene, closure tags), and set the **v2.0 roadmap** if the engine lives past the course.

**Prerequisite:** `phase6.md` — build gates ✅ (working tree + fresh `build-gate`, 0 errors), packaging defect corrected pre-freeze, residual QA items code-verified with live captures owed to the demo session itself.

---

## 1. Executive Overview

Phase 7 is the *closure* phase. It closes three loops:

1. **Evidence loop** — the Phase-6 owed captures (S6/S7/S9/S14/S15) and the Phase-5 manual session happen **inside the demo session** if run before the end of grading: one play-through yields the win screen, the Hard duel, the dodge footage, the shield beat, and the audio pass simultaneously. If the demo window already passed on the grading side, the same run is captured standalone as the team's own record.
2. **Knowledge loop** — the post-mortem consolidates the four real engineering findings of this project into reusable lessons: the latent melee-zero-damage chain (D-4.6-1), the inclusive-AABB dodge boundary (D-4.6-2), the missing enemy range gate (D-5.7-1), and the packaging-contract failure caught only by **building from the extracted archive** (§1 note in `phase6.md`).
3. **Handover loop** — the repository is left in a state any future team member (or v2.0 team) can reconstruct: frozen sources, evidence set indexed, phase docs 1–7 forming the full audit trail, closure build verified once more at the end.

### Deliverable Checkpoints

| Checkpoint | Proof Artifact | Status |
|---|---|---|
| Demo session executed (player + narrator roles) | §2.1 runbook + any session notes/screens | 🔄 scheduled |
| Q&A defense log (asked → answered → source) | §2.2 template, filled during session | 🔄 scheduled |
| Owed QA captures banked (S6/S7/S9/S14/S15) | `qa_evidence/vNN/` folders or demo-session recording | 🔄 combined with §2 |
| Post-mortem completed | §3 tables (findings → root cause → lesson → future guard) | ✅ drafted below |
| Repository closed (dirs pruned, evidence indexed, closure build) | §4 audit list + final `cmake --build` log | 🔄 finalize |
| v2.0 roadmap (optional forward work) | §5 backlog with effort/impact ratings | ✅ drafted |

---

## 2. Task 7.1 — Live Demonstration & Q&A Defense Execution

### 2.1 Demo runbook v2 (supersedes phase6 §7.2 — capture-integrated)

The demo is re-ordered so every owed capture is banked **on the same recording**, replacing the standalone `qa_evidence` sessions:

| Step | Action | Doubles as |
|---|---|---|
| D1 | launch with `NEON_P2_DEBUG=1` (fps in title) | fps evidence for the video's 60 fps claim |
| D2 | menu hover all buttons → START | S1 (menu idle + hover outlines) |
| D3 | cutscene: watcher narrates the 5 subtitle beats; no skip (full 15 s) | S2 + cutscene pacing visual |
| D4 | **wave 1**: deploy melee×4 (one per lane), narrate coverage rationale (D-5.7-4 fix) | S3, economy trace |
| D5 | Easy lane: hold **S** as volley brackets → survive; optionally show standing unit getting hit in a sacrifice lane | **S7 duck dodge (owed)** |
| D6 | Medium lane: tap **W** as volley arrives → clear | **S8 jump dodge (banked live)** |
| D7 | let Duel happen: melee vs any contact enemy — narrate halt + HP-bar grind + red tint | **S4/S6 duel mechanics incl. Hard −2 strikes if Hard present (owed)** |
| D8 | if ≥60 coins: deploy Defense + ranged behind in a tough lane; narrate wall absorption | **S9 shielding (owed)** |
| D9 | force the wave to be cleared: manual coverage of spawn lanes until empty | **S14 win screen (owed)** — if the run degenerates, narrate the alternative win-path and show restart instead |
| D10 | ESC pause → Continue → Restart re-init | S11/S12 |
| D11 | close: menu → Sound Settings → VOL+ → mute → BACK | **S15 audio matrix (owed: ears-on narration)** |
| D12 | optional 30 s: `NEON_P2_BORDER=1` window | S16 regression tool |

**Session logistics:** one player + one narrator; record ≥1080p with a voice track (recording can substitute the graded video if the graded one needs a retake — same slide sequence as `phase5.md` §5.1). Cap the whole session at ~20 minutes raw; the graded deliverable is the 2–5 min video, not this.

### 2.2 Q&A defense log (fill during session)

| # | Asked | Answer core | Source of truth | Asked by |
|---|---|---|---|---|
| 1 | *Why OpenGL 3.3 Core + single batch?* | one `glDrawArrays` per frame; ~1.5–3k quads; CPU vertex tinting instead of per-draw uniforms | `phase3.md` §7.2, `renderer` `beginFrame/endFrame` | — |
| 2 | *Why prototype without textures/atlas?* | composite procedural models exercise transform math directly; grade rubric rewards modeling detail, not texels | `phase3.md` §2 | — |
| 3 | *How does dodge work internally?* | HIGH/LOW Y-band AABBs vs ducked box top / jump offset threshold — inclusive-overlap gotcha documented | `Config.h:46-50`, `Entity.h:133-141`, D-4.6-2 | — |
| 4 | *Why START_COINS=40?* | spec floor "≥3 Melee affordable" kept; 4 lanes ⇒ full-cover deployment | `phase5.md` D-5.7-4 | — |
| 5 | *What failed during development?* (the honest one) | player engagement bug that silently zeroed melee damage; enemy snipers without range gate; packaging headers omission — all caught by build-from-archive/test gates | D-4.6-1, D-5.7-1, `phase6.md` §1 note | — |
| 6 | *Why custom GLLoader?* | small stable GL surface; swap-ready; single API contact point | `phase2.md` §4.1 | — |
| 7 | *How is loss freezing done?* | state routing removes sim update entirely; overlay re-renders frozen frame | `GameEngine.cpp:359-371` | — |
| 8 | *Audio how?* | miniaudio; synth→WAV at first launch into `%TEMP%`; 8 SFX + BGM | `AudioEngine.cpp` | — |

Rows 5–8 are the differentiation questions — rehearse them twice (this is why column 5 exists even before the session).

---

## 3. Task 7.2 — Project Post-Mortem (Consolidated Lessons)

### 3.1 Findings → root causes → structural lessons

| # | Finding (ref) | Root cause | Lesson for v2.0 / future projects |
|---|---|---|---|
| L1 | melee dealt **zero damage** for its entire life (D-4.6-1) | engagement flag was set only on the enemy side; melee damage path lived behind a cooldown reset that ran before collision could observe it | combat dispatch belongs in **one place** invoked after movement (`SpatialCollision::resolve` does all faction engagement + melee cadence today); dual-path damage = dual failure mode |
| L2 | duck didn't dodge HIGH projectiles (D-4.6-2) | inclusive `AABB::overlapsY` + band bottom exactly equal to dodged box top | when designing evasion semantics, **code the boundary asymmetry explicitly** (`yRel=30/halfH=20` bands vs `DUCK_TOP_OFFSET=8`), and unit-test the equality edge |
| L3 | enemy shooters carpeted the whole field (D-5.7-1) | fire gated only on cooldown, never on target presence/range | **every coercive system needs a target-resolution gate**, both factions; symmetric rule surfaces unfair-advantage asymmetries |
| L4 | melee was passively unwinnable vs full-HP shooters (D-5.7-3) | spec-derived stat ladder interacts with ranges; PvP-line "HP5 vs sustained 1 dps at 400 u" loses every closing race | balance retunes must be **policy pairs** (HP ladder + cadence + range) — single-knob tuning created the cover-up bugs L1–L3 were hiding |
| L5 | scripted auto-play couldn't clear wave 1 (coverage gap, D-5.7-4) | 3 buys across 4 lanes ⇒ statistical leak | economy "start floor" must be computed against **deployment coverage**, not minimum affordability |
| L6 | zip omitted two loose headers; GLFW path renamed in staging script | packaging contract drifted from what CMakeLists resolves | **the only valid package gate is: extract zip → configure → build → 0 errors** (`phase6.md` execution proved it in one shot — that test is now permanent in phase7 §4.1) |
| L7 | pixel-probe automation unreliable (DPI-virtualization + focus steal) | tooling environment, not game | eyes-on + banked screenshots is the fallback pipeline; never let scoring depend on flaky capture |

### 3.2 Schedule/process observations

- Phase header deadline drift (Nov 6 vs Nov 2 team-confirmed) was self-noticed; ledger corrected in `phase5.md` — keep a single source of truth for course dates.
- The repair sprint of 2026-11-02 (8 gaps closed in one pass) succeeded because the gap register listed **file:line + evidence-slot** per fix; carry that convention forward.

---

## 4. Task 7.3 — Repository Closure

### 4.1 Final hygiene audit (execute once)

| Item | Action | Status |
|---|---|---|
| FINAL archive-equivalence rerun | re-extract the team-built zip → `cmake -G "MinGW Makefiles" -S . -B build && cmake --build build` → 0 errors (the L6-catching guard, permanent) | ⏳ **blocked pending team zip** (team packages by hand per phase6 §1 contract; library + method banked in `%TEMP%\opencode\zip_verify`) |
| Closure build on working tree | `cmake --build build` + 30 s smoke launch with `NEON_P2_DEBUG=1` (fps in title, 0 GL errors) | ✅ **executed 2026-11-02**: rebuild `[100%] Built target Game`, exit 0; 15 s smoke launch minimized + killed cleanly — 0 GL errors in `stderr`, graceful `Intel` GL_VERSION path exercised, modules listed; **audio pipeline re-proven**: all 9 WAVs (bgm + sAttack/sHit/sCoin/sDeath/sPlace/sWave/sWin/sLose) regenerated in `%TEMP%\GameEngine2DAudio` (S15 synthesis evidence banked) |
| build-dir hygiene | keep `build/` + `build-gate/` locally (faster next open); ensure neither is inside any deliverable | ✅ (zip excludes them) |
| Evidence indexing | append an `qa_evidence/README.md` table mapping each `vNN` → what it proves (phase, scorecard ID, notable defect encountered); copy the demo-session captures into their `vNN` slots | ✅ **README.md authored** (v3–v15 mapped to scorecard rows + defect registers; "Owed" section points captures at the §2.1 demo session); demo-session captures pending D-run |
| Phase docs closure stamp | phase6 §2 statuses (freeze + hash) final entries once the team's own zip hash lands the ledger | ⏳ same blocker as archive-rerun (team zip) |
| Temp churn | `%TEMP%\pkg_neon` staging + `%TEMP%\opencode\zip_verify` optional to delete; `%TEMP%\GameEngine2DAudio` regenerates at every launch — leave | left intact (staging + verify dir = reusable frozen-contract templates) |

### 4.2 v1.x line final state

```
sources:   identical to frozen v1.0 (no code delta in phases 6–7)
docs:      phase1..phase7.md (full audit trail) + PROJECT_OVERVIEW + PDF/pdf_text
evidence:  qa_evidence/v3..vNN + demo session recording
build:     build/ (incremental) + build-gate/ (fresh proof) — re-verified at closure
```

---

## 5. v2.0 Roadmap (Post-Course Backlog — optional, effort × impact)

| # | Feature | Effort | Impact | Notes |
|---|---|---|---|---|
| R1 | Texture/CTP atlas pass (characters + environment) | M | High visual | requires GLLoader texture-credentialed per its swap-in plan (`phase2.md` §4.1) |
| R2 | Split UI draw pass + live `uGrayscale`/`uTint` | S | Modest (removes deviation notes phase3 §7.2/7.3) | cleanest after R1 |
| R3 | stb_truetype font for proportional glyphs | S | Low | defeats the text-heavy-frame vertex budget noted phase3 §8 |
| R4 | True parallax background layers (2 depth bands) | S | High aesthetic | pillars/plates already separated by z-visual layer |
| R5 | Wave authoring vs procedural (data-driven wave tables) | M | Design tooling | replaces `WAVE_COUNTS` hand-table |
| R6 | Save/replay of a run (input log → re-seed decision reset `rand()` seed) | M | Demo/QA value | would let scripted runs finally bank S14 deterministically |
| R7 | Coin drift (phase6 §5.1) re-evaluated under R5 data | S | Situational | keep the declined ruling unless data flips |
| R8 | macOS/Linux CMake presets | S | Portability | CMake is 90% there; only GLFW prelib differs |

---

## 6. Risk Register (closure window)

| Risk | Likelihood | Mitigation |
|---|---|---|
| Demo machine stutters (fps < 60, undermining the video's claim) | Medium | kill OneDrive background upload during session; close browsers; re-run `NEON_P2_DEBUG=1` first |
| Win path unreachable in the demo (bad luck lane comp) | Medium | deploy melee×4 immediately (D-4); if wave 3 collapses, narrate + capture Restart; record an alternative run later |
| Hard enemy never spawns during the demo | Medium | wave-3 trap-inverse: stall earlier waves by clearing; if still absent, narrate stat ladder from `Config.h` |
| Audio muted due to machine volume | Medium | D11 runs last — check master volume before D4 so every SFX is heard |
| Post-mortem tree drifts into "code change" territory | Low | §4.2 declares sources frozen; roadmap items live in v2.0 only, never in this tree |

---

## 7. Progress Ledger

| Done | Active | Next |
|---|---|---|
| Phase 6 executed (build gates + code-verified residuals + backlog rulings + packaging-defect resolution); Phase 7 drafted **and partially executed 2026-11-02**: working-tree closure build `[100%] exit 0` + smoke launch (15 s, killed cleanly, 0 GL errors, graceful `Intel` GL_VERSION path, audio WAVs regenerated — S15 synthesis banked); `qa_evidence/README.md` authored mapping v3–v15 to scorecard rows/defect registers + "Owed" capture targets; post-mortem L1–L7 + v2.0 roadmap drafted (§3, §5) | §2 demo session (D-run capture-integrated) + Q&A log; team-side zip → then §4.1 archive-equivalence rerun + phase6 §2 freeze hash | Course closed; voluntary v2.0 kickoff with R1+R2 (atlas + split pass) as opening milestone |
