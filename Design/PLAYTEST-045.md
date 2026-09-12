# The Battle of ATL — Playtest 045

2026-09-12 [codex-maclaptop]

Work-in-progress build, installed on the desktop. Previous build 044 retained as a rollback copy.

## Changes

- First ambient park sleeper: occasional proximity wake, brief pursuit, return to sleep and cooldown. Normal wake chance is 18% per eligible approach.
- Imported/retargeted sleeping, waking and stumbling motions; preserved the landed pose when switching back to sleep.
- Fixed returning sleepers stopping short of their resting spot. The final approach uses character collision and restores the original resting direction.
- Improved 47 concrete surfaces with aggregate variation and world-space slab joints.
- Corrected City walker ground alignment using the movement component's measured floor distance.

## Validation

Native male and female walking checks passed, with captured idle foot gaps under 1 cm and correct foot direction. Main-map automatic sleeper wake/chase/return/cooldown test passed. Packaged automatic sleeper loop passed (418.937→77.414cm approach), including return-to-sleep and cooldown. Packaged and installed-app health, cardinal phone hunt, both death resets, recollection and ordered-checkpoint checks passed. BuildCookRun completed in136.60seconds; strict ad-hoc codesign verification passed. Desktop link resolves to version0.45.0.

These are targeted checks, not full-game or visual-quality acceptance.

## Still unfinished

Rough sleeper outfits, bench sleepers/fires, continuous animation polish, street cars and intersection scenes, skatepark detail, combat presentation, environmental polish and final celebration remain incomplete. Existing major game requirements remain active.
