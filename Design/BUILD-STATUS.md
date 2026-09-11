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


## Runtime attacker navigation — 2026-09-10 [codex-maclaptop]

APiedmontThreat follows complete Recast waypoint routes, refreshes pursuit every 0.8 seconds, turns toward movement, and stops when it has no reachable path target. Nearby off-path targets project within 5 game meters; this remains a path-constrained pursuit implementation, not full terrain pursuit. The physics lab retains its short direct-steering fallback when there is no navigation data. Threats no longer attack while swimming and stop movement when the run ends.

Natural rare encounters now require a complete navigation route from the proposed spawn to the player’s nearby path surface. Projection is followed by another distance/view check; spawns remain 20–35 game meters away, outside the forward view cone, with one live attacker maximum. Existing 300-second opportunities / 35% chance remain unchanged. An editor-PIE-only validation entry point bypasses the probability roll, not reachability, placement, active-attacker limits, or the encounter clock.

Native build succeeds. Live park test chooses a route whose straight line crosses Lake Clara Meer: attacker traveled 10599.8 cm along land vs 2505.2 cm direct distance, using up to 23 waypoints and 30 route requests. It never entered swimming, reached the rider, delivered the first-stab dismount, and allowed remount escape while the clock continued. Real encounter-placement checks also pass. Nine park cases plus nine existing lab combat checks pass (shooting/blood effect/attacker defeat, remount escape, second-stab death and protected start). Evidence: Tests/Results/2026-09-10-threat-park-navigation.json and 2026-09-10-threat-after-navigation.json. Scripts/validate_threat_navigation.py uses sourced shoreline candidate pairs in SourceAssets/Terrain/threat-navigation-candidates.json.

Full-world moving-target pursuit, crowd avoidance, all isolated path fragments, production animation/gore and visual fidelity remain unaccepted. Ordinary pedestrians, scooters, bikes/hyperbikes, skaters and dog/leash traffic still need implementation; this milestone does not satisfy that separate requirement. All outstanding game/world/route requirements remain in ACCEPTANCE.md.


## Build 0.4.0-dev — walker/jogger groundwork and V3 transition
2026-09-10 [codex-maclaptop]

Added APiedmontPedestrian with NavMesh-driven AIController movement, paired walkers, occasional pauses, steady joggers, RVO avoidance, horn/proximity yielding for walkers, and distinct physical bike contacts. APiedmontTrafficDirector streams an initial configurable 24 local visitors from sourced path points; no automatic population on maps without paths. Nine live park checks passed: 18 walkers/6 joggers, pairs, 23 of 24 moving more than 200 cm during observation, no swimming, actual horn yielding, jogger non-yield, low-speed stumble/slowdown, direct high-speed collision and the then-current four-second recovery. These are groundwork, not V3 life or handling acceptance. NPC audio, final animation/art, remaining traffic types, full density and performance are unfinished.

During the work Elliott supplied the full BATTLE FOR THE A V3 spec, now preserved verbatim in V3-SPEC.md, then requested fully working native Mac delivery. V3-DECISIONS.md records Mac .app/desktop alias as replacing Windows .exe and the material changes from V2. ACCEPTANCE.md now covers full V3; the V2 ledger is archived. Build 004 / 0.4.0-dev marks this transition, not acceptance of V3 milestone 1.

The legacy bike regression completed twelve checks before its old shoreline assumption stalled after possession changed to Explorer. It was explicitly stopped for the V3 transition and is not reported as a passing suite. V3 changes water to a two-second nearest-path recovery and replaces the simulation bike, terrain crashes, four-second wipeouts, fatal-bullet/two-stab rules, seven gears, cardinal-only hints and the earlier start/finish. Do not repair or polish those obsolete rules as active targets.

Next required gate: fun Character Movement based arcade bike test map with hills/curbs/grass/stairs/lake and 50 wandering NPCs; terrain cannot tip the bike, direct-hit wipes recover in two seconds, and E enters FPS with working pistol/remount. Reuse validated source geography/bridge/nav/assets where compatible. No V3 gate accepted yet. Reference photo folder was not found; user was asked for its location. Windows environment question was superseded by the explicit Mac request.

## Build 005 — 0.5.0-dev — 2026-09-10 [codex-maclaptop]

Added the V3 Character Movement bike and saved ArcadeBikeLab, with an elevated hill, stairs, root bumps, asphalt/grass, wall, water and 50 wandering visitors. Native Mac editor compilation succeeded. Fourteen live PIE checks pass: population/movement, Character Movement class, both five-gear bounds, upright hill/stair/root traversal at speed, asphalt 1600 cm/s, grass 1200 cm/s, wall stop/bounce, cosmetic steering lean, brake-turn slide, two-second water/path return and direct traffic-hit recovery. Evidence: Tests/Results/2026-09-10-v3-arcade-foundation.json.

Navigation now builds on a later editor tick after asynchronous map loading; the initial same-frame request was locked and produced no usable mesh. Input tests capture the game viewport and separate conflicting key transitions across frames. Script errors and failed exploratory runs were corrected before the final passing run.

This is foundation progress, not V3 milestone 1 acceptance. FPS dismount/remount and pistol are next. Glancing-contact coverage, curbs, nitro, effects/audio, and subjective fun review remain. The existing park still uses the legacy bike; the new bike is isolated in ArcadeBikeLab until the FPS gate is ready. Native shipping app, desktop launch, front end, full geographic route, park life and V3 combat remain pending.

## Build 006 — 0.6.0-dev — 2026-09-10 [codex-maclaptop]

Added BattleRider, an eye-level FPS pawn using the existing rig and pistol groundwork. E parks the arcade bike at a collision-checked exit and possesses the FPS rider; E within 240 cm remounts, with magazine and health preserved. WASD/arrows move, Shift sprints, Space jumps, mouse looks/fires/aims and R reloads with unlimited reserve. Added FPS crosshair, ammo/health and controls HUD. Native Mac compilation succeeded. Fourteen live checks pass, covering real E possession changes, parked bike stability, eye camera, sprint/jump/airborne fire, aim, reload, target damage, remount distance and persistence. Evidence: Tests/Results/2026-09-10-v3-fps-handoff.json. Visually inspected the running FPS view; pistol/HUD/crosshair render correctly.

Remaining before fun acceptance: first-person arms, stronger muzzle/hit/shot feedback, bike firing, nitro, skid/splash effects and audio, horn/lights, explicit curb/glancing/blocked-exit coverage and play review. Rider health/regen includes only provisional test-course recovery; full shared combat health and route checkpoint respawn remain for the game loop. The geographic park has not been switched to the new pawn yet. No V3 milestone or standalone Mac shipping acceptance is claimed.

## Build 007 — 0.7.0-dev — 2026-09-10 [codex-maclaptop]

Added camera-directed pistol fire from the arcade bike, with increasing spread at speed, a one-handed rider pose, magazine/automatic reload, reticle and damage markers. Bike and FPS now share traced shots with renderable muzzle/tracer geometry, brief lighting, gunshot audio, recoil and FPS weapon kick. A visual review caught false damage markers on pavement; corrected and covered by a passing live check.

Close, noncontact passes earn 20 nitro; the same visitor cannot award again for 20 seconds. Enemy kills earn 25. Shift consumes a full meter for a three-second boost, with an original procedural whoosh, FOV/motion-blur change and a visible fill bar. Recovery and dismount cancel boost. New dynamically loaded effect/audio directories are included in cooking configuration; shipping packaging itself remains unverified.

Native Mac editor compilation succeeded. Tests/Results/2026-09-10-v3-bike-pistol-nitro.json has 11 passing checks, including a 2.99-second boost, real close-pass/no-contact reward, repeat-pass guard, camera-target hit, visible effect actor, enemy kill reward, speed-dependent spread and no pavement damage marker. Shared pistol FPS regression has 15 passes, including no shots after run end; boost terrain regression has 14 passes. Live view verified the nitro bar, tracer and one-handed pose.

V3 milestone 1 remains unaccepted: FPS hands, skid/splash/terrain effects, horn/lights, explicit curb/glancing/blocked-exit checks and fun review still need work. Shared player damage/checkpoint health and full weapon/enemy systems remain incomplete. Later park, life, route, front-end/radar, difficulty/audio/performance and standalone Mac requirements remain unchanged.

## Build 008 — 0.8.0-dev — 2026-09-10 [codex-maclaptop]

The arcade bike now has H horn with repeat limiting and walker yielding, plus automatic head/tail lights in dark zones and when the directional sun is below the horizon. Lights continue updating while parked, with a 1.5-second exit delay. Generalized pedestrian warning inputs to accept both the legacy and new bike; moving walkers can respond to the new bike approach. Included retained dynamically loaded audio in cooking configuration.

Saved a permanent 25 cm curb and shaded lighting-test shelter in ArcadeBikeLab. Fourteen live ride-details checks pass: horn/yield/jogger behavior; daylight/dark-zone and parked lighting; remount; safe refusal when every exit is blocked; full-speed curb crossing; glancing contact with one nudge and zero wipeouts; and real traversal through the saved shelter turning lights on then off. Fourteen updated-course checks also pass, including 50 moving visitors, five gears, terrain and direct-impact/water recovery. Evidence: Tests/Results/2026-09-10-v3-horn-lights-ride-details.json and 2026-09-10-v3-curb-shelter-course.json. Native Mac compilation succeeded. Night-sun detection is implemented but this suite explicitly exercises dark-zone/daylight transitions, not a full day/night cycle.

V3 milestone 1 still needs FPS hands, skid/splash/terrain camera and sound effects and actual fun review. The remaining health/checkpoint, park/world/life, enemy/weapon, front-end/radar/difficulty and standalone Mac requirements remain intact and unfinished.

## Build 009 — 0.9.0-dev — 2026-09-10 [codex-maclaptop]

Added owner-only skinned FPS arms using an arms-only derivative of the CC0 Quaternius Casual character. Procedural two-bone arm posing follows the pistol, with finger curls, walking bob, recoil, and a support-hand/reload motion. The view rig is independently framed and has expanded bounds so forward-reaching arms are not culled. The imported mesh uses the existing skin and purple materials.

The initial extraction dropped hand faces because FBX influence totals were not normalized. Corrected the extraction to compare normalized arm influence ratios, preserve control-point indices and original skin clusters, and rebuild polygon/normal/material arrays. The final source retains 1,140 arm polygons and removes 2,016 body polygons. Visual review in the rebuilt editor confirms complete palms/fingers and visible arms; the low-poly grip remains provisional presentation, subject to play review.

Native Mac editor build succeeds. Eight live arm checks pass (asset/finger rig, owner visibility, both wrist targets, recoil, reload support-hand travel and pistol tilt, ammo/grip recovery, cleanup on remount). Fifteen FPS regression checks also pass with the new weapon position, including sprint, jump/airborne fire, aim/reload, actual target damage and possession/health/ammo preservation. Evidence: Tests/Results/2026-09-10-v3-fps-arms.json and 2026-09-10-v3-fps-after-arms.json.

V3 milestone 1 remains unaccepted: skid/splash/terrain camera/sound effects and actual fun review remain. Full world/life/route, health/checkpoints, weapons/enemies, front end/radar/difficulty/audio/performance and standalone Mac delivery remain unfinished.

## Build 010 — 0.10.0-dev — 2026-09-10 [codex-maclaptop]

Added automatic drift on a fresh hard turn above 1350 cm/s, preserving brake-assisted drift. A fixed 160-instance tire-track pool places ground-aligned skid segments, which narrow away over their final two seconds and expire after ten. Water entry emits a bounded 48-drop splash; droplets expire within 1.4 seconds. No effect collision/navigation contribution. The effects actor is destroyed with its owning bike.

Added terrain/grass visual bob, gentle camera impulses and a 3% speed reduction for detected bumps, limited to one event every 0.3 seconds. Capsule orientation and terrain-proof movement remain intact. Water, direct impacts, glancing contacts and wall bounces have proportional feedback. Six original deterministic synthesized sounds cover skid, splash, bump, asphalt tires, grass tires and e-motor; surface/speed/pedaling controls volume and pitch, and parking mutes all ride loops. Sound assets and playback state are verified; final subjective mix/listening acceptance remains pending.

Visual review exposed an invisible lab lake: the spline had negative winding and incomplete Water Body setup, with a grass slab at the surface. Repaired positive linear winding, explicit zone/material/static water setup, and replaced the slab around an authored 2.95 m test basin. Saved ArcadeBikeLab and rebuilt navigation. Actual rendered water, splash droplets and a curved skid trail were inspected in PIE. These are provisional effects/art, not final full-world acceptance.

Native Mac compilation succeeds. Eleven live feedback checks pass: active asphalt/motor audio; automatic unbraked drift with real tracks/skid sound; upright capsule; grass sound/bob; curb response without wipeout; splash plus actual camera displacement; approximately two-second path return; droplet cleanup; dismount; parked silence; and expired/bounded track instances. Fourteen terrain regression checks pass, including 50 wandering visitors, five gears, hills/stairs/roots/curb, asphalt/grass speed, walls, sharp/brake turns and both water/direct-hit recovery. Evidence: Tests/Results/2026-09-10-v3-ride-feedback.json and 2026-09-10-v3-terrain-after-feedback.json. Corrected an initial UPROPERTY test-access error and a grass test coordinate that was actually on the plaza before the passing runs.

V3 milestone 1 still awaits actual fun/presentation review. Full geographic park and route/life, combat/health/checkpoints, all weapons/enemies, front end/radar/difficulty/audio/performance and standalone Mac .app remain incomplete. Continue the full finish-game goal; this milestone is progress, not completion.

## Build 011 — 0.11.0-dev — 2026-09-11 [codex-maclaptop]

PiedmontWorld now uses BattleParkMode, inheriting the V3 arcade bike, FPS rider, shared pistol/effects, horn/lights/nitro and a 50-visitor local director. Its HUD identifies the actual park instead of the lab. This replaces the old park pawn/mode; legacy scripts/rules are historical and must not be used to accept V3 behavior. The complete game loop and enemy systems are still pending.

Placed the PlayerStart two game meters inside mapped gate node 5674178517 on existing path way 61491566, facing inward. The gate corresponds to the eastern 14th Street terminus and the Conservancy entrance map; this identification is inferred from those sources, not an architectural survey. SourceAssets/Terrain/battle-start.json and Scripts/prepare_battle_start.py preserve the georeference, path projection and heading. Runtime spawn is within 0.08 cm of the intended XY with negligible heading error. The actual stone gate/fork presentation remains unfinished. References include two OSM snapshots and the current Conservancy map links.

Real-world integration caught a genuine four-second water return: after returning to a bridge, water logic reused the previous underwater floor and triggered a second wipeout. Water detection now runs after Character Movement refreshes the floor; the bike always checks its floor and explicitly requests a fresh check on water return. The actual lake now returns to a clear path/bridge in approximately 2.01 seconds without retriggering. An old water-test coordinate lay above the newly installed bridge, so the validator now selects polygon-interior water with a verified submerged bed and no bridge deck. Its scripted pedal release/press transitions are separated across frames.

Native Mac compilation succeeds. Thirty directional driving checks cover the main lake crossing, wood spur, nine wetland bridges, three Park Drive bridge ways and its cross-deck route, with no false water recovery. Four additional lake-bridge checks pass for FPS dismount, dry grounded walking, pistol fire and remount. Together with mode/population/path/lake checks, the park suite has 39 passes. Two focused startup checks pass, and all 14 lab terrain/handling regressions pass after the floor timing change. Evidence: Tests/Results/2026-09-11-v3-park-and-bridges.json, v3-fourteenth-start.json and v3-lab-after-park-recovery.json (same date prefix). These scoped checks do not certify every junction, high-speed full-world traversal or performance.

The finish-game goal remains active. Next major work is park landmarks/foliage and remaining world/life/route, followed by the complete V3 combat, game loop/front end/radar/difficulty/audio/performance and native standalone Mac app. Full presentation/fun acceptance remains pending.


## Build 012 — 0.12.0-dev — 2026-09-11 [codex-maclaptop]

Created a native arm64 Development app on Adam Assets and a desktop symlink named Battle for the A.app. Added a standalone launch menu, Start Park Ride, Instructions, graphics presets, Quit, and Escape pause/resume. Editor PIE remains immediately playable. The menu explicitly identifies the unfinished park playtest; this is not the complete V3 front end or campaign. Default Mac startup is PiedmontWorld, with 1080p windowed settings and a 60 FPS cap, not measured 60 FPS acceptance.

Three packaging issues were diagnosed and fixed: missing editor GameFeatureData asset-manager cook rule; UAT archive copying the executable-only bundle rather than the complete staged app; and a standalone startup crash in UConversationRegistry/UGameFeaturesSubsystem, pulled in by Aura's AllToolsets dependencies. Aura now has TargetAllowList Editor, preserving development use while excluding its dependency chain from the Game target. Finalization checks for cooked IoStore content, copies the complete staged app, applies an original icon, signs locally, and installs the desktop link. Config/DefaultEngine.ini remains untouched/untracked; AndroidFileServer settings are denied from staged config.

Native Game/Editor builds and BuildCookRun pass. The packaged app passes display-free startup: loads PiedmontWorld, selects BattleParkMode, initializes the Home menu, and exits 0 with no errors. Codesign verification passes. The first rendered launch exposed the now-fixed plugin crash; the Mac locked before visual retesting. Manual unlock requested. Start/instructions/options/pause/quit, packaged movement/dismount/fire/remount, audio and performance remain unverified in the corrected rendered app. Report: Tests/Results/2026-09-11-v3-mac-package.json.

Final app: /Volumes/Adam Assets/Unreal/Builds/BattleForTheA/Mac/BattleForTheA.app (~917 MiB). Full V3 world, foliage/life, route, enemies/weapons, retrieval/checkpoints/radar/difficulty/saves/audio/performance remain unfinished. Goal stays active.


## Build 013 — 0.13.0-dev — 2026-09-11 [codex-maclaptop]

Added reflected FBattleDifficultyRow and the actual cooked DT_Difficulty asset, imported from SourceAssets/Data/Difficulty.csv. Easy/Medium/Hard set 900/600/300 seconds and 20/50/90 nearby walking/jogging visitors; the director reads the configured jogger ratio. The isolated lab remains at 50 visitors. Table fields also preserve counts/speeds/fractions for the remaining V3 NPC/enemy/weapon/radar consumers, which are not implemented merely by defining their values. SourceAssets/Data/README.md identifies active versus pending consumers.

The standalone menu now offers Level Select with row-derived time/warning labels. Selection opens a fresh park with the chosen profile. The HUD shows countdown/timer in both bike and FPS modes, with red timer below a minute. Timeout pauses into THE A WINS THIS TIME with Retry, Level Select and Quit; Escape cannot bypass an expired run. Actual rendered menu navigation and Retry reload acceptance remain pending because the Mac is locked.

Native editor build, Data Table import/save and Mac BuildCookRun pass. Scripts/test_mac_difficulty.py runs the real cooked executable with an opt-in development-only headless probe. Each profile passes eight checks: timer ticking after countdown, actual live/desired crowd count, dismount, pistol ammo consumption, remount, pause, resume and timeout-to-menu. All three processes exit 0; results in Tests/Results/2026-09-11-v3-native-difficulty.json. The first report collector looked for a file redirected by Unreal's packaged file sandbox; corrected collection to the explicit structured native log record and reran all three successfully. Tests invoke gameplay methods, not physical keyboard/mouse events, and do not establish rendered UI, audio or performance.

Updated complete, locally signed desktop app to 0.13.0; original build 012 retained under Builds/BattleForTheA/Mac/Previous. Default branded-app headless startup selects Easy, initializes Home and exits cleanly. Source commit and user guide preserve remaining scope. Full V3 game is not complete; goal stays active. Native visual/control test still awaits unlock, while other implementation can proceed.


## Build 014 — 0.14.0-dev — 2026-09-11 [codex-maclaptop]

Added ABattleQuest and a floating, rotating, glowing lost-phone Artifact. The current park supplies 446 potential sampled path positions before filtering. Placement excludes the first 5000 game cm around the start and a 1200 cm corridor along the direct start-to-exit line; requires a paved hit, dry bike footing, clear capsule space, and nonpartial navigation both from start and toward the exit waypoint. Pickup requires proximity plus clear line of sight, works on bike or foot, sets shared item state and removes the phone. No abbreviated win condition was added.

Added a north-up circular radar with 1176 cached resampled source-path segments, analytic circle clipping, player heading, parked-bike marker, hostile-dot input, and the Artifact's pulsing gold blip only within the difficulty's radar range. Outside range a rim arrow shows direction. After pickup, a gold navigation path and target arrow point toward the park's BeltLine side; route refreshes every two seconds. RadarRange is now an active Data Table consumer. Actual rendering, hostile dots and visual/pulse readability remain unverified while the Mac is locked.

The first return waypoint is the nearest currently installed centerline to mapped park-side connector node 5674504871 (way 741964055). It lies on way 182302109 at XY 11322.24111595, -8938.93870108. The mapped connector is still about 66 real metres away: this is not the completed 10th/Monroe connection or the full home route. Scripts/prepare_battle_exit.py and SourceAssets/Terrain/battle-park-exit.json preserve that projection and limitation. Continue the actual connector/BeltLine/Cabbagetown work; do not present reaching this waypoint as winning.

Native Game/Editor compilation and Mac packaging pass. Expanded real packaged headless checks pass in Easy, Medium and Hard: previous timer/crowd/dismount/fire/remount/pause/resume/timeout checks, quest readiness, range gating, placement exclusion, segment clipping, pickup, route generation, and rejection of pickup through a temporary blocking wall. Medium explicitly collects on foot; Easy and Hard collect on the bike. Tests teleport for the collection fixtures and do not prove physical keyboard input, full route ride-through, visual rendering, final audio or performance. Evidence: Tests/Results/2026-09-11-v3-native-quest.json. Fixed native pointer/Canvas access compile errors before passing. No failing build is installed.

Installed complete signed desktop app 0.14.0, retained build 013 under Previous, and confirmed normal branded headless Home startup with a ready Artifact and clean exit. Cleaned two known failed temporary app copies. Mac remains locked on recheck; visual acceptance still awaits unlock. Full V3 world/foliage/life, complete route, enemies/weapons, health/checkpoints, saves/results/outfits/audio/performance/World Partition and final shipping acceptance remain required. Goal stays active.


## Build 015 — 0.15.0-dev — 2026-09-11 [codex-maclaptop]

Installed the 79.577 real-metre source connection from existing park pavement node 12926353123 through ways 182302109 and 1396654821 to node 2396740017, the northern endpoint of Eastside Trail way 741964053. Its authored 320 game-cm width matches the arcade paths; it is not a surveyed width. Source network, generated OBJ and source provenance are retained. The single mesh contains 599 terrain-conforming triangles; overlapping park pavement is subtracted to avoid z-fighting. All 43 installed collision samples match expected elevation within 0.000041 cm and navigation returns a 2651.366 cm route. Headless import originally failed on an unavailable editor subsystem and then the engine async-loading navigation lock; direct mesh settings and completion of the engine's own delayed unlock resolved these, with save only after checks pass.

The Artifact return waypoint now uses a generated header derived from that sourced Eastside endpoint. Connector splines are explicitly excluded from random Artifact placement, while remaining visible in radar/routing. The full trail to Krog and home remains unfinished; there is no shortcut victory at this waypoint.

The opt-in BattleConnectorAudit uses ordinary player-controller W/A/D input and real CharacterMovement to ride both directions, teleporting only to each leg's starting fixture. The cooked Mac app passes: two legs, 5119.41 cm travelled, 2.45 cm maximum centreline deviation, zero wipeouts, process exit 0. The initial failed test used an editor-only helper that did nothing in the packaged game; corrected direct input delivery passes. Evidence: Tests/Results/2026-09-11-native-connector-drive.json and 2026-09-11-beltline-connector-installed.json. Display-free tests do not establish rendered appearance or physical keyboard/window focus.

A separate coordinate audit found existing X=east, Y=north, Z=up world mapping conflicts with Unreal's left-handed geographic convention (engine GeoReferencing maps north to -Y). This produces mirrored geographic orientation despite internally consistent collision/radar. Build 015 does not correct it. Design/WORLD-EXPANSION.md records the required coherent terrain, mesh, water, spline, anchor and radar correction before further route expansion. Mac visual acceptance and Fab authentication remain pending; other game work can continue.

Final build 015 Game/Editor compilation and packaging pass. All three cooked difficulty/Artifact regressions pass in Tests/Results/2026-09-11-build015-native-quest.json, preserving the previous build report. Installed and locally signed complete 0.15.0 Mac app; previous 0.14.0 retained under Mac/Previous. Branded display-free default startup opens Home, initializes a ready Artifact and exits 0. Desktop shortcut and user guide updated. Full V3 remains incomplete; goal stays active.


## Build 016 — 0.16.0-dev — 2026-09-11 [codex-maclaptop]

Corrected the geographic handedness identified in015. Converted 330 authored actors from retained east/north/up source centimetres to Unreal east/south/up placement, with north now negative Y. The R16, OBJ and original source network coordinates are preserved; source geometry uses a reflected actor transform, while placement-only actors retain normal scale. Landscape collision is recreated after the determinant change. A WorldSettings marker prevents double migration. Generated start/exit world coordinates and the radar's north-up projection/heading use the same convention. Terrain metadata distinguishes retained source placement fields from actual world location/scale. Active terrain/lake/pavement/bridge/spline/connector/start importers and the V3 park validator now convert source fixtures; Scripts/GEO-PIPELINE.md explains the boundary.

Conversion and independent saved-map reload each compare 1,511 collision samples, covering source paths/decks and a 400-point terrain grid. All pass. Ten existing seam samples can select different surfaces after float rounding; the alternatives were explicitly measured within 2 cm before conversion and remain under 3.1 cm vertically (maximum 2.9307 cm). They are preserved as measured seam differences in the reports, not silently dropped. Start location/heading match the converted gate; saved navigation gives a 40847.00 cm route to Eastside. Initial conversion with cached terrain collision failed and was not saved; recreating collision resolved the terrain mismatch. Asset completion before measuring also prevents unloaded mesh collision from masquerading as terrain-only coverage.

Native Game/Editor compilation and Mac packaging pass. New cooked geography test verifies world marker/start/exit convention, radar cardinal axes, island exclusion, and real timed lake recovery. The bike returns to a gravel shore path outside the water boundary in 2.002 seconds with exactly one splash. The initial fixture assertion omitted valid gravel paths; corrected it to the game's actual path types (including safe bridges over the water boundary), then reran successfully. Actual connector W/A/D riding passes both directions after conversion: 5115.47 cm, 2.48 cm maximum centreline error, zero wipeouts. Easy/Medium/Hard timer/crowd/FPS/Artifact/radar/blocked-pickup/route/pause/timeout regressions all pass. Evidence: Tests/Results/2026-09-11-coordinate-conversion.json, coordinate-reload.json, native-geography.json, build016-native-connector.json and build016-native-quest.json (same date prefix).

Installed complete locally signed 0.16.0 native Mac app behind the desktop shortcut; build015 retained under Mac/Previous. Physical window/keyboard, rendered water/landmarks/radar, audio and frame-rate acceptance remain unverified while the Mac is locked. The full V3 route, foliage/park life, enemies/weapons, health/checkpoints, menus/results/outfits/saves, audio/World Partition/performance and shipping acceptance remain unfinished. Goal stays active. Next geography work can now extend the sourced Eastside corridor toward Krog and Cabbagetown without preserving the former mirrored orientation.


## Build017 — 0.17.0-dev — 2026-09-11 [codex-maclaptop]

Extended the world along the node-connected 3107.894 real-metre Eastside mainline from Monroe node2396740017 to Irwin node6016404357, through eight source ways. Added26 pavement/deck/rail meshes (60084 triangles) and8 ordered source splines. The northern join subtracts existing pavement footprints to avoid coplanar overlap. Sources remain ENU and are placed with the established ESU conversion. Native navigation across the extension passes. All1566 installed centerline collision samples are within0.208gamecm of the intended surface profile.

Three OSM bridges use authored deck grades between USGS bare-earth embankment samples12realmetres beyond the mapped ends. Smooth approach blends replace road-level DEM dips; locally finer triangles reduce nonlinear profile interpolation error. Guardrails are authored104cm high, with two horizontal rails and posts no farther than180cm apart. These are functional prototype structures, not surveyed or photo-matched architecture.

After Artifact collection, the gold route joins the nearest mainline point through navigation, then follows the ordered source trail/bridges to Irwin. Artifact placement still uses the park-exit exclusion and the extension remains ineligible for Artifact spawning. The radar target moves to Irwin; this is not the final Cabbagetown objective or a shortcut victory. Full checkpoint sequencing, tunnel and home remain required.

The real cooked Mac full-route test passes both directions in third gear through normal player-controller W/A/D events and CharacterMovement. Crowds are disabled only in this opt-in development probe to isolate terrain/decks. Travel206903.69gamecm (about6.2km of mapped distance), maximum centreline deviation48.59cm, zero wipeouts, and all12401grounded samples on pavement. Evidence: Tests/Results/2026-09-11-native-eastside-drive.json. A first compile attempt used a nonliteral checked format string; corrected before successful packaging. Source interpolation initially exceeded the desired profile tolerance; refined bridge regions and reran installed collision validation successfully.

All three updated native quest/difficulty profiles pass, including canonical route termination at the Irwin target; two-second water recovery and the short Monroe connector also pass. Results retain separate build017 filenames. Installed complete locally signed native app0.17.0 with build016 preserved under Previous. Mac remains locked, so rendered/physical-input/audio/performance acceptance is pending. Full V3 remains unfinished and the goal stays active.


## Build018 — 0.18.0-dev — 2026-09-11 [codex-maclaptop]

Added the 727.175 real-metre connected route from Irwin through both Krog Tunnel
portals to the southern sidewalk exit, six OSM ways. The sidewalk changes name
inside the tunnel; roadway44062162 supplies the full portal span rather than
ending the roof at the end of tunnel-tagged cycleway722838795. Eight meshes add
the trail, roadway, walls, roof and columns. Dimensions are authored for arcade
play at 1:3 world scale, including 270gamecm headroom, not photo-matched or surveyed.
Graffiti, echoing audio and final Krog architecture remain unfinished.

The Landscape now uses the existing lakebed plus a local tunnel excavation.
2,123 height samples are lowered; every sample outside its allowed corridor is
byte-identical. The floor grade is authored between bare-earth samples outside
the portals. The DEM does not describe the subterranean interior. A curved roof
join initially had gaps; shared cross-sections corrected those before map save.
The first commandlet attempt used a relative project filename and failed before
loading; a subsequent Python component-access error was also fixed before save.

Installed collision passes372 path samples (max error0.077cm) and50 overhead
samples (max error0.249cm), with1978 park/Eastside surface samples exactly
preserved. Navigation through the new stretch passes. The return radar route now
continues through the tunnel; home and checkpoint/finish sequencing remain
unfinished. Darkness volumes cover the tunnel. Evidence:
Tests/Results/2026-09-11-krog-installed.json.

Native Game/Editor compilation and packaging pass. The cooked Krog drive covers
both directions through normal W/A/D input and CharacterMovement:48,014.81gamecm,
63.66cm maximum centreline deviation, zero wipeouts, all3010grounded samples on
pavement. Tunnel illumination state passes476lit samples and0unlit after the
activation allowance. Crowds are disabled only for this route fixture. This
checks light state, not the rendered beam or artwork. All three difficulty,
Artifact/radar/route, dismount/fire/remount, pause and timeout profiles pass. Lake
recovery passes in2.001seconds. Evidence: native-krog-drive.json,
build018-native-quest.json and build018-native-geography.json in Tests/Results
with the2026-09-11 prefix.

Installed and locally signed complete native0.18.0 app behind the desktop icon;
build017 retained under Previous. Physical Mac window/input, rendered
visuals/audio and frame rate remain unverified while the Mac is locked. Full V3
remains incomplete; the goal remains active.
