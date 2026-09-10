# Piedmont Ride V2 — Web Experts

Author: Web Experts — www.webexperts.com
Build: 0.2.1-dev (bike validation extended; world in progress)

The complete design specification is V2-SPEC.md. No milestone is accepted on compilation alone.

1. IN PROGRESS: Bike, rigged rider, seven gears, controls, ground physics, grass, ramp, water, crash and respawn tests.
2. IN PROGRESS: measured USGS Landscape saved; OSM path surfaces and connectivity under construction; real lake and boundaries pending.
3. Pending: all named park landmarks, skyline, vegetation and reference matching.
4. Pending: full Eastside Trail to Krog Street Tunnel.
5. Pending: complete timed pickup/checkpoint/finish loop and saved results.
6. Pending: all NPC behaviors, crowd coverage and difficulty.
7. Pending: audio, lighting, UI and measured performance.

Reference photos mentioned in the prompt were not included in the received attachment.
V1 remains in Content/BeltLineGlide. V2 lives in Content/PiedmontRide with separate classes.

## 2026-09-10 — Basic bike integration checkpoint

All 15 live Unreal integration cases pass in Tests/Results/2026-09-10-bike-basic-0.2.0.json. Tests drive PlayerController key bindings and inspect actual travel/collision state. Coverage: Up/Down gears; Tab/Shift cameras; low/high gear acceleration; pedaling foot motion; 30 mph top gear with >100m actual travel; coasting; braking; grass cap; uphill ramp; solid obstacle; shoreline; excessive lean; four-second recovery.

Clothed Quaternius Casual rider (CC0), procedural two-bone limb posing, custom tire/spoke geometry and tubular commuter frame installed. V1 remains preserved.

Milestone 1 still needs broader hands-on feel/edge-case checks (progressive brake pressure, downhill behavior, grass braking, airborne lake entry, full rider crash presentation). This checkpoint does NOT accept the full V2 game. No geography or later milestone is accepted.

Unreal's CrashReportClientEditor crashed during the process-level restart. The editor recovered, Aura remained running, and the next complete bike test passed. Use the supported QUIT_EDITOR console command for future normal reloads; avoid process termination for routine reloads.

## 2026-09-10 — Extended bike checks and measured terrain

All 20 live bike integration cases passed in Tests/Results/2026-09-10-bike-extended-0.2.1.json. The five added cases cover progressive brake pressure, longer grass stopping distance, downhill coasting, airborne water entry, and a gentle turn without crashing. Hands-on riding feel and presentation remain subject to review.

PiedmontWorld is saved with a 1513 x 3025 measured USGS heightmap, 288 Landscape components, real distance/elevation scale 1:3, and no missing DEM samples. Import inspection confirms component count and transform. The collision-height test is NOT yet passed: the first attempt hit a Python API error; its repaired version awaits execution.

The source network contains 236 clipped OSM path pieces. The initial vertex graph has 20 components; SourceAssets/Terrain/park-connectivity-audit.json locates all 19 separated components and flags bridge crossings. No fabricated shortcut edges were added. Terrain-following pavement source generation is in progress, with bridge decks deferred. This does not establish that all paths are connected or rideable.

The Mac is currently locked, preventing editor UI inspection following Elliott's crash report. Process/file checks confirm Unreal, Aura and Xcode running and both V2 maps present. Resume desktop validation after manual unlock; do not launch duplicate editors.

Pavement source checkpoint: 56 OBJ chunks, 148,697 triangles across concrete/asphalt/gravel, with 14 bridge ways explicitly deferred. Independent exported-geometry checks pass, including coverage and 3 cm Landscape clearance (worst error below 0.000002 cm). Report: Tests/Results/2026-09-10-park-pavement-source.json. Unreal import, axis orientation, collision, connectivity and riding acceptance are still pending.

## 2026-09-10 — Installed park paths and recovered editor

Unreal was responsive after manual unlock. Two normal editor exits and relaunches completed without process termination. The initial native surface-query wrapper and APiedmontPathSpline class compiled successfully.

PiedmontWorld now contains all 236 OSM centerline spline actors and 56 placed pavement meshes. Corrected the OBJ Y-axis conversion. Set explicit full-detail Nanite fallback geometry and finer position precision after the first collision audit found simplification errors. All 727 installed pavement collision samples now pass, archived in Tests/Results/2026-09-10-park-pavement-installed.json. Source OBJ geometry checks also pass. Neither test accepts the bridges or full network ride-through.

Terrain interior height samples pass. Rays at exact grid vertices can miss; a 2 cm sphere fallback for the actual bike ground probe is authored but NOT YET COMPILED OR TESTED. The expanded terrain validator now requires this pending native rebuild. Do not present it as passed. Existing 20-case bike results predate these latest movement edits and must be rerun.

Lake Clara Meer's OSM outer ring and island are prepared, with 9,735 interior DEM samples agreeing on a water elevation of 267.359985 m. Authored underwater depth (maximum 4.5 real metres) is explicitly separate from measured 3DEP surface data. Source lakebed R16 and shoreline collision OBJ are ready, but no actual lake actor/bed/shoreline has been installed in PiedmontWorld yet. Island exclusion and nearest unobstructed path recovery code are authored, also awaiting compilation and regression.

The Mac locked again before the next normal editor restart. Resume by closing the editor normally, compiling, enabling session-only world jobs if needed, and running the expanded terrain checks and all 20 bike regressions on BikePhysicsLab. Then install the lake, verify shoreline/airborne/island/bridge behavior and resume connectivity work. Full game scope remains pending.
