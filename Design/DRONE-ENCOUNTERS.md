# Drone encounters — build 036

2026-09-11 [codex-maclaptop]

One drone at most; first attempt after 35 seconds of eligible riding, subsequent attempts 60–100 seconds apart. Spawn requires a clear approach outside obstructions; no spawn while on foot, dead, stunned or recovering. Controlled subsystem audits freeze ambient spawns.

Two-second HUD warning precedes a swoop toward the rider's position at commitment. The drone does not track during that dive, allowing evasion. A swept sphere respects walls/terrain and detects bike contact. A hit deals 15 health damage, attempts safe dismount, and locks recovery for two seconds before local remount. Existing on-foot countdown naturally costs recovery time; no extra flat timer penalty. The drone retreats after one impact or pass. Safe-dismount retries handle briefly occupied exit space.

Mechanical model has a body, four arms/motors, spinning propellers and a warning lamp. 40 health permits shooting it down; no zombie time reward. Dedicated rotor audio, destruction effects and authored rider impact/get-up animation remain unfinished. Model and warning require native visual inspection; this is not final art or balance acceptance.

Native audit covers warning timing, actual swept impact, damage/knockoff/action lock/recovery/remount, a physical blocking wall and damage/destruction. Fixed test placement does not prove all world spawn locations or dodge balance.


## Physical recovery update — 2026-09-12 [codex-maclaptop]

Post049 source now uses the shared physical rider/bike fall on a successful mounted drone hit. The two-second stun is a minimum lock; physical settling and the get-up clip take additional time before on-foot control.15 damage, warning, committed dive, collision sweep, repeat protection and spawn rarity are preserved. Native rendered hit/recovery/remount and broader headless wall/destruction checks pass. Not yet packaged; visual motion/audio and broad balance acceptance remain unfinished.
