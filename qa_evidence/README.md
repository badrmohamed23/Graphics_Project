# qa_evidence — Automation Run Index

Each folder = one instrumented session: a scripted `Game.exe` run (screenshots at fixed timestamps `t*`) plus `actions.log` (the exact automation inputs). Captures are 1280×720 client shots; DPI-aware capture, game window forced `HWND_TOPMOST` (methodological notes in `phase3.md` §8).

Read order authority: the screenshot filenames + the phase docs. Where a run's purpose is not restated below, `actions.log` inside the folder is the ground truth of what was clicked/pressed.

| Run | Phase/Defect it proves | Covers (scorecard / req) | Key frames |
|---|---|---|---|
| `v3` (23 caps) | First full phase-5 sweep | S1–S13: menu hover (`01`), cutscene (`02`), gameplay settle + WAVE 1 (`04_wave1`), melee+ranged placement (`05`,`06` / S4, S5), duck-sequence march/shot-over (`07`–`10` / S7), duel lock + grind + kill (`12`–`15` / S4, D-4.6-1), coin after kill (`16`), wave-2 zone (`17`), pause/resume (`19`,`20` / S11–S12), post-run (`21`,`22`) | `15_duel_kill.png`, `09_duck_shotover.png` |
| `v5` (6 caps) | Phase-5 economy + pause probes | S10/S11/S12 supplemental: coin pickups over time (`s3_01..03`), pause + resumed (`s3_04/05`) — cited for unlock/relock churn (`s3_02_coin_t14.png`) | `s3_04_pause.png` |
| `v6` (8 caps) | Ranged turret session (pre-range-gate retune) | S5 supplemental + economy (`s5_01..07`) — cited in phase5 S10 (gray Defense card <30) | `s5_01_t6.png` |
| `v7` (6 caps) | Border tool + jump help frames | S16 border (menu + game), S8 jump sequence (`s6_03..05`) | `s6_01_menu_border.png` |
| `v8` (7 caps) | Border + full jump-dodge frame pair | S16 + S8 (phase5 cited `s7_03_jump_mid.png` mid-air) | `s7_02b_prejump.png` → `s7_03_jump_mid.png` → `s7_05_landed.png` |
| `v10` (8 caps) | **D-5.7-1 fix proof** (enemy `gunTarget` range gate) | field silent until range contact (`s9_01_t6`), turret out-ranges and opens first (`s9_03_t16`), first kill + collectible drop (`s9_05_t26`) | `s9_05_t26.png` |
| `v11` (8 caps) | **D-5.7-3 retune** evidence melee 2-v-1 | lone melee grinds a 2-v-1 pair before falling (`s10_04_t21`) | `s10_04_t21.png` |
| `v12` (8 caps) | **D-5.7-3 live proof**: duel lock + damage exchange after HP/cooldown retune | halted duel, red tint, grounded HP bars | `s11_04_t21.png` |
| `v13` (8 caps) | Combat-session automation pair (s12 series) — general combat traces; not tied to a specific defect register row | see `actions.log` | — |
| `v15` (12 caps) | **D-5.7-4 economics**: 40-coin open, full 4-lane coverage; waves 1+2 cleared; wallet growth 60→65→150 (coin-drift decline evidence, phase6 §5.1) | `s14_00_menu_coins40`, `s14_01_covered`, WAVE 2/3 (`s14_05_t31`), WAVE 3/3 (`s14_08_t52`), COINS 150 (`s14_10_t66`) | `s14_10_t66.png` |

## Owed (not yet captured — phase7 §2.1 demo session targets)

| Slot | Item | Capture plan |
|---|---|---|
| S6 win | "ALL WAVES CLEARED – YOU WIN!" overlay | human-played run (automation always leaked lane coverage; v15 reached wave 3 but lost) |
| S9 live | Defense wall absorption + friend vulnerability after wall death | needs Defense deployment (wallet ≥30 + friend) — demo D8 |
| S7 live-survival | ducked player surviving full Easy volley (math-verified: band [10,50] vs ducked top yc+8) | demo D5 |
| S15 ears-on | BGM loop + ≥4 distinct SFX audible + VOL±/M | demo D11; synthesis pipeline already proven (WAVs regenerated in `%TEMP%\GameEngine2DAudio` on the 2026-11-02 closure smoke) |

## Related artifacts outside this folder

- `%TEMP%\opencode\phase3_shots_v4..v11` (original phase-3 instrumentation; superseded by folders above)
- `%TEMP%\opencode\zip_verify` — extracted-archive build proof for the corrected packaging contract (see `phase6.md` §1 note)
