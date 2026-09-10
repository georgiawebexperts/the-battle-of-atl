# Piedmont Ride V2 — Web Experts

Author: Web Experts — www.webexperts.com
Build: 0.2.0-dev (milestone 1 in progress)

The complete design specification is V2-SPEC.md. No milestone is accepted on compilation alone.

1. IN PROGRESS: Bike, rigged rider, seven gears, controls, ground physics, grass, ramp, water, crash and respawn tests.
2. Pending: real elevation, connected OSM park paths, lake and boundaries.
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
