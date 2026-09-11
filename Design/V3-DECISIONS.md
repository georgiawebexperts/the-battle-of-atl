# BATTLE FOR THE A — active decisions

2026-09-10 [codex-maclaptop]

V3-SPEC.md is the active design specification. It supersedes conflicting V2-SPEC.md and WORLD-EXPANSION.md requirements. Those older files and Archive/V2-ACCEPTANCE.md remain historical evidence only.

## User override after V3

Elliott: “lets get it fully working on the mac”. Final delivery is a native macOS BattleForTheA.app, with a custom icon and desktop shortcut/alias, launching directly into the title screen without Unreal Editor. The V3 Windows .exe packaging requirement is replaced. Do not spend work setting up a Windows build.

## Major rule changes to implement

- 14th Street stone gate start; Cabbagetown bungalow gate finish.
- Character Movement based upright arcade bike, five gears, cosmetic lean, controllable drift, nitro. Terrain never causes a wipeout.
- Direct high-speed actor impacts/enemy attacks cause a two-second wipeout. Glancing contact nudges. Walls bounce/stop. Bike water entry returns to the nearest path after two seconds.
- First-person dismount combat; pistol from bike; four weapons plus U-lock melee.
- Health/regeneration/health pickups; zero health returns to last checkpoint with ten seconds deducted. Previous fatal-bullet/two-stab/reset-item rules are superseded where conflicting.
- Radar with range-dependent artifact blip and path outlines replaces cardinal-only item guidance.
- Difficulty controls time, crowds and all enemy categories; previous single-attacker cap/rare-only rule must not override V3's explicit Medium/Hard Kroger counts and zombie waves.
- Fully populated park, required foliage/landmarks, complete BeltLine/Cabbagetown route, front end and audio remain full requirements.

## Evidence and next milestone

Existing measured terrain, OSM surfaces/decks, navigation, character assets and walker/jogger groundwork can be reused. Old simulation tests and four-second recovery tests do not prove V3 acceptance. Next gate is V3 milestone 1: hills/curbs/grass/stairs/lake test map, 50 wandering NPCs, terrain-proof arcade bike, two-second recovery, dismount into FPS and pistol. Confirm with actual play before world polish.

Reference photos were not found beside the project or in its References folder (which contains geographic data). Photo folder location is pending user input; continue the independent bike milestone meanwhile.
