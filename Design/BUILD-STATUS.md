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

## 2026-09-10 — Rendered lake and safe shoreline recovery

Ground-probe sphere fallback is compiled. All 50 terrain contact samples pass, including exact grid corners; the initial 20 bike regressions also pass (archived under Tests/Results with terrain-ground-contact and bike-ground-probe-regression names).

Installed Lake Clara Meer using the OSM outer ring and island, measured water elevation, authored lakebed and island bank, and hidden solid shoreline mesh. The DEM does not measure underwater depth or the island bank profile; those remain authored estimates. Positive ring winding and a native OnWaterBodyChanged shape refresh fix empty water geometry. The water surface and island are visually confirmed in the editor.

All five real-park integration cases pass in Tests/Results/2026-09-10-park-lake-recovery.json: path travel, nearby-path recovery, shoreline water crash, airborne water entry, and dry island footing. Recovery checks a 42 cm footprint against water and capsule clearance, then holds the bike stationary until pedaling so gravity cannot roll it back into the lake. Test duration now follows the movement component's capped simulation time; these tests do not establish real-time performance or 60 fps.

Elliott supplied a CrashReportClientEditor crash report at 18:43. The main editor remained running. Its log identifies an Interchange OBJ UV-index ensure during shoreline import; the reporting helper itself faulted while shutting down its task scheduler. Added valid explicit UV indices to all shoreline faces and reimported. No process termination was used. A clean subsequent restart is being checked.

The current map is still a geography/physics development build. Fourteen bridge ways, separated path components, full-network riding, landmarks, BeltLine south, race loop, traffic including scooters, audio, and visual/performance polish remain pending. Full V2 scope is not accepted.

Final checkpoint checks: latest 20/20 bike regressions pass (bike-shore-recovery-regression); 727/727 installed pavement samples still pass after the lakebed change (park-pavement-after-lake). Clean normal restart log contains no ensure or fatal error entries. Full gameplay and performance remain unverified.


## 2026-09-10 — Expanded world direction

New requirements are captured in WORLD-EXPANSION.md and override conflicting V2 behavior. Current passing water tests validate the old nearby-path recovery only. They do not accept swimming, an independently persistent bike, realistic dismount/remount, hostile encounters, hyperbikes, skaters, horn, automatic lights or post-tunnel transitions. Next foundation work revisits rider/bike separation in the lab before finishing park geography. Full scope has increased; no implementation milestone was completed by this design update.


## 2026-09-10 — Two-stab chase, player gun and coarse compass

Design clarified, implementation pending: first stab dismounts the rider, second before remount kills; voluntary dismount and on-foot shooting are required. The core game is continuously timed during combat and exploration. Death discards the item and resets its randomized search. A coarse four-direction item compass replaces the original warm/cold guidance. Current laboratory and park results do not validate these new systems. Next rider/bike separation work must support voluntary exits, persistent bike ownership and movement-dependent weapon permissions as well as swimming.


## 2026-09-10 — Separate rider foundation

Added APiedmontExplorer, a separately possessed Character using the existing clothed rider asset, a chase camera and provisional procedural walk/swim posing. E dismounts and remounts; WASD or arrows move on foot, mouse turns the view. The unoccupied bike remains parked with movement disabled, so changing possession cannot keep applying old riding input. Remount requires proximity and unobstructed access. Voluntary dismount currently requires speed <=350 cm/s (about 7.8 mph), preserves rider momentum at allowed speeds, and rejects overlapping or obstructed lateral exits. This is not the final high-speed crash/ragdoll system.

Eight live lab checks pass in Tests/Results/2026-09-10-explorer-possession.json: E possession transfer, actual walking while bike stays still, remote remount rejection, E remount, resumed pedaling, high-speed dismount rejection, blocked exits, and thin-barrier crossing rejection. Seventeen retained bike/land regressions pass in bike-land-after-explorer; the three old automatic water-respawn cases are explicitly superseded by swimming tests.

Water crashes now attempt rider separation into surface swimming, retaining the bike at its contact position. The rider alone ignores bike-blocking shoreline guards, can move around the lake, exit onto a bank, return on foot and remount. Six actual-park swim/return checks passed before the final no-teleport fallback guard; rerunning those final changes is required before acceptance. Surface swimming uses a simplified Character flight mode constrained to the water surface; underwater diving, full swim animations and complete shoreline/island traversal are not yet accepted. The old automatic recovery remains historical code but is no longer the intended water-entry route; the new failure guard parks the bike instead of relocating it when no exit is available.

Combat, knife pursuit, player gun, blood effects, timer/item compass/restart loop, traffic and the remaining full-world geography/presentation are still pending. This foundation does not complete the expanded game.

Final movement checkpoint: all seven actual-lake cases pass after the no-teleport and immediate-stop changes, including animated hand travel, bank exit and remount. Archived swim-return-remount report supersedes the earlier six-case result. Procedural arm IK replaces the initial T-pose swimmer; visual review remains necessary before claiming presentation quality.

Visual check completed in the running park: rider is prone at the lake surface and arms animate through a stroke instead of holding the T pose. This is a provisional swim animation, not final realistic presentation. No full milestone or full-game acceptance is implied.


## 2026-09-10 — Timed retrieval and combat prototype

Added a continuously running ten-minute prototype clock after a three-second countdown, a cardinal-only item hint, item loss on death, and a fresh item placement on restart. The final difficulty selector, off-direct-exit placement constraint, complete reachable network, Krog finish, results and saved best times remain pending. The item is currently an orange placeholder sphere.

Added on-foot pistol draw/aim/fire/reload, obstruction-aware shot traces, lethal player bullet hits, first-stab dismount and chase, second-stab death before remount, and successful-remount reset of the two-hit window. Rare encounter scheduling waits five minutes and then has a 35% opportunity to spawn one attacker; some runs have none. Initial live attacker integration passes nine checks, including actual pursuit, escape by remount, a defensive shot with blood particles, attacker defeat, and two-hit death. Combat is provisional: simplified pursuit, posed falls and short blood particles, with realistic ragdolls, production effects and full park navigation still pending. Reloads currently have unlimited reserve ammunition.

Added an over-shoulder camera, H horn with repeat limiting, and head/tail lights responding to night or authored darkness volumes. Native build succeeds; latest integration results are recorded below when complete. Darkness volumes still need placement in the future Krog tunnel. The full game is not complete.

Latest rebuilt integration: 17/17 combat/run rules, 9/9 live pursuit/defense, 8/8 horn/automatic-light and 7/7 real-lake swim/return checks pass. Reports are archived under Tests/Results/2026-09-10-*. These checks do not establish finished visuals, complete geography or target frame rate.

Final bike regression also passes 17/17 retained land/handling cases on the combat build. Total fresh checks for this checkpoint: 58 (17 bike + 17 combat/run + 9 live threats + 8 lights/horn + 7 swimming). Visual quality and full route acceptance remain separate.


## 2026-09-10 — Lake crossing restored

Installed OSM way 102679938 lake bridge deck plus mapped wood spur 146304988, with distinct deck/rail materials, Nanite meshes and full fallback collision. Deck elevation is authored from bank heights with a 12 cm crown; it is not a surveyed bridge structure. Updated editable splines retain those deck heights when rebuilt. Added a bridge-ground exception to dismount water rejection.

Five live checks pass in Tests/Results/2026-09-10-lake-bridge-crossing.json: crossing each direction including 150 cm bank approaches, dismounting on the deck, remaining on foot above the water, and remounting. Imported mesh bounds agree within 0.001 cm. The wooden spur is installed but not independently ride-tested. Final bridge architecture/reference matching, the other twelve mapped bridge ways, remaining network connectivity and full end-to-end acceptance remain pending.


## 2026-09-10 — Seven creek/Northwoods crossings and public plazas

Installed decks and separate rails for OSM ways 146304984, 182302113, 226119763, 226324512, 226324514, 1032063077 and 1050793219. Decks use authored bank-anchored elevations and an 8 cm crown, constrained above sampled terrain; these are not bridge surveys or final architecture. Editable splines preserve their revised deck heights. All 35 live checks pass across these seven crossings: both bank approaches/directions and dismount, non-swimming footing and remount for each. Results: Tests/Results/2026-09-10-wetland-crossings.json. Fourteen imported meshes have verified bounds and full fallback collision. Five mapped bridge ways remain undecked, and full network traversal is still unaccepted.

Added the two public OSM pedestrian areas 1089139296 and Welcome Plaza 1089185052. Private courtyard/terrace areas are excluded. Their pavement follows exact quantized landscape triangles and subtracts existing path footprints to avoid coplanar overlaps. All 445 source triangles pass geometry checks; 25/25 installed collision samples pass. Source intersection data identifies Welcome Plaza as the connection between ways 503237946 and 1089185057 (the latter belongs to an isolated path component in the old centerline-only graph). A full drive through every network junction is still required.

Narrow deck tests exposed dismount capsules touching rail posts at the old 105 cm lateral offset. Reduced it to 90 cm, preserving floor, overlap and swept-barrier checks. Restart now restores gear 1 and zero cadence; encounter spawning runs only after checking timeout. Walking and combat regression results are recorded separately after the rebuild.

Rebuilt regressions pass: 8/8 walking/possession/blocked-exit cases, 18/18 combat/run cases including reset to gear 1, and 9/9 actual attacker pursuit/defense cases. The closer dismount remains protected by overlap and swept-path checks.


## 2026-09-10 — Park Drive and final two northern bridge decks

Generated and installed a unified Park Drive deck for OSM ways 61490793, 1389725669 and 1389725671, plus individual decks for 1384353622 and 1384366242. Park Drive uses a smooth authored elevation field constrained to its six bank endpoints, retaining terrain where higher. It is not a surveyed bridge reconstruction. The overlapping road/sidewalk ribbons share one surface with outer rails, a solid underside, and no internal barriers. Closing faces are subdivided at the deck grid/triangle edges; the first unsplit caps caused invisible approach barriers and were replaced. All fourteen sourced bridge ways now have a verified installed deck midpoint within 2 cm of their editable spline. This is coverage evidence, not full network acceptance or final architectural matching.

Ground probes now ignore bridge surfaces above the wheel's allowable floor level, preventing capture by an overhead deck. A low-deck fixture validates riding underneath without climbing/crashing and support from above. Small-curb handling now permits landings up to 24 cm lower as well as higher, retaining swept up/over collision checks against tall obstacles. One descending sidewalk lip exposed this asymmetry. Final rebuilt route and bike regression results are recorded below when complete.

Config project version now matches the development HUD (0.3.0-dev). Design/ACCEPTANCE.md preserves the complete requested scope and lists unaccepted requirements; it does not replace or narrow V2-SPEC.md or WORLD-EXPANSION.md.

Final rebuilt results: 30/30 remaining-bridge route checks (five OSM ways plus movement across Park Drive), 5/5 lake crossing checks, 5/5 short wooden spur checks, 2/2 underpass fixture checks and 17/17 retained bike/land regressions pass. Reports are archived under Tests/Results/2026-09-10-*. All fourteen sourced bridge ways have installed coverage and scoped live riding evidence. This does not certify every park junction, full connected navigation, architectural fidelity, or performance.


## Path-only Recast navigation foundation — 2026-09-10 [codex-maclaptop]

Installed/saved navigation over tagged paths, dirt ribbons, bridge decks and barriers; excludes open landscape, lake and unrelated scenery. Native editor helper builds geographically centered brush bounds (CubeBuilder resets location, so center is reapplied after building). Agent settings: radius 36 cm, height 180 cm, slope 40 degrees, cell 10 x 2 cm, step 24 cm.

Resolved a UE editor scripting ensure caused by invoking NavigationSystemV1 static Python build-state polling on its class default object: polling now uses a native instance wrapper. The audit unregisters its Slate callback before saving to prevent reentrant saves. Native build succeeds; final editor log has no navigation ensure or play-mode errors. Map save returns true.

Initial routes exhausted the default search-node budget. Exposed that diagnostic (-2 route result), then set Recast DefaultMaxSearchNodes to 32768 and recreated its default filter. Final regular audit: 675/708 reachable samples, 225/236 path pieces fully reachable at their three samples, zero exhausted searches. Dense follow-up verifies all 121 vertices of dirt path 1278380268 reachable. The budget change restores three sampled path pieces without invented geometry.

Remaining isolated sampled paths: 182392760, 182392761, 182398937, 182466135, 226119770, 442709517, 1209254623, 1384353621, 1384353622, 1384366241, 1384366242. Some are northern/boundary fragments or disconnected source spurs. Review genuine public connectors and full BeltLine expansion before adding geometry. Scripts/audit_source_connectors.py records exact shared-OSM-node candidate chains and preserves their source IDs/coordinates; no candidate connector was installed in this milestone.

Evidence: Tests/Results/2026-09-10-park-navigation.json, navigation-default-budget.json and navigation-expanded-budget.json (same date prefix). These are scoped audits, not full ride-through, actual AI path-following, performance, or finished-game acceptance. Next: actual NPC navigation/traffic and missing genuine geographic links; preserve all requirements in ACCEPTANCE.md.
