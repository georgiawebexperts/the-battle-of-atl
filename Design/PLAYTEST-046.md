# The Battle of ATL — Playtest 046

2026-09-12 [codex-maclaptop]

Installed Mac development build 0.46.0. Build045 remains available as a rollback copy on the external drive.

## Changes

- Rare ambient bench ignition: 20% eligibility per run after45 seconds of eligible play, at most one attempt. The character spawns behind the camera and begins the interaction when approached. Tutorial, countdown and inactive-player states are protected.
- Authored reach, handheld prop, delayed fire/smoke/light, interruption handling, and a short walk away. Bench ownership prevents overlapping encounters. Current fire is a visual ambient scene; heat damage and scorch marks are not implemented.
- A bench and level seating pad beside The Fancy Roach Motel, facing the park and separated from nearby paths.
- Dark aggregate asphalt material on30 mapped surfaces. Build045's concrete and sleeper improvements remain included.

## Validation

BuildCookRun passed in140.72 seconds. Packaged rare-encounter tests passed forced0/1 probability, quiet-period protection, behind-camera spawn, approach ignition and the one-attempt limit. Packaged Motel scene captured and burning frame inspected; character retreated241.397cm, with prop tip1.159cm above the slat at ignition. Packaged sleeper wake/chase/return/cooldown passed (355.795→77.193cm approach). Health, cardinal phone hunt, both death resets, recollection and ordered checkpoints passed in both the package and installed app. Strict ad-hoc codesign and desktop version/link checks passed.

These are targeted checks. Default rarity statistics, a full user ride, all animation transitions and frame-rate acceptance remain open.

## Still unfinished

Rough sleeper/igniter outfits, bench sleepers, cars and intersection scenes, race dressing, continuous animation and combat polish, richer environment assets and the final celebration still need work. This build does not complete the full game goal.
