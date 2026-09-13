# Piedmont Ride — world expansion

2026-09-10 [codex-maclaptop], based on Elliott's requested additions.
Status: approved feature direction; implementation pending. This document supersedes conflicting behavior in V2-SPEC.md, especially automatic water respawn and the tunnel being only an endpoint. Numeric tuning below is an initial implementation choice, not a user-specified balance value.

## World and progression

One continuous Atlanta world with consistent traffic, terrain, collision, injury, lighting and vehicle rules. The core game combines a timed retrieval race, obstacle avoidance and on-foot shooting. Swimming and hill exploration are supported, not restricted to the direct route, but the countdown continues through riding, dismounting, walking, swimming, combat and recovery. An untimed free-exploration mode is not part of the currently requested core loop.

Preserve the race completion condition at Krog Street Tunnel, but separate completing the race from leaving the world. The tunnel must be traversable, with its dark interior, graffiti and lighting. Crossing through its far end can later hand the player into a different game area/activity. Provide an explicit transition interface; the destination and genre are not yet chosen. Do not fabricate or ship an unfinished destination as playable.

## Bike, dismounting, swimming and recovery

- Separate the rider character from the bike actor. Represent riding, crash/dismount, on-foot, swimming, remounting and dead states explicitly; only the active controlled body receives movement input.
- At water entry, leave the bike at the contact point on the bank, with a stable identity and stored transform. The rider can fall into the water and swim to any reachable shore, including exploring around Lake Clara Meer and its island. No automatic nearest-path teleport or forced short swimming timeout.
- The bike remains where it hit the lake even when the player swims away, changes camera or travels across streaming boundaries. Return to that location to remount. Show its position on the map/HUD so it can be found again. Never silently move it to the player's chosen exit shore.
- Support climbing out at physically suitable banks and walking back to the bike; reject exits through walls, steep inaccessible banks or bridges overhead. Swimming cannot let the bike ride on water.
- Water camera transitions, buoyancy/surface movement, swim animation and safe shore transitions must work together. No drowning mechanic has been requested; surface exploration should not be limited by an invented oxygen timer.
- Replace the current instant R water recovery. R must not summon or teleport the abandoned bike. Use an interact/remount action near it (initial binding E). Preserve normal riding controls. Use WASD and arrows for walking/swimming via contextual input so swimming arrows do not change gears.
- Improve crashes through velocity-dependent rider separation, grounded impacts and recovery animation, with ragdoll or a properly rigged physics reaction. The bike and rider should not remain rigidly joined or clip through the terrain. Camera follows the rider during dismount/swimming and returns to the bike on remount.
- Ride up and down the real hills and accessible grassy slopes. Preserve downhill acceleration, uphill gear demands, grass traction and obstacle collision. Test Oak Hill and representative terrain grades, not only laboratory ramps.

## Living traffic

Retain pedestrians, joggers, ordinary bikes and scooters. Add rollerskaters with skating/turning animations and dogs that remain attached to their owners by visible leashes. Owner/dog spacing and leash span create readable path obstacles; no detached or stretching dog-owner pairs.

Add occasional hyperbike riders capable of 50 mph (2235.2 Unreal cm/s at the existing displayed speed convention). This does not raise the player's bike speed cap. They overtake or approach as traffic hazards, with visible riders, meaningful braking/avoidance, collision and enough spawn distance to react. Their route look-ahead and swept movement must handle the speed without tunneling through actors. Spawn frequency is independently tunable from ordinary traffic density. Confirm that the 1:3 map still leaves a fair reaction window.

## Rare hostile encounters

Occasional fictional gun and knife attackers can emerge from plausible places and threaten the player. This is intermittent danger, not constant combat; some full runs should contain no hostile encounter. The player can draw and fire a gun while on foot to defend against attackers; player shooting is now required (see Combat and retrieval revision below).

Use a central encounter director with a shared cooldown and at most one active hostile encounter. Initial tuning target: zero or one hostile encounter in a typical ten-minute outing, with at least five quiet minutes between encounters and a protected start/restart period. Use discrete eligible encounter opportunities with explicit no-encounter outcomes; do not roll a spawn chance every frame. Values remain data-driven for playtesting.

Attackers spawn out of view at valid locations, then visibly emerge with audio/animation cues. Do not spawn an attacker already intersecting the rider or place unavoidable hits at a blind corner. Gunshots use line-of-sight, a visible aiming delay and collision against cover; shots can miss. A bullet hit is fatal: stop control and the active run, show failure, and restart from the original start with a fresh objective/timer, not the last checkpoint. Preserve saved best scores and settings. Prevent multiple hit callbacks from restarting more than once.

Knife attacks are rare and use the two-hit chase rule: the first stab causes a bloody injury and forces the rider off the bike; the attacker then pursues on foot. A second stab before successful remount kills the player and restarts the whole run. See the detailed state rules below. Do not imply ordinary walkers or dog owners are hostile.

Integrate the earlier Hard-only Kroger pursuer into this shared encounter budget rather than stacking it with unrelated attacks. Difficulty can change traffic and challenge, but rarity remains a requirement on every setting. Death always clears the current retrieved-item state and restarts the timed retrieval run from its beginning.

## Horn, lights and environment

Add a bike horn (initial binding H) with audible directional sound and sensible nearby NPC reactions. The horn does not guarantee clearance, stun actors, or remove collisions.

Fit a real headlamp and rear light. Turn lights on automatically in dark areas and the Krog Street Tunnel, using authored darkness zones plus time-of-day state. Add hysteresis so thresholds do not flicker; tunnel lighting activates before visibility drops. Headlamp illuminates the route and obstacles rather than being only an emissive mesh. Return to ambient-light behavior after leaving the tunnel. Avoid relying on unsupported automatic screen-luminance readings.

Trees and park benches must match Piedmont Park's species, forms, materials and placement. Preserve the existing requirement for willow oaks, magnolias/hardwoods and geographically grounded prop placement. Obtain usable visual references before claiming a match; the photos referenced by the original spec have not been attached. Keep trees and benches solid and leave path navigation clearance. Use the real hills instead of flattening the park for prop placement.

## Implementation order and acceptance

1. Revisit the bike laboratory to separate rider/bike and test crash, swimming, bank exits, walking, stationary bike persistence, return and remount. Preserve the passing gear/steer/brake/terrain regressions. Water respawn tests are superseded, not proof of this new behavior.
2. Finish bridges, connectivity and bank access in the real park. Test full lake exploration and riding representative hills; ensure the abandoned bike stays at the original entry point across streaming.
3. Build recognizable trees, benches and landmarks, and extend the BeltLine through a traversable Krog Tunnel. Introduce horn and automatic light components with bike and tunnel integration tests.
4. Build the continuously timed retrieval loop, coarse item compass and complete death/restart semantics, retaining existing race rules where not superseded.
5. Implement ordinary traffic, skaters, owner/leash/dog groups and 50 mph hyperbikes. Then add on-foot player shooting and rare hostile encounters with deterministic test seeds, cooldown/no-spawn tests, cover/miss tests, two-stab chase/remount tests and fatal-hit restart validation.
6. Polish animations, blood/impact effects, swimming cameras, audio and performance. Keep the eventual post-tunnel destination as a separately defined extension.

Acceptance examples: swim a complete lake circuit, exit on a different bank, find the original bike unmoved, walk back and remount; crash at several speeds without mesh penetration; ride uphill and downhill; see a fast hyperbike early enough to evade; encounter a coherent dog-owner leash pair and a skater; sound the horn; enter and exit the tunnel with stable automatic lights; complete several ordinary runs with no attack; survive a first stab, escape the chase and remount; take one bullet hit and restart the run exactly once from its beginning.


## Combat and retrieval revision — 2026-09-10

2026-09-10 [codex-maclaptop], incorporating Elliott's next clarification. These rules supersede earlier injury-only stabbing, absent player weapons and untimed exploration assumptions. Implementation remains pending.

### Knife encounter state rules

First successful stab while mounted forces a crash/dismount, blood effects and a recoverable injury. The knife attacker actively chases the rider, who can run back to the bike or defend themselves on foot. A second successful stab before remount is fatal. If initially attacked on foot, the first stab causes injury/stagger and begins the same chase window; the second before remount is fatal.

Count actual confirmed attack contacts, not overlap ticks. A single stab animation must never register twice. Provide a readable windup and recovery between attacks, so the player can regain control after the first forced dismount. Do not pause the race clock during hit reactions.

Successful remount ends that two-hit vulnerability window. It does not teleport the bike, remove the attacker or award time. The rider must actually reach the bike and complete remount before the second hit. Killing or escaping the attacker is also a viable way to avoid a second stab; injury is not an automatic delayed death. Subsequent encounters still obey the global rarity/cooldown rules. Remount resets the encounter's stab count; this is the implementation interpretation of Elliott's 'before they get back on the bike' condition.

### Voluntary dismount and on-foot defense

Allow normal dismount and remount independently of crashes or water. Initial controls: E dismounts when a safe adjacent location is available and remounts only within reach of the persistent bike; 1 draws/holsters the gun on foot, right mouse aims, left mouse fires, and R reloads on foot. These are provisional bindings; keep mounted W/gear arrows/steering/Space controls unchanged and keep WASD plus arrows for on-foot movement. A blocked exit must not place the character inside a wall or into the bike collision. A moving dismount must account for speed and momentum rather than erase them.

Gun use is allowed while standing/running on foot, not while riding, swimming, remounting, ragdolling or dead. Remount holsters the weapon and clears held-fire input; unpossessed rider/bike actors cannot keep firing or moving. Aiming, drawing and firing do not stop the countdown. Add a readable ammunition/reload state with initial values tuned in the lab rather than inventing a final weapon balance now.

Make combat visually strong and bloody: muzzle flash, recoil and recovery, directional sound, hit reactions, blood particles/decals and convincing collapses. Bullet traces must stop at solid cover and use the muzzle as well as camera aim so shots cannot originate through walls. Include impact effects appropriate to the surface and clear feedback when an attacker is defeated. Keep effects bounded/poolable for frame rate and visibility. Exact weapon models and animation assets remain to be selected; these effects have not yet been built.

Retain rare enemy gun encounters with aiming cues and possible misses. One enemy bullet that hits the player remains fatal, whether mounted or on foot. Defeating an attacker does not reset the race clock or replace the retrieval objective. The game remains an obstacle-avoidance race with intermittent combat rather than a constant succession of gunfights.

### Coarse item compass and full restart

Before pickup, show a coarse cardinal hint toward the item's general area: NORTH, EAST, SOUTH or WEST. Quantize the horizontal player-to-item bearing into four sectors using the map's north/east axes, with sector hysteresis to avoid boundary flicker. Never display an exact bearing, meter distance, item map pin, world-space marker through cover or continuously pointing needle. Do not use a tighter hint at close range; the player searches visually to finish the retrieval. The old warm/cold proximity pulse is superseded by this compass as the required pre-pickup guidance.

After pickup, replace the search hint with the existing exit/checkpoint/finish guidance. A visible item can still be collected normally; its exact location is never leaked by the compass or minimap.

On either a fatal gun hit or the second knife hit, end the run once, clear item ownership and checkpoint credit, and restart at the original start with a reset difficulty timer and a newly randomized valid item location. The player must find it again, even if it was already collected before death. Avoid reusing the previous location when multiple valid locations exist. Preserve best scores and settings only; no mid-run inventory/checkpoint advantage survives death. The original pickup placement constraints still apply. Restart only from the death/results flow, never automatically multiple times from repeated damage callbacks.

Additional acceptance: voluntarily dismount and remount with the bike staying put; dismount after one stab and observe an actual chase; evade and remount before the next hit; verify a second distinct stab before remount causes exactly one failure; draw and fire on foot but reject fire in every forbidden movement state; timer advances through aiming, firing, reloading, injury and swimming; take a fatal bullet after collecting the item and verify the new run has no item and a fresh search location; check compass sector transitions and ensure UI exposes no precise item distance or position.


## Coordinate handedness audit — 2026-09-11 [codex-maclaptop]

Before further geography expansion, correct and validate the current georeference handedness. Existing terrain/path source uses X=easting, Y=northing, Z=up with positive scales. Unreal uses a left-handed world: its installed GeoReferencingSystem.cpp lines 167, 204–205 and 234–235 explicitly map northing to negative UE Y. Thus the current world has a mirrored geographic orientation even though paths, collision and north-up radar agree numerically. This is not fixed in build 015.

The correction must be coherent across DEM row order/Landscape location, path and bridge meshes/winding, splines, lake/island collision and hazards, PlayerStart heading, route anchors, crowd samples, radar axes and validators. Preserve original geographic data and use an explicit shared transform rather than a display mirror. Rebuild collision/nav and rerun water, start, bridge, drive and quest tests before installation. Check known east/north landmarks in a rendered camera when unlocked. Do not treat existing numeric route tests as proving geographic handedness.


## Handedness correction implemented in016 — 2026-09-11 [codex-maclaptop]

The preceding015 defect is corrected in the saved map and native radar/anchors. Original ENU source geometry stays unchanged; apply Scripts/battle_geography.py exactly once at Unreal placement. Converted map marker: BattleGeography_ESU_v1. Terrain's actual actor transform uses metadata world_location_cm/world_scale (negative Y), not the legacy source baking fields. Both conversion and reload pass 1,511 collision comparisons; cooked connector, water recovery and all three quest profiles pass. Rendered acceptance is still pending while locked. Use the retained eastside-krog-corridor-osm.json to continue the actual trail/home route, keeping surveyed alignment separate from authored arcade widths and bridge elevations.


## Eastside installed to Irwin in017 — 2026-09-11 [codex-maclaptop]

Source route: SourceAssets/Terrain/eastside-trail-network.json. Height policy: eastside-height-profiles.json. Rebuild order: prepare_eastside_trail.py, bake_park_pavement.py with --network eastside-trail-network.json --output-dir EastsideTrail --prefix EastsideTrail --include-bridges --height-profiles eastside-height-profiles.json --subtract-park --subtract-network beltline-connector-network.json, bake_eastside_rails.py, install_eastside_trail.py. Install requires the converted map. The three bridge grades/rails are authored arcade structures; full architectural matching remains pending. The native two-way ride passes.

Continue from Irwin node6016404357 toward the tunnel and Cabbagetown. Retained source ways1043756762,722838793,1421664781,1353860787 and722838795 trace the named trail toward/through the Krog area; validate the intended sequence against the source graph before building. Way722838795 is tunnel=yes and must not be draped over the railroad berm. Road tunnel44062162 is a distinct nearby Krog Street roadway. Need a properly graded/carved floor, actual overhead structure and safe headroom; do not label a surface crossing as the tunnel. Full source corridor/landmarks/checkpoints/return-home finish remain incomplete.


## Tenth Street traffic and crossing encounters — 2026-09-12 [codex-maclaptop]

User requirement: represent broad 10th Street along Piedmont Park, its bike lane and moving cars as obstacles. Preserve the installed graded road and separate cycle track. Add hazardous vehicle crossings at Monroe and the Krog approach, including the intersection immediately before the tunnel. Irwin Street is the likely additional pre-Krog crossing; confirm its placement against the retained map graph before authoring encounters.

Add an occasional crashed scooter rider with two or three bystanders helping near the north Krog tunnel approach. This is an ambient scene, not every run. Proposed staging: rider seated or down beside a fallen scooter, one helper kneeling, another warning approaching traffic; leave a visible navigable route around them. No personal backstory in game content. Spawn before the player can see the scene, use a run-level rarity gate, and prevent overlapping traffic spawns or repeated appearance during one visit.

10th Street should include Peachtree Road Race atmosphere: race banners, folded barriers, water tables and a few runners/spectators at the park edge. Keep active vehicle lanes readable and open during normal gameplay; race dressing should suggest preparation or aftermath rather than put normal traffic through a fully closed race course. This is a proposed interpretation of the user's request to retain cars alongside race details.

Sources checked: Atlanta Track Club course page https://www.atlantatrackclub.org/course and press center https://www.atlantatrackclub.org/press-center confirm the Piedmont Park finish and 10th Street finish chute. Discover Atlanta https://discoveratlanta.com/things-to-do/outdoors/beltline-trails/ identifies the Monroe crossing at 10th Street. Geographic positions still require source-map checks; these references do not establish signal timings or exact traffic lane behavior.

Status: requirements recorded; moving cars, crossing controls, race dressing and scooter assistance encounter are not yet implemented or present in installed build 044. Acceptance must include a complete ride past all crossings, car braking/turning and collision checks, no vehicles appearing inside the player, a usable bike lane, and both scene-present and scene-absent runs.


## Concrete surface presentation — 2026-09-12 [codex-maclaptop]

Identified sleeper support as Park pavement SM_ParkPlaza_Concrete_4_18 using M_Concrete; it is mapped plaza pavement, not missing landscape. Created M_ParkConcreteWorld with warm grey aggregate variation, roughness and 180cm world-space slab joints. Compared actual runtime screenshot before/after; foreground material reads substantially better. Applied to47 matching concrete actors in review map, then47 in main PiedmontWorld; import/install commands exit0. Geometry/collision unchanged, main map saved. Scripts create_park_concrete_candidate.py and install_park_concrete.py reproduce the work. Other pale background surfaces remain unresolved; no whole-world visual/performance acceptance. Not yet packaged into desktop build044. Evidence Tests/Results/2026-09-12-concrete-install.json and work/ambient-sleeper-runtime-render/sleeping.png.


## Asphalt surface and collision-readiness fix — 2026-09-12 [codex-maclaptop]

Pale background paths in sleeper review identified as SM_Park_Asphalt_4_17/4_18 with M_Asphalt. Added M_ParkAsphaltWorld: dark aggregate variation, high roughness, low specularity, no concrete-style joints. Actual native offscreen before/after inspected; path now distinguishes clearly from concrete. Applied to30 matching actors in main map after review. Geometry and collision unchanged; install exit0. Not packaged; desktop remains045. Evidence Tests/Results/2026-09-12-asphalt-install.json and work/asphalt-after/sleeping.png.

Surface scan initially returned168 Landscape samples because collision assets were still loading. Waiting via finish_editor_asset_loading() before tracing yields105 Landscape samples plus asphalt/concrete/plaza hits (and one sleeper component hit). Added that wait to find_sleeper_sites.py and reusable audit_park_surface_materials.py. Earlier scanner terrain labels were unreliable; installed sleeper remains supported by separate native runtime tests. No need to relocate it solely from that earlier label. Future placement scans must wait for collision readiness.


## Reaching animation candidate — 2026-09-12 [codex-maclaptop]

Downloaded free Mixamo Picking Up Object with source rig, provenance and hash; imported ReachReference and retargeted ReachCandidate to City male with existing Mixamo retargeter (3.433 seconds). Two editor poses inspected: standing waist-height reach, so reject it for low bench ignition and retain only as a potential item interaction. No runtime assignment or map change. Import and retarget exit0; editor render script completed with four images, but commandlet exit1 from occupied HttpListener port30010. No native playback, full animation or bench alignment acceptance. Next bench scene step needs a lower crouching reach, then contact placement and native review.


## Low reaching variant — 2026-09-12 [codex-maclaptop]

Mixamo Picking Up Object exposes Object Height. Downloaded a distinct variant at0 (other controls default), imported LowReachReference and retargeted LowReachCandidate to City male:4.0sec. Two editor poses show a bend and low reach. Bone trajectory sampled30fps gives lowest right hand at2.2sec: component XYZ(-16.29,19.09,54.21)cm. Existing bench seat45cm; account for body rotation, actor floor position and a held prop before contact, do not force lower body into floor. Native playback, actual bench placement, ignition timing, interruption and rare encounter remain pending; candidate unassigned. SourceFBX and settings/hash kept on external drive. Import/retarget/trajectory audit exit0. Editor render exported four frames but exit1 from occupied HttpListener30010, even with ModelContextProtocol disabled for that invocation. No project plugin settings changed.


## Native bench reach review — 2026-09-12 [codex-maclaptop]

Added City male BeginBenchReach playback with completion cleanup and horn, impact, damage, swimming/run-end cancellation paths. Native review flag -BattleBenchFireReview -BattleBenchReachReview places a paused City male at first bench local(0,55,90), facing bench(-Y), instead of spawning fire. Editor build exit0; native render exit0 with three images, started=1 and final reaching=0. Inspected bent and final standing poses. Sample2 right hand bench-local(23.535,25.654,59.406), still above/front of45cm seat; needs held prop/contact alignment and continuous motion review. Interruption paths coded but not yet exercised; no natural trigger/ignition or packaged assignment. Reservation/two-fire-cap/expiry native regression passes. Review evidence work/bench-fire-review/d360a3569d38462c980a75a26f010b6e. Desktop remains045.


## Bench reach interruption checks — 2026-09-12 [codex-maclaptop]

Added BeginBenchReach run-end guard and BeginSleeping active-reach guard. New BattleBenchReachAudit/test_native_bench_reach.py passes native duplicate-start rejection, sleep conflict, horn cancellation/restart, bike impact cancellation, stumble rejection, post-stumble restart, run-end rejection/cancellation, and damage/death rejection. Build exit0, native test exit0. Initial fixture timed out because setting run-ended across frames opens controller loss menu and pauses world. Final fixture explicitly ticks visitor with run-ended before menu can pause; this verifies cancellation-on-update, not full loss-menu lifecycle. No ignition or natural encounter yet; contact/prop and scene integration remain next.


## Delayed bench ignition — 2026-09-12 [codex-maclaptop]

BeginBenchIgnition now reserves an available bench, validates same-world index and front placement (localX within25cm, Y40–75, capsuleZ65–120, facing dot>.9), starts the reach, and at2.2sec hands reservation to existing capped fire API. Rechecks placement while reaching. Horn/impact/damage cancellation and actor EndPlay release pending reservation; after ignition fire owns reservation independently. One attempt can ignite once. Added BenchesIgnited counter and native audit. Editor builds exit0; delayed ignition audit passes no early fire, occupied-bench rejection, horn cancellation, restart, single ignition, ownership transfer and actor/fire destruction cleanup. Reach interruption regression passes. No held prop/contact validation, rendered combined scene, natural rarity director or packaged assignment; desktop045 unchanged. Next: render complete interaction, fit visible prop/contact and retreat before natural placement.


## Combined ignition review and retreat — 2026-09-12 [codex-maclaptop]

Native --ignite review now starts real BeginBenchIgnition, captures1.5/3/5.85sec and verifies pre-fire0/final1 plus reach completion. Initial scene showed character standing beside flames after clip; first navigation-projected retreat did not move him. Replaced with1.8sec CharacterMovement retreat toward bench-front260cm, using an80cm-ahead downward ground probe (150cm, normalZ>.8), collision via normal movement, and interruption/run-end guards. Native rerun exit0: bench-localY55→296.396cm, Z89.072→48.001 follows local slope, character leaves fixed frame while fire remains visible. Capture script now requires >100cm retreat as well as ignition/reach markers. Inspected fire and final frame, not continuous movement. Combined images work/bench-fire-review/16338e4358e04714a6e0a514f063283c; failed nav attempt d052e0181a554bcd9f5c8db7bcb5f2e9; initial scene41c169f78e2d449da58758d4ddf9c3b5. Final build and native ignition reservation/cancellation regression exit0. Prop contact, blocked-retreat cases, natural encounter and packaged acceptance pending. Note legacy BenchFireReview expired marker refers only to direct visual-review fire pointer, not delayed ignition fire; ignition acceptance uses separate ignition counters and positions.


## Handheld ignition prop candidate — 2026-09-12 [codex-maclaptop]

Measured hand/finger transforms at0/1.5/2.2/3.5sec (audit_bench_hand_grip.py). Added hand_r-attached noncolliding utility-lighter candidate:2x2x5cm handle using M_Safety atlocal(-4,0,0), metal cylinder.8cm diameter x7cm at(-4,0,-6). Visible only during BeginBenchIgnition, hidden by CancelBenchReach/completion. Native review second capture now2.2sec to expose hand/seat before full flames. Builds and native --ignite render pass; sample2 shows narrow stem under hand toward seat, grip partly occluded. Native ignition regression adds assertions that both components are visible during reach and hidden after horn cancellation; passes. Final bench-localY297.321 from55 proves retreat still moves. Prop is a candidate: closeup/continuous attachment, actual tip-to-seat contact and ignition spark not accepted; current ignition remains timing/actor-placement based. Evidence work/bench-fire-review/58b95a1a8ef041939463bb95b1bdc995. Natural encounters and packaging still pending, desktop045 unchanged.


## Closeup prop contact fit — 2026-09-12 [codex-maclaptop]

Added --ignite --close native camera plus ignition-time tip measurement (stem localZ-50 transformed through component and bench). First sample at2.202sec XYZ(22.506,25.665,49.148) missed seat top46.75 and frontY24.25. Adjusted grip X-4→-6.4cm, stem centerZ-6→-6.75 and length7→8.5cm. Final native sample at2.206sec XYZ(22.705,24.151,46.797): within front slat footprint, .047cm above its top. Closeup inspected, stem meets edge. Build/render exit0 and retreat verified; evidence e433be304f7e42db9c72801ba8c0fd6d. Capture report now parses contact coordinates and nearest slat-top error from actual geometry dimensions. One fixture/instant verified; continuous movement/all-site/low-frame-rate contact and natural encounter remain pending. Ignition still timing and actor-placement gated, not tip collision. Desktop045 unchanged.


## Rare bench encounter director — 2026-09-12 [codex-maclaptop]

BattleParkFurniture now ticks every.5sec with default ambient enable and20% per-run eligibility roll after45sec eligible gameplay. One attempt per run, resets using Mode.RunNumber. Protect tutorial/countdown/dead/swimming/run-end states. Find an available bench900–1700cm from player, verticalgap<=200, behind camera(dot<=-.1); spawn City male with DontSpawnIfColliding atbenchfront55cm and reserve bench. Wait up to60sec; player within650cm and verticalgap140 starts actual BeginBenchIgnition. Failed/expired/interrupted pending attempts release reservation; ordinary visitor can resume walking. One current furniture director is spawned by BattleBike.cpp. No additional direct fire spawn from director; existing2-fire API cap remains.

Build exit0; test_native_ambient_bench.py passes forcedchance0, protectedquietperiod, forcedchance1 behind-camera spawn, approach-driven ignition via normal ticks, and one-attempt cap. Fixture accelerates eligible time via Tick calls and updates camera cache after placement; first version failed because stale camera view was used. No statistical rarity/full route/player visual acceptance, no Motel-specific bench/rough wardrobe yet. New behavior is in source/editor build, not desktop045. Design requirement for occasional1–2 benches currently implemented as at most1 natural attempt per run; direct fire API max2. Tune only after actual ride review.


## Motel bench installed in main world — 2026-09-12 [codex-maclaptop]

Surveyed12 east-side sites using loaded collision:8–28cm support variation, no raw-grass site passed level bench criterion. Added160x260cm concrete seating pad atXY(-17470,11680), top=max9terrain probes+2cm, thickness covers variation+5; all sampled supportLandscape, maxdrop<=35cm. EarlierY11750 footprint hit mapped concrete path and was rejected before saving; shifted north. Bench faces east(yaw-90) away from building and parallel to10th, so retreat avoids motor lanes. TargetPoint tags AuthoredParkBench/MotelBench; runtime furniture imports authored anchors before generated benches. Placement/save script rechecks terrain and rejects duplicates.

Reviewed isolated PiedmontMotelBenchReview native --ignite --motel: build/render exit0, inspected bench beside building and burning frame; retreat241.494cm, ignition tip1.132cm above slat. Then saved pad/anchor into main PiedmontWorld via -BattleInstallMotelBench (exit0). Native main ambient encounter test passes after installation (forcedchance0/1, accelerated eligible clock). Main map now contains Motel bench; desktop045 does not. All-site/continuous/full-ride acceptance, rough wardrobe and next package still pending. Reviewmap retained locally; source/main map and reports tracked.


## Mac build046 installed — 2026-09-12 [codex-maclaptop]

Packaged new asphalt surfaces, bench reach/prop/fire/retreat, rare director and Motel pad/anchor; BuildCookRun exit0 in140.72sec. Packaged ambient forced0/1 lifecycle, sleeper approach/chase/return/cooldown and health/cardinal/death/checkpoints passed. Packaged native render confirmed Motel scene assets and241.397cm retreat; final fire frame inspected. Initial capture wrote no project-folder images under app sandbox; harness now uses bundle container Documents and copies images back. Strict ad-hoc codesign passes, desktop resolves to version0.46.0. Retained TheBattleOfATL-build045.app rollback. Installed-app health/checkpoints also pass. Fullgame/visual/performance/rarity acceptance remains incomplete; see PLAYTEST-046.md.


## Vehicle asset review — 2026-09-12 [codex-maclaptop]

Imported 55 bundled UE 5.8 vehicle assets (39 MB) on the external project drive, with source hashes. The sports-car skeletal mesh is a rig; visible body, glass and four wheels require separate static mesh components. Native Mac review now shows the assembled car on the graded 10th Street road. Editor build and offscreen native capture passed. Static body bounds are approximately 471 x 228 cm; wheel radius 39.27 cm. Do not size traffic using padded skeletal bounds.

This is an asset candidate, not installed moving traffic. Lane routes, slope and four-wheel contact, wheel rotation/steering, swept collision, braking, intersection behavior and packaged verification remain pending. Desktop build remains 046. Review command: python3 Scripts/render_native_car.py.


## Road-car movement prototype — 2026-09-12 [codex-maclaptop]

ABattleRoadCar now follows supplied world-space routes with acceleration, braking-distance sweeps, swept box movement, four static-ground wheel probes, slope alignment and wheel rotation. StartRoute rejects invalid routes and overlapping spawn locations. Endpoints stop rather than teleport or wrap. Native test on a 40 m 10th Street segment passed grounded travel, obstruction braking, two-second stationary hold, resumption after removal and endpoint stop (3997.563 cm). Editor build passed; native moving frame reviewed.

Not installed in the main map or desktop build 046. Current test route is close to the centre marking; generate actual lane paths before traffic placement. Still needs curved-lane steering review, four-wheel contact measurement, emergency cut-in / swept-hit route progress handling, bike/rider collision consequences, signal crossings, safe population spawn/despawn and packaged validation. Native moving capture has motion blur; no continuous-animation or frame-rate acceptance.


## 10th Street lanes and slope correction — 2026-09-12 [codex-maclaptop]

Generated two opposing through-lane candidates with 408 points each, following the installed lane paint. Eastbound stays at 750 cm offset; westbound uses 450 cm in the four-lane portion and tapers to 150 cm in three lanes. Shared turn space remains clear. Routes stop before Monroe's conflict zone pending crossing controls. Source data retains indices where the vehicle footprint intersects marked cycle crossings. No traffic population has been installed.

Footprint analysis found narrow gaps at separately buffered OSM road joins. Added a 342-triangle, 13,570 cm² repair excluding the cycle track and separator. All 7,344 native footprint probes passed: 7,331 on the road, 11 on marked crossing pavement and two on the repair. Maximum source/native height error <0.001 cm. Repair is now installed in the main map with an explicit seam collision probe; desktop build remains 046.

The first full native driving test failed: both cars stopped against the road on hills. MakeFromXZ preserved the horizontal X direction, preventing body pitch. Changed to MakeFromZX to preserve the road normal. Both complete lane traversals then passed (combined 81,395.797 cm), including support and endpoint stops. The separate braking/hold/resume fixture passed again after this fix. Retained the failing report and diagnostic logs to explain the regression. Continuous visual motion, emergency cut-ins, signals/crossing yielding, safe population spawning and packaged acceptance remain pending.

Reproduction: Tools/road-geometry-venv (Python 3.12, requirements in Tools/road-geometry-requirements.txt) runs build_tenth_road_seams.py; ordinary Python runs build_tenth_car_lanes.py. Unreal commandlet runs validate_tenth_car_lanes.py to rebuild the isolated PiedmontCarLaneReview map; then python3 Scripts/test_native_road_lanes.py. Use absolute project and script paths with UnrealEditor-Cmd. Review map is generated, not committed.


## Crossing entry control — 2026-09-12 [codex-maclaptop]

Added ABattleRoadCrossing with an authored occupancy volume and externally controlled vehicle-green state. ABattleRoadCar accepts crossing references and stop distances along its route. Red or occupied crossings limit forward travel to the stop line. A car that has committed to entry can clear the junction after red; it does not stop in the middle solely because the light changed. Route initialization rejects missing/foreign-world crossing references and out-of-range stop lines; destroyed gates fail closed before commitment.

Native Mac fixture passed a red stop, green-with-occupant hold (occupant was laterally outside the car's ordinary braking sweep), release after occupant removal, red transition after entry and endpoint stop (3997.573 cm). Editor build passed. Controls remain unplaced: no traffic signal timing, visible lamps, multi-car reservation arbitration, population or packaged acceptance yet. The mapped Piedmont Avenue bike crossing intersects westbound candidate footprint indices 312–321; eastbound has no footprint overlap in the retained through-lane section. Monroe traversal still awaits authored crossing routes.


## Crossing reservations — 2026-09-12 [codex-maclaptop]

Crossings now grant an exclusive reservation when a car commits at its stop line. Competing entry requests are denied even before the first car's body reaches the occupancy volume. Ownership survives a red transition; it releases after the owner's collision bounds have entered and then cleared the volume. Vehicle destruction and route restart release reservations, and wrong-owner releases are ignored.

Editor build passed. Native API fixture with two vehicle-sized blocking actors passed ownership, contention, idempotence, red commitment, tail clearance and destroyed-owner cleanup. The moving-car crossing fixture also passed again (3997.549 cm), covering the actual car's reservation call alongside red/occupied-green stopping and subsequent exit. Full moving queues, fair scheduling, visible signals, world placement and traffic spawning remain unverified/unimplemented; no desktop update.


## Visible traffic signal candidate — 2026-09-12 [codex-maclaptop]

Added ABattleTrafficSignal: a metal pole and foot, dark housing/backplate, three lamp rims and visors, with dynamic red/green lenses bound to ABattleRoadCrossing::bVehicleGreen. Missing crossing defaults to red. Amber remains inactive pending the signal-phase controller. Pole blocks collision and actor carries RideBarrier; approach collision has not yet been tested. Uses authored housing/pole/lens materials in /Game/BattleForTheA/Traffic and bundled engine primitive meshes.

Editor build and native red/green screenshot capture passed. The first review clipped the head and caught shaders compiling; the review now frames the complete signal and finishes editor shader compilation before capturing. Lamp intensity was reduced after inspection to preserve red/green color rather than orange/white highlights. Both revised frames inspected and accepted for isolated daylight appearance. The asset is not placed in the main world or desktop 046; street siting, approach visibility, amber/timing, live queues, population and packaged review remain pending.


## Timed signals and amber decisions — 2026-09-12 [codex-maclaptop]

Crossings can now run an optional repeating gameplay cycle: 18 seconds green, 3 amber, 9 red by default, with durations clamped to at least one second. These are tunable game timings, not measured Atlanta signal timings. Auto-cycle defaults off so existing manually controlled crossings remain unchanged. Signal lamps display the corresponding three states.

On first observing amber, a car with enough stopping distance commits to braking for the remainder of that amber phase. A car already too close for the configured braking rate may reserve entry and clear; occupancy and exclusive ownership still apply. Stop decisions reset after amber ends. This prevents a distant car from changing its mind as it approaches the line while braking.

Native test passed the repeating cycle, near-line amber clearance at 650 cm/s without braking, distant amber stop and two-second hold, and green resumption to route end. Three-state native screenshots inspected: red top, amber middle and green bottom, other lamps dark. Editor build passed. No signals or traffic population installed in the main map yet; real approach visibility, moving queues, crossing placement and packaging remain pending.


## Mapped signal crossing installed — 2026-09-12 [codex-maclaptop]

Surveyed 32 pole sites with five native terrain/path probes each. Rejected sites intersecting apartment roofs, cycle paving or mixed surface edges. Selected signal (-20800,12050,272.150), yaw 180, fully on the grass strip; 3.25 cm support variation is absorbed by the foot's placement. The mapped cycle crossing defines a volume centered (-21218.163,12580.8275,402.511), with a 90 cm horizontal margin. Westbound lane binds at route distance 31124.293 cm, 100 cm ahead of the first car-footprint crossing sample.

In isolated PiedmontSignalCrossingReview, both complete routes passed native driving (81395.703 cm combined), including an actual stopped signal wait and resumed completion. A 90-degree, 140 cm-high camera at westbound route point 305 showed a readable red lamp at the right of the approach; frame inspected. This is one approach sample, not complete rider-camera/night/stop-line acceptance.

Installed the crossing and linked signal into the main map, tagged TenthPiedmontBikeCrossing and TenthPiedmontBikeSignal, with the 18/3/9 gameplay cycle. Test cars remain only in the generated review map. Natural traffic spawning, multi-car queues, remaining junctions, full visibility review and packaged integration remain pending. Desktop stays 046. The review-builder removes these owned main-world actors transiently before rebuilding, avoiding duplicate signals on repeat runs.


## Live road traffic director installed — 2026-09-12 [codex-maclaptop]

Added ABattleRoadTrafficDirector and authored lane data: six cars maximum, three per lane, eight seconds between spawn attempts. Spawn points must be at least 35 m from the camera and 30 m from the pawn, and either behind the camera or fully hidden by static cover across a conservative car envelope. Cars are hidden and non-colliding until StartRoute verifies support and an unoccupied spawn. Completed cars remain while observed/nearby; removal occurs only when unobserved and distant. Director teardown removes its cars and their crossing reservations through normal car EndPlay.

Native API fixture passed camera proximity, visible-spawn rejection, occupied-spawn rejection, spacing, cap, visible completed-car retention, hidden removal and teardown. A separate automatic run used both actual 10th Street lanes and the installed signal: nine cars spawned, four completed and recycled, peak six, with an observed stopped signal wait. The camera was deliberately kept facing away from spawn/removal areas; ordinary rider interaction, rendered traffic motion, performance and packaging are not proven by that test.

Installed the director in the main world under tag TenthRoadTraffic, binding the westbound lane to TenthPiedmontBikeCrossing. The test map remains generated and uncommitted. Main-world live play, collision consequences, remaining road junctions and packaged validation still pending; desktop remains 046. Routes still terminate before Monroe crossing traversal; the director preserves visible finished cars rather than popping them out.


## Main-world traffic appearance and bike impact — 2026-09-12 [codex-maclaptop]

Captured and inspected three native main-world frames of naturally spawned traffic: six cars active, tracked route progress increased from 19953.924 to 22456.643 cm at 650 cm/s. Cars are assembled and road-aligned, with opposing traffic visible. This accepts sampled placement only; identical bright orange paint, continuous motion, rider-camera visibility and performance remain unfinished.

Fixed road cars missing the RideVehicle tag, which had caused the bike to treat them as ordinary walls. Review builders now preserve constructor tags. Native keyboard-driven bike impact against a stationary road car passed with exactly one Traffic impact wipeout. This does not validate car-initiated impacts, glances, injury or realistic crash animation; the existing recovery pose remains rudimentary. Editor build succeeded. Desktop remains 046; no packaging performed.

Keep the requested Monroe, pre-Krog and Krog tunnel road crossings, occasional fallen scooter rider with helpers, and 10th Street Peachtree race dressing with active cars in remaining scope.


## Moving car collision — 2026-09-12 [codex-maclaptop]

Road-car swept impacts now compute closing speed against the bike and trigger the existing Traffic impact recovery above 500 cm/s, only for a mounted living rider. This covers late cut-ins where braking cannot prevent contact. Normal forward braking remains in place. Partial swept travel now advances route distance and wheel rotation by the completed fraction, avoiding a stale route position after emergency contact.

Editor build passed. Native late cut-in fixture at 650 cm/s caused exactly one wipeout; normal road-car obstruction test also passed braking, stationary hold, resume and endpoint at 3997.578 cm. The cut-in is a deliberately placed fixture, not a rider-controlled crossing or visual crash-quality test. Physical rider ejection, health damage, on-foot collision response, glance tuning and packaging remain unfinished. Desktop remains 046.


## Mac build 047 installed — 2026-09-12 [codex-maclaptop]

BuildCookRun succeeded in 152 seconds. Packaged main-world traffic captured six live cars in three frames; frames 1 and 3 inspected for assembled bodies/wheels and opposing-lane placement. Packaged bike-to-car and late-cut-in moving-car tests passed with one recovery each. Health/checkpoint test passed two death resets, phone recollection and ordered checkpoints.

Installed branded version 0.47.0 with strict ad-hoc signature verification at the existing desktop link. Previous 046 is retained in Builds/BattleForTheA/Mac/TheBattleOfATL-build046.app. New render/collision scripts accept explicit app and report paths, preserving historical reports. This is narrow packaged integration evidence, not full game/visual/performance acceptance. See PLAYTEST-047.md for remaining limitations.


## Monroe traffic lane preparation — 2026-09-12 [codex-maclaptop]

Prepared two opposing candidate lanes on the existing 4994.865 cm Monroe frontage, inset 400 cm from endpoints and offset 150 cm either side of its mapped centreline. Short frontage is insufficient for natural traffic lifecycle without further extension/visibility review. Lane generation uses installed road/cycle/seam triangles and preserves the main world.

Native collision audit checked 1512 footprint samples: 1481 hit road and 30 hit cycle crossing pavement; one fell through a road seam onto Landscape at (12371.346,11688.619,-341.566). Source mesh coverage independently misses that point. Candidate is explicitly failed; no cars or controls were installed. Repair the junction seam, repeat native coverage, then test driven traversal and author signal/conflict areas before production traffic.

OSM source retains multiple crossings, including the angled cycle crossing way 1396654821 and cycletrack connection way 1396654823, plus older signalized footway crossing segments 231270041/1387296277. Do not collapse these into one line or assume every source footway remains the primary BeltLine alignment. Official project description confirms realigned trail, raised crossing and improved signals: https://beltline.org/blog/construction-to-begin-on-10th-and-monroe-intersection-project/ . Current topology still requires full native/visual comparison.

Evidence: SourceAssets/Terrain/MonroeTraffic/car-lanes.json, car-lane-probes.json and Tests/Results/2026-09-12-monroe-car-support.json. Desktop stays 047.


## Monroe pavement repair installed — 2026-09-12 [codex-maclaptop]

Generated centred road-join repairs from adjacent Monroe OSM segments at their shared minimum lane width. Total 4473.446 cm² (0.447 m²), 182 terrain-following triangles; zero protected cycle-track/separator overlap. Imported SM_Monroe_RoadSeams with existing world asphalt and complex collision.

All 1512 native footprint samples now pass (1481 road, 30 crossing pavement, 1 repair). Both candidate lanes then completed native vehicle traversal and endpoint stops, 8294.260 cm combined. The generic lane audit now accepts an explicit Monroe mode with the appropriate short-route threshold while preserving 10th Street checks. No traffic controls or production cars were added.

Installed only the repair in PiedmontWorld, tag MonroeRoadSeams. Re-probed the discovered gap at (12371.346,11688.619): it now hits repair Z=-327.558 instead of terrain. Desktop remains 047; packaging, crossing signal logic, longer approach routes and visual rider-level review remain pending. Scripts regenerate isolated review map PiedmontMonroeTrafficReview.


## Monroe crossing control candidate — 2026-09-12 [codex-maclaptop]

Retained four individual OSM crossing ways (1396654821, 1396654823, 231270041, 1387296277) in MonroeTraffic/crossings.json. A conservative shared vehicle-control envelope spans XY (11747.156,10007.506) to (12673.163,11481.898), with a 100 cm margin around those alignments. This groups vehicle reservations without changing path geometry; it is not a claim about real-world traffic timing.

Created isolated PiedmontMonroeCrossingReview, binding the two cars to a shared ABattleRoadCrossing with accelerated 2/1/6 green/amber/red timing. Body-envelope entry determines stop distance with a further 100 cm buffer: northbound 1950.460 cm; southbound only 98.482 cm from its candidate start. The very short southern lead-in confirms production approaches need extension before live spawning.

Native test passed both route completions (8294.221 cm combined), wheel support, endpoint stops and at least one actual stopped signal wait. Underlying crossing occupancy/reservation behavior was previously unit-fixture tested; this run did not deliberately place a pedestrian inside this specific Monroe envelope. Visible signals, player crossing interactions, approach extensions, production traffic timing and packaging remain pending. No crossing controller installed in main world yet; desktop remains 047.


## Extended Monroe approaches candidate — 2026-09-12 [codex-maclaptop]

Extended retained Monroe topology to 15591.150 cm, versus the old 4994.865 cm frontage. Generated 2756 terrain-following approach triangles while subtracting installed road/cycle/separator coverage. Native checks initially found 96 samples on obsolete sidewalk end caps; generated a trimmed sidewalk candidate (8714 triangles) retaining the parallel sidewalks, and replaced it only in the isolated review map. All 5328 native footprint checks then passed.

Full two-car driving test FAILED: southbound completed, but northbound stopped at route distance 1749.77 cm around (11203.452,16121.470,-326.555). The reported collision actor StaticMeshActor_132 was resolved in the editor to the new Monroe extended approaches mesh itself. This points to road/body clearance or local road profile, not an unrelated prop; investigate contact geometry before changing the car or terrain. Do not install this candidate. Source/footprint acceptance does not prove body clearance. Desktop/main-world approaches remain unchanged at 047.

New scripts prepare_monroe_approaches.py, build_monroe_approaches.py, validate_monroe_approaches.py, test_native_monroe_approaches.py and inspect_monroe_blocker.py reproduce the candidate and failure. build_monroe_car_lanes.py --extended preserves the original shorter lane files. Results: monroe-extended-support, native-monroe-extended-lanes and monroe-blocker dated 2026-09-12.


## Monroe body-contact diagnosis — 2026-09-12 [codex-maclaptop]

Added opt-in BattleRoadContactAudit logging. First box contact at route distance 1748.804 cm is the rear lower corner, local (-237.830,111.075,-53.462), against road normal (-.014,.011,1), pitch 4.085 degrees; no starting penetration. This is road/underbody contact, not an unrelated prop. Centreline grades sampled over 1100–2300 cm are only 0.9–2.5%; compare all four wheel contacts and local lateral road profile before assuming a generally excessive road grade.

Bundled sports body had zero collision shapes. Created separate SM_RoadCarHull by convex decomposition (3 hulls), preserving original. Added opt-in BattleBodyHull component sweep candidate with body collision; ordinary gameplay still uses the existing box. Candidate editor build succeeded but full extended Monroe route test FAILED at the same rise (northbound ~1747.90 cm; southbound finished). Do not promote this experimental collision or claim a road fix. It does not yet have bike-impact, crossing, or packaged regression acceptance.

The asset-generation full editor completed its save and shutdown handler but remained alive; its owned process was terminated after confirming the generated report/asset. No user editor was touched. Next work: inspect native wheel-support positions versus local road mesh/terrain and repair the actual clearance transition. Desktop remains 047.


## Extended Monroe snag resolved — 2026-09-12 [codex-maclaptop]

Native wheel logs showed two contacts on Landscape, 14 cm below pavement, while the others hit road. Visibility and object-type traces agreed. Direct source OBJ height queries at (11084.373,16233.117) and (11259.542,16274.541) returned no triangle: the generator's separately buffered, flat-capped OSM ways left wedges at a bend. Existing coverage checks measured only the already-gapped polygon, and the earlier 9-point vehicle envelope sampling missed the actual wheel locations.

Fixed the road generator by adding connected three-point buffers across adjacent shared nodes at their minimum lane width, before subtracting installed crossing surfaces. Expanded source/native validation to include the four real wheel offsets as well as the body envelope. All 7696 checks now pass. Both extended Monroe routes completed with the DEFAULT box collider, combined 29494.305 cm, including endpoint stops. No vehicle clearance or terrain flattening change was needed. The experimental body-hull option remains unnecessary/unaccepted and is not default.

Rebuilt only isolated PiedmontMonroeExtendedReview. Visible road/sidewalk review, extended crossing controls, signals and live traffic installation remain pending; desktop stays 047. The earlier hypothesis of excessive road curvature is superseded by this verified source-geometry gap diagnosis.


## User-confirmed Irwin/Lake crossing — 2026-09-12 [codex-maclaptop]

Elliott supplied a map screenshot identifying the previously unspecified dangerous crossing before DeKalb Avenue: the Atlanta BeltLine Eastside Trail crossing Irwin Street NE / Lake Avenue NE. Screenshot landmarks: Lingering Shade Social Club northwest, Icebox Cool Stuff northeast, BRASH Coffee southwest, Rowan Krog District southeast, Krog Street NE to the east. Treat this as a separate required road-traffic hazard from Monroe and the Krog Street/DeKalb Avenue tunnel approach. Preserve the trail continuation across the road and crosswalk placement when authoring geometry. Include crossing cars in gameplay; exact lane geometry, signal/stop behavior and native placement still need verification. User screenshot identifies the intended location; no real-world crash-rate claim is inferred.


## Extended Monroe crossing and signal review — 2026-09-12 [codex-maclaptop]

Built PiedmontMonroeExtendedCrossingReview using the repaired extended approaches. Shared crossing stop distances are now northbound 6151.613 cm and southbound 6447.545 cm, giving useful lead-in distance instead of the earlier 98 cm southern start. Native test passed both full routes with a stopped signal wait, support and endpoint stops (29494.279 cm combined); accelerated 2/1/6 cycle remains test-only.

Surveyed five terrain support points per pole candidate, rejecting non-terrain sites and >8 cm variation. North signal (12898.872,11787.262,-316.091), yaw -76.800; south (11635.776,9677.731,-359.996), yaw 104.198. Initial approach images showed backs of lamps: corrected orientation to road heading, because this signal mesh faces local -X. Re-rendered and inspected both approaches; red lamps are visible on the right at 90-degree FOV, 140 cm eye height and 650 cm before stopping position. Narrow fixed-view acceptance only.

Wide review shows missing Monroe lane markings/crosswalk paint and unfinished surroundings; do not claim overall landscape quality. Signals and crossing remain isolated, main unchanged. Next: markings, deliberate crossing-occupancy/rider interaction, live traffic and installation. Desktop remains 047.


## Monroe road paint candidate — 2026-09-12 [codex-maclaptop]

Generated non-colliding white/yellow paint draped 1 cm over pavement triangles: road centre and edge lines, four-lane dashed dividers, approach stop bars at car-centre stop +270 cm, and stripes along the retained mapped crossing alignments. Crossing segments are merged before stripe spacing; the initial independent-segment version overlapped into a solid white band and was rejected in screenshots. Rebuilt version preserves stripe gaps.

Imported only into PiedmontMonroeExtendedCrossingReview with tag MonroePaintReview. Latest white mesh 4062 triangles, yellow 1818. Wide native image inspected; lines/stop bars/stripes visible, but overall crossing-layout fidelity, three-lane turn-lane detailing and rider readability are still unfinished. No collision or gameplay changes from the paint. Main and desktop remain unchanged (047).

render_monroe_crossing.py now accepts --report to preserve historical signal evidence. Paint results are separate from the previously accepted signal-facing report.


## Monroe traffic installed in main world — 2026-09-12 [codex-maclaptop]

Native occupancy fixture placed an actual ACharacter capsule in the crossing on green. Both opposing cars held before their stop distances together for 2.001 seconds, then completed after fixture removal. This tests collision occupancy, not animated pedestrians or a player-controlled crossing.

Automatic director test with 18/3/9 gameplay timing passed: 8 spawned, 2 recycled, peak 6, stopped signal queue observed. Isolated test removed 10th traffic and used a controlled offscreen observer. Installed Monroe approaches, trimmed obsolete sidewalk caps, non-colliding paint, two signals, shared crossing control and two-lane director into PiedmontWorld. All 7696 support probes passed again in main world; existing 10th director retained. Main tags: MonroeInstalledSurface, MonroeCrossing, MonroeSignal, MonroeRoadTraffic. Each director caps at six; combined performance is unverified.

Road geometry/traffic integration is now playable in the project, but three-lane detailing, overall scenery, main-world rider interaction and combined traffic performance remain unfinished. Desktop remains 047 until packaging. Review rebuild removes owned installed Monroe actors transiently, preserving main map.


## Irwin/Lake source placement and surface survey — 2026-09-12 [codex-maclaptop]

Matched retained OSM crossing way742982445/node69331892 to the screenshot at longitude -84.3648923, latitude33.7575617. Prepared eight nearby road segments in SourceAssets/Terrain/IrwinTraffic/network.json using the active tenth-graded heightmap. Native 231-point survey locates the installed Eastside asphalt centre at world (26082.2603,100847.4302,794.9536) cm. The preliminary terrain-following road centre is 11 cm higher: blend the eventual road to the installed trail before enabling traffic, rather than introducing a step. Scripts/prepare_irwin_crossing.py and survey_irwin_crossing.py reproduce placement and survey. This is preparation only; no Irwin road, controls or cars installed and desktop build047 is unchanged.


## Irwin/Lake road candidate — 2026-09-12 [codex-maclaptop]

Built 600 cm two-way road candidate across eight retained nearby segments, clipped to 6500 cm site radius. Existing Eastside/Krog asphalt and concrete footprints are subtracted. Road height blends to original trail triangles over 300 cm; source frame agrees with all 35 native trail reference samples within 0.000059 cm. Source mesh has 7875 triangles, negligible missing area and no meaningful overlap with preserved paths. Native import and all 7875 triangle-centre support checks pass; original trail probes retain their actor and height. Isolated map: /Game/PiedmontRide/Maps/PiedmontIrwinRoadReview. Main map and desktop047 unchanged. Scripts/build_irwin_roads.py requires Tools/road-geometry-venv (terrain venv lacks constrained triangulation); Scripts/validate_irwin_roads.py uses absolute project/script paths in editor commandlet.

Still pending: actual lane/wheel-envelope traversal, traffic control consistent with mapped crossing, paint, curb/scenery, rider viewpoint and packaged acceptance. Import reports degenerate tangent/nearly-zero normal warnings on small clipped triangles: inspect and repair shading before accepting visuals. Do not infer playable crossing from support probes alone.


## Irwin/Lake opposing traffic and occupancy — 2026-09-12 [codex-maclaptop]

Generated opposing east/west routes along connected Irwin/Lake ways (13243.463 cm source centreline), 150 cm lateral offsets, 400 cm endpoint insets, 500 cm/s cruise. All 6474 native body/wheel support probes pass on road and retained trail surfaces. Native drive completes both lanes with support and endpoint stops, 24794.055 cm combined. Evidence: 2026-09-12-irwin-lane-support and native-irwin-lanes.

Source crossing tags indicate continuous/unmarked cycle crossing with traffic-calming table. Review control therefore remains eligible when clear and yields when occupied; no invented signal cycle/poles. Conservative shared crossing reservation bounds X25704.161..26458.188, Y100302.902..101355.387. Stop distances5399.260/5496.782 cm. Native character-capsule fixture holds both cars for2.002 seconds, then cars complete after removal. This is collision-occupancy evidence, not player walking or animation acceptance. Reuses existing Monroe audit tags only in isolated review maps. Scripts/create_irwin_lane_review.py and create_irwin_crossing_review.py reproduce the maps. Main and desktop047 unchanged. Next: natural traffic population, visual surface repair/review, player crossing and main integration.


## Road potholes requested — 2026-09-12 [codex-maclaptop]

Elliott explicitly requests potholes to reflect Atlanta road conditions. Add authored, visible and avoidable road depressions on street approaches, distinct from accidental mesh cracks. Proposed gameplay: shallow potholes jolt/scrub speed; deeper potholes can cause a high-speed wipeout. Preserve a navigable line through each road and intersection, including cycle access. Tune against both arcade and real-physics modes, verify actual wheel traversal and rider-visible warning/readability, and avoid random invisible penalties. Hazard placement, modelling, response and verification remain pending.

2026-09-12 [codex-maclaptop] — Elliott clarified potholes should be sparse, not everywhere. Use a few scattered trouble spots with long stretches of clean roadway, not a dense obstacle course.


## Irwin population and visual diagnosis — 2026-09-12 [codex-maclaptop]

Native recurring traffic passed8 spawned/2 recycled/peak6/observed stopped crossing wait. Generic population audit has an Irwin offscreen camera and allows occupancy proof to remain in its separate fixture rather than requiring a forced signal phase; other corridor requirements unchanged. Editor build passed.

Rendered road rejected: visible thin cracks and patchy shading. Added explicit normals and Nanite position precision8/full fallback matching existing trail; 7875 support checks still pass, but cracks remain. Hide-landscape render shows black gaps in same places, pointing to road mesh discontinuities rather than only terrain intersection. Likely next diagnosis: build_irwin_roads.py adaptive per-triangle subdivision introduces T-junctions with nonlinear height on subdivided edge, while unsplit neighbours interpolate straight edges. Make tessellation conforming and recheck rendering/support before installation; source 2D coverage alone cannot prove 3D watertightness.

Main is unchanged and desktop remains047. install_irwin_traffic.py is prepared but guarded by road_geometry_accepted_for_integration, currently absent/false; do not bypass. Review images before precision68e7c4c59ac546cf8196088c64056953, afterf52802db4bd54268bbfb85a59fdf80a9, landscape-hidden6108b02cf9b04322a61e15b22c6aab3a under work/irwin-crossing-review. Landmarks, paint and scenery remain unbuilt here.


## Irwin mesh repaired and traffic integrated — 2026-09-12 [codex-maclaptop]

Confirmed nonconforming tessellation: 1200 subdivided shared edges had up to1.7364 cm height disagreement. Generator now inserts all shared edge vertices and retriangulates conforming polygons; explicit edge-incidence audit finds zero open internal edges. Rebuilt9165 triangles; all9165 native surface samples and preserved trail checks pass. Both rendered approach views no longer show the thin road cracks (work/irwin-crossing-review/3c49385304544adea9352905e5b7d1de). Geometry accepted for ongoing integration, overall visual acceptance remains false: streetscape/paint absent.

Reran opposing car drive on repaired asset: passed24794.064 cm combined. Installed Irwin road, occupied-area yield control and two-lane recurring traffic into PiedmontWorld. Main support6474 probes pass. Retained both10th andMonroe directors; each of three corridors may have up to6 cars, combined performance not accepted. No traffic signals invented at Irwin. Desktop047 unchanged, pending packaging after further work. Potholes still sparse planned hazards, not implemented. Next includes visible street treatment/landmarks, rare potholes, Krog/DeKalb crossing and fallen scooter event, main rider playthrough, combined performance and all remaining full-game requirements.


## Irwin/Lake lane markings installed — 2026-09-12 [codex-maclaptop]

Added double yellow centre and white edge paint to Irwin/Lake and adjoining Krog roadway, draped1 cm above exact repaired triangles. Cleared trail crossing and Krog junction; retained source-unmarked crossing without inventing zebra paint/signals. Both approach render views inspected: lane lines readable and crossing unobstructed. Installed two NoCollision paint actors into main world. Reproducible scripts build_irwin_markings.py, import_irwin_markings.py, install_irwin_markings.py; result irwin-paint-install. Images work/irwin-crossing-review/38516b7bc5e9448492a56956da0b9977. Overall art acceptance remains false: terrain streetscape, buildings, signs and sparse potholes unfinished; desktop047 unchanged.


## Irwin building exterior study — 2026-09-12 [codex-maclaptop]

Fetched149 nearby building elements into References/irwin-lake-buildings-osm.json (OSM/ODbL); selected seven immediate footprint ways211061295/296/297/298/637,1139043893,1396654871. Source named Gravel Building and Stove Works; estimated1–3 storeys at340 game cm/storey explicitly not surveyed. Trimmed footprints against road330 cm half-width to preserve expanded gameplay roads. Added generic brick shells,275 window bays, trim and roofs using existing project materials.

Separate PiedmontIrwinBuildingsReview created;6474 native road support probes pass with parked fixture car collision temporarily disabled and restored. Render approach1/wide3 inspected in work/irwin-crossing-review/1cf6d7ddfa1b44449569e4d7f8c756c4: enclosure improved but repeated facades not accepted as landmark likeness. Needs sidewalk/patio/door treatment, facade variation, accurate heights/frontages, extended trail clearance and vehicle passage before main integration. Main and desktop047 unchanged by this study. Scripts prepare_irwin_buildings.py uses terrain-venv(pyproj); build_irwin_buildings.py uses road-geometry-venv(constrained triangulation). Official Krog District directory checked for context: https://www.thekrogdistrict.com/directory ; no imagery copied into assets.


## Irwin building path clearance — 2026-09-12 [codex-maclaptop]

Original Stove Works footprint overlapped widened installed trail by441.587 cm². Added100 cm trail setback and increased road centre setback to480 cm, reserving180 cm between road edge and footprint for future sidewalk construction. Rebuilt exterior study;6474 road and1447 native nearby trail probes pass. Reproduced source geometry with identical OBJ hashes. Retained source trail footprint extraction/probes in survey_irwin_building_paths.py.

Approach render d29fadf0613844e1bf721cbeaf11acb1 inspected: roadside space clear, still grass (not completed sidewalk). Some clipped frontages now lose window bays because curved/densified clipping boundaries yield many short edges. Simplify frontage segments conservatively or distribute bays along merged facade runs before acceptance. Still no entrance/patio/photo-faithful architecture, main installation or desktop update.


## Irwin frontage and entrance correction — 2026-09-12 [codex-maclaptop]

Simplified clipped footprints within5 cm with protected-area inset assertion. This removes short facade segments that prevented bay placement;311 window bays now generated. Added seven closed exterior double-door panels, handles and small canopies, choosing a substantial facade toward the crossing. Entrance coordinates retained in manifest. Generic treatment remains a study, no enterable interiors or surveyed elevation claim. Native6474 road and1447 trail support checks pass. Render approach1 at work/irwin-crossing-review/4f4529eb080849caae63e628b995512e inspected: missing frontage bays restored and doors visible. Remaining grass strips, threshold elevations/approaches, storefront variation and landmark likeness prevent full visual acceptance. Main/desktop047 unchanged.


## Irwin roadside pavement candidate — 2026-09-12 [codex-maclaptop]

Generated flush concrete strips in the reserved road480 cm envelope, subtracting repaired road, existing trail and building footprints. Shared-edge conforming triangulation retained. Report explicitly includes one0.046692 cm open micro-edge from polygon clipping (under0.1 cm threshold), missing area0.002688 cm²: do not call mesh perfectly watertight. Imported12591 triangles into separate PiedmontIrwinSidewalkReview; all12591 centroid support probes and original crossing trail samples pass. Render approach1 inspected at work/irwin-crossing-review/6cf15ddfa35d4fb588884abe2680a7a0: concrete visible beside roads, no large cracks. Walking edge transitions, door landings, undulation treatment, extended clearances and full streetscape remain pending. Main/desktop047 unchanged. Scripts build_irwin_sidewalks.py, validate_irwin_sidewalks.py, render_irwin_sidewalks.py reproduce candidate.


## Irwin entrance thresholds and landings — 2026-09-12 [codex-maclaptop]

Native original entrance survey found door40 cm approach offsets from -8.1 to126.7 cm relative to surface. Exterior closed door thresholds now use that retained measured approach height+4 cm, independently of estimated building floor heights; no interior floor consistency claim. Added separate colliding Landing mesh (seven small slabs), adjusted door/canopy heights. Threshold validation passes all seven: door height matches landing at sampled centres within0.25 cm. Broad connecting paths and accessibility/walking traversal unverified.

Pavement support now12575 pavement hits+16 expected landing overlays. Floor traces use±100 cm to avoid hitting overhead canopies, explicitly not a headroom proof. Road6474/trail1447 checks still pass. Approach1 render53f0d187750c4ee3a8fcefbb29737770 inspected, doors sit closer to pavement. Main/desktop047 unchanged. Original irwin-entrance-survey is stable generation input; validate_irwin_entrances writes separate entrance-landings result to prevent iterative threshold drift. Remaining canopy headroom, complete walk approaches, facade likeness, streetscape and game scope persist.


## Irwin approach capsule clearance — 2026-09-12 [codex-maclaptop]

Added static walking-sized capsule sweeps (radius32/halfheight90, centre floor+95) every20 cm from40 to500 cm outward along all seven entrance approaches, with overhead rays at three lateral points to230 cm. Initial test found two landing drop snags (ways211061296 and211061637) at80/100 cm. Retained failure evidence as irwin-approach-before-aprons and added measured sloped aprons to Landing mesh. Short140 cm version retained abrupt descent and failed; final260 cm apron stays level initially and distributes descent through300 cm from door. Reimport road/trail support checks pass; all seven approach sweeps now pass.

This is static geometry clearance only, not actual CharacterMovement, accessibility, side-edge comfort, complete foot traffic simulation or visual acceptance. Main/desktop047 unchanged. Final aprons need rendered review and actual walking before integrating the exterior study. Broader game requirements remain unfinished.


## Irwin walking and first streetscape integration — 2026-09-12 [codex-maclaptop]

Added BattleEntranceWalkAudit on actual ABattleRider after bike dismount: normal CharacterMovement with direct world movement input, seven measured500-to40 cm approaches and return. All14 legs pass, grounded at endpoints and no sustained fall/stall. Spawns frozen/nearby pedestrians removed for isolated traversal; no keyboard binding, animation, accessibility or busy-crowd acceptance. Json module dependency added for retained route input; editor build passes.

Latest wide render e851ac66d8c549a091bbebdec0e68a21 reviewed. Integrated five exterior mesh groups plus roadside pavement into main PiedmontWorld (six actors), preserving original roads/trails: all7921 main car/trail samples pass. First architectural pass remains explicitly rough: generic facades, estimated heights, remaining grass approaches, missing business-specific frontage/props and broader scenery quality. Desktop047 unchanged. Main integration does not mean whole game or streetscape is finished; performance, packaged acceptance and original full-game requirements remain.


## Sparse pothole contact implementation — 2026-09-12 [codex-maclaptop]

Added ABattlePothole contact controller, not yet placed. Sweeps approximate front-wheel trajectory (bike forward80 cm), radius65 cm, vertical guard180 cm; ignores airborne, parked, recovery and speeds below150 cm/s. Shallow/slow deep contact keeps72% speed and gives existing RideImpact feedback. Deep contact at1000 cm/s invokes existing2-second wipeout recovery; no separate time/health deduction. Latches until leaving radius+100 cm; large teleports excluded. Settings are initial gameplay values, not accepted balance.

Editor build passes; native contact fixture passes fast segment crossing, near miss, airborne/parked exclusions, duplicate prevention and exit/re-entry deep recovery. Test does not prove actual road riding, visible depression, wheel geometry or real/arcade-mode feel. No pothole actor is placed in main world, no purchased assets, desktop047 unchanged. Next requires visible authored sparse depressions, actual traversal/render/balance checks and placement with a clear avoidance line. Existing crash animation limitations remain.


## Sparse pothole visual candidate — 2026-09-12 [codex-maclaptop]

Elliott reiterated potholes should be occasional, not everywhere. Keep deliberately authored road hazards with a clear route around each, rather than blanket/random coverage. One shallow 8 cm depression is isolated in PiedmontPotholeVisualReview near Irwin; original main road and desktop047 remain unchanged. Alternate road passes9269 surface checks, preserves crossing trail, and has zero open internal mesh edges. A noncolliding dark surface overlay follows the colliding depression. Approach renders inspected: currently looks too much like a flat brown patch and has faceted shading; visual acceptance remains false. Actual bike traversal, improved asphalt edge detail and final sparse placement remain pending. Runtime contact fixture from previous entry is not a riding playtest.


## Pothole surface revision — 2026-09-12 [codex-maclaptop]

Replaced brown flat fill with procedural neutral asphalt aggregate and radial broken-wear rim; centred UVs at0.5 to tolerate importer V flip. First aggregate render was too bright and rejected. Final b603eef3e7754eb694d7ed48bcdc99ee approach1/2 inspected: dark interior and lighter irregular edge readable, suitable for actual traversal testing. Full visual acceptance remains false: close-up fracture detail, speed-dependent avoidance and gameplay/packaging pending. Geometry unchanged from9269-probe passing road. Scripts compile and native import/render exit0. One candidate only in review, main and desktop047 unchanged.


## Shallow pothole riding and main installation — 2026-09-12 [codex-maclaptop]

Added opt-in BattlePotholeRideAudit, normal W keyboard input and bike movement across the authored review road. Starts are teleported; contact actor is reset between passes and camera fixture cars removed. Near pass at130 cm lateral offset triggers zero contacts; direct pass triggers exactly one, no wipeout, grounded endpoint. Native editor build and test pass. This does not verify jump/deep recovery, busy traffic, real-physics mode or packaged feel.

Installed one shallow pothole near Irwin into main PiedmontWorld, replacing only Irwin road mesh with physically depressed alternative and adding one contact/visual actor. All9269 native road surface probes pass; original trail samples preserved. Sparse requirement retained. Desktop047 is still unchanged. Broader game, scenery, crash animations and remaining requirements remain unfinished.


## Farmers market at 12th Street — 2026-09-12 [codex-maclaptop]

Elliott requested a note for future implementation: place a farmers market beside fictional Billy’s at the 12th Street park entrance. During the untimed test ride, its stalls and barriers must physically block entry there, directing Ellison to the 14th Street stone arch where the timer begins. Keep the street tutorial routes usable. Carry the market atmosphere into adjacent park paths; this provides the setting for Farmers Market Vendor zombies. Vendor zombies may spawn and pursue only inside Piedmont Park. The homeless-themed and punk-rocker zombie variants may appear anywhere in the playable world. This is a requirement recorded for implementation, not an installed feature.

Reference research: Piedmont Park Conservancy’s Green Market page (https://piedmontpark.org/green-market/) and its event listing (https://piedmontpark.org/event/green-market-6/2025-12-13/) identify the real market and 12th Street/Piedmont Avenue location. Photo references: https://www.flickr.com/photos/taedc/5743665419/ ; https://www.postcard.inc/places/the-green-market-at-piedmont-park-atlanta-D2lNB0fE4Zq ; produce display https://piedmontpark.org/wp-content/uploads/2017/03/IMG_6441.jpg ; vendor reference https://www.patchworkcityfarms.com/market-csa . Visual direction: canopy tents along tree-shaded paved paths, folding produce tables, baskets/crates and handwritten signs. Use photos as reference, not imported game assets. Physical entrance closure is fictional gameplay staging, not a claim about the real market.

Acceptance pending: ride/run/jump attempts cannot bypass the tutorial entrance block; both street approaches to 14th remain open; park-side dressing feels connected to the market; vendor spawn and chase never cross the park boundary; other variants remain eligible beyond the park.


## 12th Street market source layout — 2026-09-12 [codex-maclaptop]

Retained fresh OpenStreetMap API bbox data in References/twelfth-market.osm. Gate node316638596 converts to world(-19107.994,3187.444); restaurant node1595679215 (Billy’s reference) to(-19150.221,4477.490). Pedestrian way28798697 defines the park-side approach. Scripts/prepare_twelfth_market.py produces ten authored300cm square stall footprints, five on each side of the path; every footprint is within the retained park polygon and at least50cm from mapped building footprints. This is not a surveyed market stall layout. Plot inspected in SourceAssets/Terrain/TwelfthMarket/layout-review.png. Small paths in the market footprint need gameplay treatment; both14th approaches and bypass prevention remain to verify.

Scripts/build_market_stall.py authors reusable six-material source OBJ geometry: fabric canopy/valance, metal legs and bracing, folding table/cloth, three slatted produce crates and produce. Source only; no native import, materials, collision, ground survey or rendered acceptance yet. Market not installed; desktop050 unchanged. Next: native terrain survey and art review, entrance setup barriers, street-route preservation and actual ride/run/jump bypass checks.


## Native market art study — 2026-09-12 [codex-maclaptop]

Scripts/review_twelfth_market.py imports six original stall mesh/material pairs and places ten transient stalls on main-world terrain without saving the map. Fixed missing OBJ UVs after an Interchange ensure; final import/render exits0. Ground sampled at canopy/table feet, up to23.97cm variation; levelling blocks connect ground to each raised leg. Importer reflects source Y: now measure imported Wood bounds and use that orientation for aisle-facing tables and table-leg supports. First support render rejected for misplaced blocks; corrected entrance render f647203633604311aac8ec521c879c91 inspected.

Assets under /Game/BattleForTheA/Environment/TwelfthMarket. Study remains visually rough: plain material colors, repeated produce, dark canopy shadows and conspicuous levelling blocks need art polish. No native gameplay/route/bypass checks, no entrance barrier, no main-map save, no desktop update. Report Tests/Results/2026-09-12-market-review.json contains ground samples and three capture paths.


## Market material revision; signage unresolved — 2026-09-12 [codex-maclaptop]

Added original deterministic512px wood/fabric textures via Scripts/texture_market_stall.py, per-face dominant-axis UV projection and varied produce positions/sizes. Native import/render exits0. Added candidate named sign boards (five fictional names, paired stalls), white23cm text on dark270x36cm boards. Close-up signage FAILED visual review: no text visible, despite component visible/not hidden, matching actor/component transforms, explicit rotation, unlit cloned default text material, two-sided diagnostic and29cm board offset diagnostic. Captures548b63f75c2b41c1847a0aec7cf80af2; source restored intended155cm local sign offset, capture used180cm. Do not claim signs readable or market accepted. Need diagnose actual text geometry/font/render capture versus native gameplay; shaded produce remains too dark. No main-map save or desktop update.


## Market signage fixed — 2026-09-12 [codex-maclaptop]

Replaced failed TextRender candidates with original baked lettering on a static sign panel, imported alongside stalls. Scripts/build_market_signs.py writes five1024x144PNG labels and270x36cm panel OBJ; no external photos or font files copied. Native review imports panel plus five unlit label materials. Import/render exits0. Capturesa0180a5f961b455395e199ad39f44215/stall.png and entrance.png inspected: PEACHES & GREENS clearly readable in shaded close-up, signs face aisle on both rows. This resolves that label presentation failure; full-market visual acceptance still false. Dark produce, repetitive stall contents, levelling blocks and market closure/gameplay work remain. Old M_MarketText asset is retained but no longer used by the review. Main map and desktop050 unchanged.


## Market shade and produce visibility — 2026-09-12 [codex-maclaptop]

Inspected current skylight: movable, intensity1, captured-scene source, real-time capture enabled. Recapture alone leaves static commandlet scene black in shadow. New optional -MarketRecaptureSky temporarily disables real-time capture and recaptures the existing skylight; canopy ambient becomes visible. This is a static-preview diagnostic, not a change to saved world lighting or proof of native game lighting. Review report records both original and diagnostic state.

Produce centres were99cm while crate rims reach108.5cm, occluding fruit from rider-height view. Raised centres to112cm without changing collision-bearing table/crates. Native import/render exit0, shaded close-up7e16e580a005491e96468f1077002171 inspected: colored produce visible above rims and sign readable. Still low-detail/faceted, repeated stalls and dark crate fronts; full art acceptance false. No main save, no barrier, no desktop update.


2026-09-12 [codex-maclaptop] Market access prerequisite fixed in source: old13th Street south expansion fence made12th market unreachable. Extended mapped Piedmont branch to market corner and moved south expansion stop beyond it. Native keyboard approach and both14th gate routes pass; see STORY-AND-TUTORIAL.md. Market entrance barrier and installed stalls still pending.


## Market gate closure prototype — 2026-09-12 [codex-maclaptop]

ABattleMarketClosure creates a360cm-high,1640cm-wide visible welded-mesh fence, feet and a readable FARMERS MARKET SETUP / PARK ENTRY:14TH STREET sign using engine UnlitText. Root grounds by native trace at mapped gate(-19107.993785,3187.443844). Only spawns under nonshipping BattleMarketClosureReview; not normal gameplay. Static review optional MarketClosurePreview places it transiently with market stalls.

Native build passes.237 sphere30cm sweeps across fence width at90/180/300cm pass. Initial test hit the adjacent tutorial street fence atY+600 instead of market fence; isolated sweeps now ignore that actor to test market geometry. Actual keyboard market approach still includes both fences and passes4471.1cm, maxerror4.3cm, untimed/no wipeout. Scripts/test_tutorial_routes.py --closure market reproduces; report2026-09-12-market-closure-market.json. Previous extension-only report preserved.

Static entrance renderbe8eaec49a83433bbdcf072f2ec764d7 inspected: sign readable, market visible behind. Fence deliberately prototype quality. Ground end fit, side bypasses, actual collision/jump attempts, and post-tutorial lifecycle not verified; do not treat as complete access restriction. Normal gameplay/main map/desktop050 unchanged.


## Tutorial-only market fence lifecycle — 2026-09-12 [codex-maclaptop]

ABattleMarketClosure now ticks and destroys itself once the active park mode leaves tutorial state. It does not evaluate lifecycle in BeginPlay, because the tutorial spawns it before setting initial practice state. Static editor art study is unaffected (no gameplay mode). Native build and Scripts/test_tutorial_routes.py --closure direct pass: fence present and237 isolated crossing sweeps pass during practice; actual7692cm keyboard gate route with max11.8cm centreline error; after real gateway start no market closure actor remains and countdown/single-start timer pass. Report2026-09-12-market-closure-direct.json. Still opt-in prototype: jumping, around-end bypasses and full closure design remain; desktop050 unchanged.


## Actual bike attempts at market fence — 2026-09-12 [codex-maclaptop]

Added opt-in BattleMarketImpactAudit, driven from closure actor. Teleports fixture1400cm west of gate onto ground, then actual W input at gear4 for6seconds; optional J at450cm approach. Native build passes. Both tests stay outside: ground attempt closest55.10cm, no airtime; jump attempt closest55.03cm, observed airtime and peak107.37cm above start. Both retain tutorial state/zero run elapsed. Script test_market_impact.py [--jump] verifies requested mode and jump peak>65cm.

Narrow fixtures only: no actual side bypass, on-foot jumps, different speeds/jump timings, ramps, or rendered crash/animation acceptance. Closure remains opt-in, no main installation or desktop update. Next access work remains perimeter/end bypasses and walking/jumping, not more identical direct collision sweeps.


## North return fence candidate — 2026-09-12 [codex-maclaptop]

Added sourced return from north market-fence end along park boundary toward14th, ending at horizontal line400cm south of gate. First nearest-point endpoint chose a point north of gate and was rejected before test. Scripts/prepare_market_return_fence.py now intersects the desired south line. First boundary-aligned candidate interfered with native direct tutorial route (failed200cm deviation). Revised points move parkward to clear the game’s widened/rounded road centreline by310cm minimum (road half-width225cm), max authored eastward adjustment490cm.183 points,7422cm total revised length, retained OSM provenance and authored offsets in return-fence.json.

ABattleMarketClosure grounds each point in BeginPlay and creates visible collision posts/rails. Native build passes; both full keyboard routes with closure pass: direct7695.1cm/error12.2cm; Juniper25013.9cm/error11.6cm. Fence removed after gate start in both. Existing237 sweeps test the central market fence only, not this new return. Return still requires per-segment ground/collision/bypass checks and visual review; static review does NOT run BeginPlay and therefore currently omits the return. No claim that all shortcuts are sealed. Opt-in only; desktop050 unchanged.


## Market return inspection — 2026-09-12 [codex-maclaptop]

Made BuildGroundedReturn callable and idempotent so transient static review builds the same return as BeginPlay. Native editor build passed; latest render commandlet exits0. All546 sphere crossings across182 return segments at three heights hit the closure;183 ground samples have maximum neighboring elevation change2.829cm. These are local segment checks, not perimeter containment.

Added market-end and gate-end camera views to Scripts/review_twelfth_market.py. Inspected six-view report directory work/market-review/6775dbca06e84fd8ab2a68d0698a0428. Market-end connection visibly overlaps; gate-end terminates openly near crossing paths, requiring an actual route/bypass check and connection design before activation. Images also show overlapping/raised path ends and strong dark shadows near gate, requiring geometry review. Fence looks overly heavy and remains prototype art. Review remains unaccepted; normal game and installed050 unchanged. Next: resolve gate-end escape without blocking either14th approach, review south boundary, then actual foot traversal and path surface cleanup.


## Market return to stone pier — 2026-09-12 [codex-maclaptop]

Extended north return323.71cm to actual14th gate south stone-pier centre, Gate+(0,360), with40cm sampling. New connection retains311.62cm minimum distance from both tutorial road centrelines; generator asserts greater than300cm and exact final pier coordinate. Total192 samples. Native editor build passes. Both actual keyboard gate routes pass with opt-in closure: direct7693.8cm/maxerror12.2cm, alternate25020.9cm/maxerror13.2cm. Gate starts once and removes closure on both. This closes the geometric separation identified in static view; standing/jumping around the pier and south end still require native verification. No new render acceptance or path surface repair claimed. Desktop050 unchanged.


## On-foot market impact — 2026-09-12 [codex-maclaptop]

Extended test_market_impact.py with --foot: normal Dismount creates/possesses Ellison, actual W+Shift input sprints at the barrier, Space initiates jump450cm before it. First fixture released Space in the same frame and failed airtime; corrected to200ms hold rather than relaxing acceptance. Native editor rebuild and --foot --jump now pass:112.81cm observed rise, airborne true, closest fence gap53cm, no crossing or practice countdown. This covers one central sprint-jump, not freeform perimeter containment.

South-end layout investigation: market fence and existing south street barrier intersect at(-19107.993785,3887.443844); market fence continues100cm beyond that crossing. No additional south return needed based on XY layout alone. Vertical/capsule traversal at junction, gate-pier bypass and overlapping path visuals still pending. Source-only; desktop050 unchanged.


## Gate path surface investigation — 2026-09-12 [codex-maclaptop]

Added read-only survey_market_path_seams.py, loads main map and finishes asset loading before sampling.88 vertical samples:43 terrain,33 concrete,12 asphalt. All45 pavement hits lie3.00–3.01cm above landscape. Component bounds inventory finds only those two pavement chunks and three canopy groups here. Initial survey before finish_editor_asset_loading was incomplete and replaced; disabling actor collision was not reliable, so final survey traces downward below each hit without scene mutation.

Transient MarketPathShadowReview disables shadow casting on those two chunks. Commandlet exits0, but inspected fa6ed518deec45f4931134cff1aa3c7a/return_gate_end.png still has the large dark shapes. Do not apply a blanket no-shadow or geometry-height fix from this evidence. Earlier description as raised overlapping paths was an unverified visual interpretation: sampled colliding surfaces are grounded. Next inspect concrete/asphalt material and rendered geometry correspondence. No saved map changes; desktop050 unchanged.


## Gate pavement identity resolved — 2026-09-12 [codex-maclaptop]

review_gate_pavement_materials.py temporarily replaces concrete with cyan and asphalt with magenta unlit materials, without saving assets/map. Explicit used_with_nanite avoids late usage compilation displaying checker fallback; final commandlet exits0 and both colors render correctly. Inspected image in Tests/Results/2026-09-12-gate-materials.json: previously dark detached-looking strips exactly match asphalt geometry. Original materials are M_ParkConcreteWorld and M_ParkAsphaltWorld, neither has a world-position-offset input. Combined with previous3cm ground survey, evidence rejects large floating/duplicated-shadow interpretation in this sampled gate area. Next improve asphalt readability and authored parallel path/end layout; no geometry lowering or shadow-disable fix warranted. Desktop050 unchanged.


## Weathered park asphalt applied — 2026-09-12 [codex-maclaptop]

Created original M_ParkAsphaltWeatheredCandidate with world-space aggregate variation, base linear(.13,.135,.14), roughness .85–.93, low specular and explicit Nanite usage. Native lit gate render inspected: previously near-black strips now read as gray asphalt with aggregate and retained tree shadows. No unlit production surface or geometry/collision adjustment. Reproducer create_weathered_asphalt.py; review_gate_pavement_materials.py -WeatheredAsphaltReview, result2026-09-12-gate-weathered.json. Initial material pin-name assertion corrected to unnamed unary inputs; final commandlet exits0.

install_weathered_park_asphalt.py applies only22 Park pavement SM_Park_Asphalt_* actor overrides in main PiedmontWorld, saves and reloads, verifies all22 material assignments; exits0. Road corridor actors excluded by exact label prefix. New material/map staged for next package; desktop050 unchanged. Whole-park motion/night appearance not yet accepted. Gate path dead ends/parallel layout still need authored cleanup separately.


## Market dressing saved to main park — 2026-09-12 [codex-maclaptop]

Explicit InstallTwelfthMarket mode in review_twelfth_market.py saves only authored dressing, rejects diagnostic sky/shadow/fence flags. Replaces only prior TwelfthStreetMarket tagged actors; previews rebuild those transiently to avoid duplicate stalls. Saved ten stalls/150 actors under Piedmont/12th Street Market, before review camera creation. Main map has no temporary market closure.

Install render process exits0; entrance image026305bee16f4e20b03183893f8dfc92 inspected with normal skylight. Shapes, produce and signs visible, but repeated stock, overlarge levelling blocks and general art quality remain rough. Fresh process verify_twelfth_market_install.py exits0:150 labels retained, ten each of six stall meshes and sign, metal/wood BlockAll, five aisle midpoint ground samples unobstructed by stalls. This is not full aisle player traversal. Closure activation and remaining perimeter tests pending; desktop050 unchanged.


## Market support polish — 2026-09-12 [codex-maclaptop]

Replaced conspicuous18cm-square wooden levelling blocks with metal leg continuations:4cm-square canopy adjusters and2.8cm-square folding-table adjusters, retaining measured support bottom/top elevations. Native normal-light entrance preview inspected; visible clutter reduced, stalls remain rough/repetitive overall. Reinstalled using explicit InstallTwelfthMarket; process exits0. Fresh-process persisted market verification passes after replacement (150 actors, same ten stalls/five aisle samples). No geometry or collision movement beyond narrower support cross-sections. Main map updated; desktop050 unchanged.


## South-junction sprint jump — 2026-09-12 [codex-maclaptop]

Extended actual keyboard impact fixture with --offset and --junction. Offset550 test crosses fence but starts beyond existing street closure, so retained failure does not demonstrate a reachable tutorial bypass. Corrected junction fixture approaches24.12degrees, from1000cm behind target at marketY+640, inside street boundary. Dismount at that narrow fixture position failed; normal practice-start dismount succeeds, then fixture repositions the possessed rider on the approach.

Native editor build and --foot --jump --junction pass:94.37cm rise, airborne true, closest35cm to market plane, no crossing or tutorial timer start. This covers one inside-angle sprint jump at south junction; not exhaustive perimeter closure or gate-pier freeform bypass. No production behavior changed. Desktop050 unchanged.


## Market practice closure enabled — 2026-09-12 [codex-maclaptop]

Added --pier actual keyboard fixture: normal tutorial dismount, reposition1000cm west of gate south pier, sprint and jump. Native pass100.99cm airborne rise, closest169.05cm to gate plane, no crossing/countdown. This is one aimed approach, not exhaustive freeform testing. Together with central bike/foot and south-junction tests plus both preserved street routes, enough evidence to enable current prototype for normal practice.

Moved closure spawn from nonshipping review flag into normal ABattleTutorial initialization after tutorial mode becomes active. Explicit skip-tutorial audit early returns still omit closure. Starts during untimed practice, clears when gate starts countdown; market dressing remains. Editor build passes; new actual direct route7693.1cm/error11.6cm passes with normal-spawn closure,237 sweeps and removed-after-start check. Previous alternate with same geometry remains passed; not rerun here. Saved stalls/materials and new closure/vendor code ready for next package verification. Desktop050 unchanged. Art polish and broad freeform/performance acceptance remain incomplete.


## Market requirement reconfirmed — 2026-09-12 [codex-maclaptop]

Elliott reconfirmed: farmers market at the 12th Street entrance beside Billy’s physically blocks park access during untimed test riding and influences the adjacent park area. Keep market stalls, shoppers and produce dressing connected to nearby park paths. Farmers Market Vendor zombies are park-only, including pursuit; homeless-themed and punk-rocker variants remain eligible anywhere in the playable world. This is a note-only request; existing implementation and acceptance records above remain authoritative. No gameplay changes in this update.

Additional photo references found: Conservancy honey/vendor stall https://piedmontpark.org/green-market-more-than-a-shopping-experience/ ; chef demonstration with canopy, produce table and brick-building context https://chefbeee.com/blogs/events/join-us-for-a-chef-demo-at-the-green-market-at-piedmont-park ; broader market canopy/crowd reference https://www.flickr.com/photos/cizauskas/25815935651 . Official location reference remains https://piedmontpark.org/green-market/ . References only, not imported assets.


## Optional realistic bike handling — 2026-09-12 [codex-maclaptop]
Current BattleBike lacked a real/arcade selector. Added P while mounted, with readable-mode text below the horn panel. Arcade remains default. Realistic mode uses a 110 cm bicycle wheelbase model, 32-degree maximum steering demand, lateral acceleration cap of 680 cm/s² on road or 350 on grass (reduced during braking), gravity projected along the floor tangent, rolling and quadratic air resistance, motor assistance tapering at the gear cap, and acceleration-derived lean. No powered steering or pedaling in air; coasting downhill may exceed the gear cap up to 2200 cm/s. This remains CharacterMovement-based, not a fully simulated two-wheel rigid-body bicycle. Native main-world keyboard controls, eased steering/lean, gears, remount persistence and P toggle both ways pass. Initial test checked key state before queued input processed; fixed timing and rerun passes. Hill/traction/high-speed/jump behavior, HUD render and packaged playability are still unverified; mode is a development implementation, not full physics acceptance. Installed055 unchanged.


### Controlled realistic-handling movement checks — 2026-09-12 [codex-maclaptop]
New opt-in native audit creates a transient elevated colliding plane and runs the actual bike movement. On a 10-degree grade, 600 cm/s coasting falls to411.264 uphill or rises to746.847 downhill after1s. Flat braking stops from600; full steering at1600 produces max lateral acceleration679.951 on pavement or349.956 on grass, with24.817/13.015degrees yaw. All grounded samples stay grounded. During0.2s airborne with W/D held, speed stays600 and heading stays unchanged. Editor build and six phases pass. Initial positions/speeds are injected; the plane is a controlled fixture, not actual park hills or full tire simulation. No persistent map mutation, HUD render or package update. Remaining acceptance includes actual-world hills, jump transitions, visual readability and subjective balance.


## Scooter scene support review — 2026-09-12 [codex-maclaptop]

Expanded the transient Krog scooter from five black primitives to eleven distinguishable deck, grip, stem, wheel and hub parts with original metal/green trim materials. Added native editor-only skinned LOD0 ground-clearance measurement and fitted each frozen character using clothing/shoes: 24,899 vertices per role, minimum +1 cm after adjustment. Face-asset geometry near the root skewed whole-asset measurements, so it is excluded; this is not full visible-mesh or physical support proof. Forced character LOD0 for matching capture scope and enabled diagnostic fill-light shadows.

Both final views at work/krog-wreck-review/03d83dc278064f3bad696d4e9095e493 were inspected. The scooter reads more clearly; the people still appear insufficiently supported, despite the narrow numerical clearance result. Do not treat this as visual acceptance or integrate these frozen poses as finished gameplay. Next resolve the support/shadow discrepancy, replace get-up snapshots with credible injured/helper idles and varied characters, then implement rare offscreen staging/collision/route checks. Editor build and rendering exit0. No map saved, no event implemented, installed056 unchanged. Evidence: Tests/Results/2026-09-12-krog-wreck-visual-candidate.json.


## Scooter review shadow diagnosis — 2026-09-12 [codex-maclaptop]

Extended the native clothing-clearance helper to return each closest vertex, ground point and hit actor. All nine sampled minima hit the USGS landscape; diagnostic markers align with the visible clothing/footwear. The previous pose-evaluation explanation did not account for the apparent suspension. Comparing otherwise unchanged poses under different diagnostic light settings identifies detached shadows as the main visual contributor: bias .15, slope bias .3, contact length .03 and shadow resolution scale4 improve contact appearance. A near-zero-bias trial caused striping and was rejected. Both final marker-free views at work/krog-wreck-review/e36bce3a759f4f1681c7011370ccdbd8 were inspected.

This fixes the review lighting only; it is not a main-world lighting change or complete scene acceptance. Preserve explicit pose/character-variety/hair/rare-event/collision requirements. Native build and final render exit0, no map saved, installed056 unchanged. Diagnostic point markers are optional via SHOW_CONTACT_MARKERS.


## Scooter encounter character distinction — 2026-09-12 [codex-maclaptop]

Replaced the identical standing male with the existing female City Sample character and its native idle clip; retained male injured/helper snapshots. Added distinct blue, orange and green clothing via transient material instances using the actual A/B_CrowdColor parameters (generic Color_Tint parameters did not affect these materials). Original material assets are unchanged. Hair cards now attach in bind pose before animation and follow head movement; static hair components initially rejected attachment to movable bodies, corrected by explicitly setting movable and asserting the parent.

Both final views at work/krog-wreck-review/9494f389157b47ac8990b9eeeee3a135 were inspected. Character roles are more distinct; hair highlights remain coarse and the two get-up snapshots are not credible finished injured/helping loops. Local retarget inventory contains get-up clips and reaching clips, not a dedicated helping idle. Final commandlet exit0, measured clothing ground checks pass. No map saved or installed056 change. Rare runtime activation and player interaction/collision remain pending.


## Native incident pose lifecycle — 2026-09-12 [codex-maclaptop]

Added BeginIncidentPose/ReleaseIncidentPose to existing APiedmontPedestrian instead of creating unresponsive display characters. It holds an authored sequence frame, suppresses destination changes, and resumes the rest of the sequence after its hold timer or a nearby horn. Once get-up completes, ordinary idle/navigation resumes. Bike impacts and damage cancel the pose before existing knockdown/death handling; sleeping and bench-reaching entry reject an active incident pose. The underlying sequence hold flag resets on new/stopped sequences.

Native editor build passes. Scripts/test_native_incident_pose.py launches the actual game and checks held-pelvis stability after 0.6s pose-transition settling (0cm drift), horn recovery, timed recovery, bike-impact knockdown and damage interruption; all pass, exit0. Initial fixture incorrectly compared the initial standing-to-pose transition against the held frame; failed report retained separately. This is lifecycle support, not a complete placed event: prone/kneeling collision envelopes, compatible-role appearance/placement, animation quality and rare offscreen activation are still unverified. No main-map or installed056 changes.


## Incident posed collision — 2026-09-13 [codex-maclaptop]

While an incident pose is active, a hidden query-only skeletal component uses the existing City physics asset and updates its physical bone transforms from the visible pose each tick. It blocks Pawn and Visibility queries; the standing movement capsule is disabled and movement paused. Clearing/interruption destroys that component and restores capsule/movement before existing recovery or impact handling. Get-up checks the standing capsule space first and leaves the pose held while blocked.

Native build and Scripts/test_native_incident_pose.py pass. The fixture verifies a complex Visibility ray through the posed head, no person target in empty standing-height space, a 20cm-radius Pawn-object sweep across the pelvis, and waiting under a temporary overhead blocker before horn recovery. Held pose, timed recovery, bike-impact and damage interruption remain passing. This is selected query coverage only: the first all-world sweep did not return the target (retained failure report); the passing Pawn-object sweep isolates that query from terrain. Actual bike passage/contact, full limb/clothing collision envelopes, moving obstructions and rendered native incident acceptance remain to test. No main-map or installed056 changes. Report Tests/Results/2026-09-13-native-incident-pose-collision.json.


## Actual bike traversal against incident pose — 2026-09-13 [codex-maclaptop]

Added a native W-input traversal fixture on an isolated flat floor: settle an existing pedestrian, hold the injured frame, ride past 300cm to the side, reset the bike start and ride directly through the posed pelvis corridor. Both checks pass: zero near-miss contacts, then one actual movement collision, incident pose cancelled, pedestrian knockdown phase1 and exactly one bike wipeout. No direct BikeImpact call is used by this fixture and no bike movement code change was needed.

Build and Scripts/test_native_incident_bike.py exit0. This establishes one arcade approach, not all directions, kneeling poses, full recovery or the mapped Krog placement. The rare event director and normal-world scene are still pending. Report Tests/Results/2026-09-13-native-incident-bike.json. Installed056 and main map unchanged.


## Native rare scooter scene candidate — 2026-09-13 [codex-maclaptop]

Implemented ABattleScooterScene, not yet placed in the saved main map. Default appearance selection is18% once per actor/level instance. It waits until the player is at least35m away and the conservative5m scene sphere lies entirely behind the camera. Ground/standing-clearance checks precede spawning. Three existing interactive pedestrians are hidden while settling and entering the two held poses; the third uses ordinary idle. Eleven collidable scooter parts reproduce the reviewed geometry. Distinct clothing uses transient material instances. Scene reveal waits for pose settling and another offscreen check. A player visit within15m starts a25s timer to release the poses and let people resume navigation. Mode-ended worlds do not advance the scene. No repeated re-selection or visible despawning.

Native build passes. Forced chance0/1 fixture at (30349.800013,114057.877225,1110.508188) passes: visible wait, offscreen assembly, two incident poses, third bystander, eleven scooter parts and empty skipped case. Scripts/test_native_scooter_scene.py and render_native_scooter_scene.py reproduce. Native render at work/scooter-scene-runtime/85648b66da914242af0ceb3e5559a521/scene.png inspected: distinct characters/scooter present under normal world lighting/HUD; pose/contact-shadow and hair presentation remain rough. No final art acceptance.

Still required before main-map/install promotion: complete route clearance and actual approach, visit-triggered recovery/navigation, hidden-stage behavior if camera turns during setup, normal rarity/run-reset behavior and broader character presentation. Native lifecycle/contact fixtures from earlier turns remain separate scope; this capture is not whole-scene collision proof. Installed056 and main map unchanged.


## Visit completion failure and queued recovery fix — 2026-09-13 [codex-maclaptop]

Extended native scooter audit with --visit: position the player nearby, run the real25s hold plus get-up time, then require all three participants to move at least1m. It fails at the mapped site: all remain stationary; rider and bystander leave pose/idle pause, helper remains held. Diagnostic standing overlap lists are empty by the end. Nav projection within10m finds coverage844.44,827.87 and891.98cm away for the three participants. The scene's physical-ground placement is outside the retained walking mesh. Evidence saved in2026-09-13-scooter-visit-navigation-gap.json and failed2026-09-13-native-scooter-visit.json.

Fixed an independent recovery bug: ReleaseIncidentPose now retains an external request when its first standing-space check is blocked, by expiring the hold timer. Tick retries until space clears. Updated native obstruction fixture removes its second horn: it now passes automatic recovery after blocker removal, alongside held pose, damage/bike interruption and collision checks. Build and fixture exit0.

Next: survey a safe ground-following navigation connection from this grass scene to the existing walkable network and validate actual exits; do not just teleport participants8m onto the road or remove the movement acceptance check. Navigation export currently deliberately includes RidePath/RideDirt/bridges/barriers, not the whole terrain, so the connection needs a local authored solution. Full visit remains failing until that is done. No main-map/installed056 changes.


## Grounded scooter navigation connection — 2026-09-13 [codex-maclaptop]

Read-only survey now takes endpoints from retained BattleKrog_ path splines at their authored elevations. Nearest arbitrary navigation projections had selected road surfaces and railway decking; those alone were insufficient to choose a pedestrian destination. Found an18.03m ground corridor to mapped route point (29385.985172,115581.759002,997.378565), with50cm-spaced ground/clearance sampling across220cm and overhead obstruction checks.

Created separate PiedmontScooterReview from main. Added236 hidden, collision-disabled navigation tiles fitting sampled real ground: a staging pad and2m connector. Custom navigation geometry exports despite NoCollision; the tiles neither render nor create a physical platform. All236 follow-up ground traces still hit original surfaces within0.1cm. Native Recast projects the scene and destination and returns an1807.19cm connecting path. No main-map mutation. Reproducer Scripts/build_scooter_navigation_review.py; helper only creates tiles in the dedicated review map.

The previously failing real25s visit now passes in the review map: both poses release, all three resume walking, minimum observed travel801.37cm. Native comparison preserves all826 sampled routes and796 reachable samples with zero regressions (30 existing unreachable samples remain). Reports2026-09-13-native-scooter-visit-nav-review.json and scooter-navigation-regression.json. Native bike route through the complete scene, broader traffic/geometry behavior, normal selection and final presentation still need acceptance before main-map promotion/package. Installed056 unchanged.


## Scooter route and removal checks — 2026-09-13 [codex-maclaptop]

The first forced-scene mixed-traffic realistic-handling Krog ride failed before reaching the encounter: 159.36m traveled, no wipeouts, 16.24cm maximum path error, stopped at (26329.779,116191.721,1087.254). Scene assembled three participants but nearest rider approach was45.51m, so visit never began. Preserve this failure in Tests/Results/2026-09-13-scooter-krog-ride.json; it does not establish route acceptance. The braking-only automated driver needs diagnostic evidence of its blocker before changing game traffic or repeating this run.

Changed scene visit radius from15m to25m so the mapped path about18m away can trigger eventual get-up. Added EndPlay cleanup of owned participants, including partial assembly/abort; forced-scene fixtures remove existing scenes first to avoid duplicates after future map placement. Native build succeeds. Main-map placement and installed056 remain unchanged; final scene animation/art and mixed-traffic passage remain unfinished.

Native review-map visit and removal verification passes: both held poses release, all three participants walk (minimum823.78cm), and all three are invalid/destroying immediately after scene removal. Report2026-09-13-native-scooter-visit-nav-review.json. This verifies one forced encounter lifecycle, not random normal-game placement or route acceptance.


## Krog mixed-traffic diagnostics and lamp boundary fix — 2026-09-13 [codex-maclaptop]

Repeated full ride failed at the same location after159.36m: stationary BattleRoadCar_22 reported BattleRoadCar_21 as blocker. Added opt-in route diagnostics reporting waiting people and nearby car state/obstacle/route completion; short --yield-probe terminates on a prolonged car wait only. The first probe caught a pedestrian wait before that restriction; retain it as diagnostic-only evidence. A later run completed both legs481.52m,0wipeouts,34.85cm maximum centreline error, all46597 ground samples paved. Forced scooter visit passed with nearest approach18.047m and three participants. Earlier intermittent car queue did not reproduce in that run and remains unresolved; no claim of a traffic fix.

That completed ride failed the light check:6251 lit and3234 unlit tunnel samples. Automatic lighting previously checked only the headlamp location, whereas the bike body can already be inside a dark volume. Updated native UpdateLights to consider either body or lamp. Native narrow-volume regression passes body-only illumination, lamp-only illumination, exit hysteresis and daylight shutoff. Editor build and Python checks pass. Complete Krog re-drive/rendered lighting after this fix still pending. No main-map scooter placement or package update; installed056 unchanged. Reports2026-09-13-scooter-krog-yield-diagnostic.json,2026-09-13-krog-queue-probe.json,2026-09-13-krog-car-queue-probe.json,2026-09-13-native-light-boundary.json.


## Confirmed DeKalb endpoint queue and turnaround candidate — 2026-09-13 [codex-maclaptop]
The post-light-fix ride stopped before the tunnel, so it cannot verify lights. Queue diagnostics identify the cause: lead BattleRoadCar_1 at(25666.958,116804.546,1088.919) has finished=1; two following cars stop behind it. The population director intentionally retains a visible endpoint car. This is a real stationary queue at a short authored lane end, not a scooter collision. Evidence2026-09-13-scooter-krog-light-recheck.json.

Added opt-in closed car routes: validate closed/tangent-continuous endpoints; wrap distance/crossing reservations without teleporting, stopping or resetting lifetime travel. Nonloop routes retain their behavior. Director lane setting forwards loop mode. Built a separate PiedmontKrogTurnaroundReview from scooter review, connecting opposing DeKalb lanes with slow250cm/s semicircles and two crossing bindings. 594 native road footprint probes match source surfaces. Native one-car test completes175.56m loop plus8m continuation, maxframe displacement3.90cm, no obstacle; camera follows throughout. This is narrow collision/continuity evidence; the tight200cm turn radius and wheel appearance still need rendered review, plus mixed-traffic behavior. No main promotion.

Mixed realistic rider/cars/pedestrians/scooter/light test started as session96726, script test_krog_candidate_drive.py --scooter --turnarounds --crowds --real-handling --report2026-09-13-krog-turnaround-mixed-ride.json; logwork/krog-turnaround-drive.log. Poll existing handle/report before starting a replacement.

Prepared install_scooter_scene.py to back up and extend current main rather than overwrite it. It requires verified mixed ride and already-promoted loop traffic; not run. Main nav-tile helper now permits main as well as review. Installed056 unchanged. Full game, traffic/art acceptance, main promotion and packaging remain unfinished.


## Mixed traffic, steering and contact-shadow review — 2026-09-13 [codex-maclaptop]
Turnaround mixed ride avoided the finished-car endpoint queue, triggered the scooter scene, and traversed the tunnel in both directions with18352lit/0unlit samples. Overall failed after344.55m/one completed leg because the braking-only driver waited indefinitely for a stationary pedestrian188.3cm beside the line. Retained2026-09-13-krog-turnaround-mixed-ride.json. Changed test-driver pedestrian check from fixed220cm half-width to colliding component bounds+50cm, minimum100cm, and a2s lateral-velocity crossing prediction. Ordinary game input/AI unchanged. Full corrected mixed recheck remains pending; intended report2026-09-13-krog-turnaround-mixed-clearance.json.

Rendered loop exposed frame-dependent front-wheel steering (Turn*12 per frame). Native car wheels now derive target angle from264.2cm wheelbase and yaw per distance, clamped55degrees and exponentially eased by travel/speed; stopped wheels retain their angle. Editor build passes. First capture wrote onlyone image due FParse comma handling; fixed by disabling stop-on-comma. Second capture produces allsix images and completes a full loop+8m, maximum frame movement62.24cm at render hitches. Images0,1,4 at work/krog-turnarounds/3ac8ed5e4aeb449ba5f8fa7dbb167c05 inspected: wheel turn visible, but car-ground contact still appears weak; no final visual acceptance. Otherthree images still available for review. Report2026-09-13-krog-turnaround-steering-render.json preserves that set.

Review-map sun was movable with bias.5/slope.5, contact shadows0, resolution scale1. Set review-only bias.15/slope.3/contact.03/resolution2 in review_turnaround_contact_shadows.py. Main lighting unchanged. Fresh six-image render is live session99721, output report2026-09-13-krog-turnaround-render.json; poll existing handle before restarting. Shadow appearance/performance not yet accepted.

Prepared selective promote_krog_turnarounds.py and updated install_scooter_scene.py dependency to the corrected mixed report. Neither run. Promotion only transfers traffic settings; if accepting the new shadow treatment, its main-map transfer still needs implementation and verification. No main changes or packaged update. Installed056 unchanged; full game remains unfinished.


## Turning tyres grounded; shadow-quality investigation — 2026-09-13 [codex-maclaptop]
Rejected low-bias shadow candidate after first image showed road self-shadow stripes; intentionally terminated its owned render after confirming that artifact, not because of an observation timeout. Preserved2026-09-13-krog-low-bias-rejected.json. Original bias with contact shadows removed stripes but produced noisy detached outlines; no visual acceptance.

Added native LOD0 lower-tyre-vertex/real-road queries at allsix turning snapshots. All24wheel snapshots pass: minimum gaps range-0.2337to+0.3114cm, zero missing ground traces. This rules out a significant tyre lift in those snapshots; don't lower wheels to disguise bad shadows. Report2026-09-13-turning-tire-contact.json and imageswork/krog-turnarounds/6a4e9ffbb25644d3b29bba41205c241e. Build passes.

Found project defaultsg.ShadowQuality=1, whose engine settings cap CSM at1cascade/1024resolution with.7distance scale. Current sun requests4cascades over40000cm with distribution exponent3, so the cap spreads detail across a large range. Restored original review sunlight(bias.5/slope.5/contact0/resolution1). Started isolated render with command-line r.Shadow.CSM.MaxCascades2, without enabling group2fog/other effects. Live session1410; output2026-09-13-krog-turnaround-render.json. Poll handle/report before restart. Verify override, appearance and performance before modifying production settings.

Updated Design/BUILD.json unreleased notes; installed056/main unchanged. Mixed-clearance ride still needs running after visual work; traffic and scooter promotion scripts remain unrun. Full game unfinished.


## Two-cascade playtest profile and driver right of way — 2026-09-13 [codex-maclaptop]
Allsix two-cascade images inspected at work/krog-turnarounds/10ba06c6c0294e39acbabf1755d68598: restored full near-car/fence shadows with original sun bias/contact0, visible wheel steering and barrier clearance. Accepted this treatment for the turnaround playtest; soft shadow edges, tight radius, generic art and full performance remain unfinished. Updated render report visual scope accordingly. Added Config/DefaultScalability.ini with only[ShadowQuality@1]r.Shadow.CSM.MaxCascades=2. Native automatic-light/profile test passes and reads2without command-line override; build passes. No main sunlight transfer is necessary because review settings were restored to original values.

First corrected mixed ride failed after186.03m at mutual yield: car reported BattleBike_0 as obstacle while driver waited for that stopped car. It also exposed oversized colliding-component bounds in pedestrian lookahead (logged people over5m off line). Driver now uses standing capsule radii+margin, conservative200cm allowance for low poses, and the existing lateral motion prediction. If a stopped car is explicitly yielding to this bike, driver proceeds only when a sweep of the real bike capsule against that car component finds a clear path ahead. Collision/wipeout behavior unchanged. Failure preserved2026-09-13-krog-mutual-yield-failure.json.

Rerun live session5916, same test_krog_candidate_drive.py --scooter --turnarounds --crowds --real-handling --report2026-09-13-krog-turnaround-mixed-clearance.json, logwork/krog-turnaround-drive.log. Poll existing session/report before restarting. Current main/installed056 still unchanged; if full ride passes, selective traffic promotion and scooter installation scripts are ready, followed by fresh verification via verify_scooter_main_install.py (prepared, not run). Full goal remains unfinished.

## Krog traffic and scooter main integration — 2026-09-13 [codex-maclaptop]

Review mixed ride passed 481.54m in both directions, zero wipeouts, 5,067 lit tunnel samples and zero unlit, with cars/pedestrians and a scooter visit active. This seed did not exercise the stopped-car capsule-clearance exception. Promoted only lane settings onto the existing main Krog director/crossing. Backup: work/map-backups/pre-turnarounds-20260913-013551/PiedmontWorld.umap.

Installed scooter navigation and scene into current main. An initial preservation check falsely compared temporary Python struct addresses; corrected to the four GUID integer fields. Fresh backup/main comparison proved all 804 original actor IDs preserved, with one navigation actor added. Resumed scene installation without duplicate tiles; save now occurs after preservation check. Pre-navigation backup: work/map-backups/pre-scooter-20260913-014057/PiedmontWorld.umap; pre-scene backup: pre-scooter-20260913-014300/PiedmontWorld.umap.

Fresh-process main verification passes: one scene, chance0.18,236 hidden noncolliding navigation tiles, native route1807.19cm, one loop lane/two crossing bindings. Native main forced visit passes real25s release, all3people walking(min811.1cm) and cleanup. Reports: 2026-09-13-scooter-main-install.json, scooter-main-fresh-read.json and native-scooter-visit.json. Normal random startup, full main mixed ride and packaged acceptance remain pending. Installed056 unchanged. Full world/art/gameplay goal remains unfinished.

## Main mixed ride and hidden scooter startup — 2026-09-13 [codex-maclaptop]

Main mixed route tested using --main --scooter --crowds --real-handling. It FAILED at185.94m with no crashes, before tunnel, because test rider and westbound car mutually yield. Rider(28896.426,115622.833), car(29278.283,115643.170); car stopped for bike or pedestrians, not finished or waiting at crossing. The capsule straight path is actually blocked; do not bypass collision or claim review success proves main pass. Scooter visit itself passed atnearest2135.74cm. Report2026-09-13-main-integrated-krog-ride.json. Investigate the physical approach and test-driver steering limitations next; no packaged057 yet.

Found a real startup gap: behind-camera-only scene placement could wait indefinitely while1.26km ahead of home. Added static-world visibility sampling at eight envelope corners and four actual participant/scooter positions, all requiring blocking geometry at least6m before the target. Keep35m player exclusion and existing behind-camera branch. Native full-intro/home-camera startup passes (selection forced only):3participants ready atworld5.2s,125922.5cm away. New test flag ends Check to avoid the existing audit intro-skip rule and requires an actual opening-finished log.

Visibility regression uses reviewed camera550cm east/300cm up, verifies clear static line of sight to the scene, and updates camera manager immediately. Earlier50m camera was occluded and first-frame stale camera could give a false result; preserved failure report. Final native scene test passes visible wait, chance0skip, two poses/three people/11parts, offscreen assembly and cleanup. Build passes. Scripts/test_native_scooter_startup.py and reports2026-09-13-native-scooter-startup.json/native-scooter-scene.json cover these cases; no random distribution, rendered occlusion edge cases or packaged acceptance claim. Full goal remains unfinished; installed056 unchanged.

## Krog approach geometry diagnosis — 2026-09-13 [codex-maclaptop]

Read current main-map Krog spline and traffic lane geometry, plus450 native surface probes, using Scripts/inspect_krog_standoff.py. No map mutation. Scripts/plot_krog_standoff.py produces work/krog-standoff-layout.png; inspected plot shows mapped rider approach hugs westbound lane and then cuts across the stopped car footprint. Five saved centreline points fall inside the2D car box at the observed failed-run position, using yaw161.557degrees inferred from nearest saved lane tangent. Exact runtime yaw was not captured, so this is geometry diagnosis, not a full collision replay. Report2026-09-13-krog-standoff-diagnosis.json retains scope and points.

Next: correct/verify the approach or demonstrate an actual controlled steering manoeuvre around traffic before calling main route acceptance passed. Do not solve by disabling collisions, deleting visible cars, ignoring blocking sweeps, or presenting review-route success as main acceptance. User expects cars as obstacles and free steering; centreline-only test driver is not a complete human-playability test. Preserve real crossing geography and physically verify any alternate line/footway against native surfaces. No production change or package this turn; installed056 and goal remain unfinished. Matplotlib installed in existing external atl-geography-env solely for the diagnostic plot.

## Wider Krog approach candidate — 2026-09-13 [codex-maclaptop]

Surveyed100/150/180/220cm outward approach offsets on current main: all108ground samples per option have support, but17/24/45/49 are unpaved. Therefore an offset alone is insufficient. Created separate PiedmontKrogApproachReview from current main, with220cm eased outward route offset over the final24m and a2.4m-wide ground-fitted concrete strip. Only BattleKrog_2 changes.40centre probes pass;78triangles. First endpoint-edge trace failure fixed by extending the strip10cm beyond both endpoints. Native62cm sphere sweeps at95/155cm above actual ground across the whole approach find0static obstructions in main and review. Full bike dynamics/traffic still pending.

Files: Scripts/survey_krog_approach_options.py, create_krog_approach_review.py, check_krog_approach_clearance.py, render_krog_approach.py; SourceAssets/Terrain/KrogApproach/KrogApproach.obj; native asset Content/BattleForTheA/Environment/KrogApproach/SM_KrogApproach.uasset; reports2026-09-13-krog-approach-options/review/clearance.json. First renders work/krog-approach/wide.png and rider.png inspected but buildings obscure the approach; visual acceptance remainsfalse. Render script camera positions adjusted afterward but NOT rerun yet. Need proper overhead/rider view and join validation before promotion.

LIVE native ride session25167: python3 Scripts/test_krog_candidate_drive.py --approach --scooter --crowds --real-handling --report2026-09-13-krog-wider-approach-ride.json. Confirmed live by write_stdin this turn. Logwork/krog-approach-drive.log; outputwork/krog-wider-approach-result.log. Last observed leg0at60s/13118cm, with normal pedestrian waits. Poll the same handle or inspect process/report before starting anything else; do not restart from observation timeout. Main map/installed056 unchanged. Full game unfinished; no wider-approach success claimed yet.

## Correction: approach isolation and actual candidate — 2026-09-13 [codex-maclaptop]

IMPORTANT: The first claimed wider-approach ride did NOT test the wider approach. SaveMap wrote a copy but kept main active; the builder modified/saved main while the review stayed baseline. Fresh inspection found main807actors with approach, review806without. Marked2026-09-13-krog-wider-approach-ride.json passedfalse/invalid_candidate_evidence. Its481.5m result proves only a different baseline traffic seed can pass, not that the proposed fix works. Prior notes that main stayed unchanged and candidate clearance/render labels were correct are superseded by this correction.

Restored main by removing only tagged KrogApproachReview actor and restoring BattleKrog_2 points from untouched review. Preserved all remaining main actor GUIDs;806actors. Exact misplaced-state backupwork/map-backups/approach-isolation-repair-20260913-021007/PiedmontWorld.umap. Restore script/report retained. Original-copy GUID comparison was invalid across SaveMap duplication; repair instead verifies actor class/label inventory against copy and preserves main's own GUID set minus only removed patch. No blanket git restore. Main's postrepair SHA2567717e848f76af2541fe49e328b97c5ea747e45cc3e7eae7250eb591312b737d1.

Builder now explicitly loads/validates PiedmontKrogApproachReview after SaveMap and asserts main file hash unchanged. Actual candidate rebuilt with220cm offset/240cm concrete strip. Added dedicated M_KrogApproachConcrete with Nanite usage; first actual render checkerboard was missing material preparation. Set mesh precision8/full fallback. Underlay survey then found a real2.013cm intersection with older concrete between vertices; lifted authored finish from1.5to5cm. Fresh312interior samples nowminimum1.487cm above underlying surfaces. Both final overhead/rider images inspected: no checkerboard or observed overlap. Surface accepted for drive testing only; terminal shape, curb detailing and whole streetscape art remain rough.

Fresh clearance check requires main0/review1tagged approach actors and passes both maps. Native connector audit now requires exactly1candidate actor and logs world name when --approach is used; Python requires that actual-game log. Editor build passes.

New actual-candidate ride LIVE session65919: python3 Scripts/test_krog_candidate_drive.py --approach --scooter --crowds --real-handling --report2026-09-13-krog-verified-approach-ride.json. Logwork/krog-approach-drive.log, outputwork/krog-verified-approach-result.log. Poll same handle/process before starting another UE process. Its result is not known yet. Before promotion: verify this real ride, rebuild candidate navigation (native helper whitelist currently lacks this review name), compare route connectivity, selectively transfer only accepted geometry/spline into backed-up current main, then fresh/cooked acceptance. Installed056 unchanged; full game unfinished.


## Wider approach promotion and navigation repair — 2026-09-13 [codex-maclaptop]

Actual candidate ride 2026-09-13-krog-verified-approach-ride.json passed: runtime actor/world guard verified, 48385.74cm over both legs, 0 wipeouts, 41016/41016 paved samples, 10793 lit and 0 unlit tunnel samples, cars/pedestrians active. This is one traffic run, not all-seed or full-game acceptance. Selective promotion added one concrete actor and copied only BattleKrog_2 points into backed-up main, preserving other actor GUIDs. Prepromotion backup: work/map-backups/pre-krog-approach-20260913-022711/PiedmontWorld.umap.

Main navigation rebuild preserved 826 route samples (796 reachable, 30 preexisting unreachable), but dropped scooter grass coverage. Thus the initial promotion report alone did not prove scene navigation. Existing custom navigation flag was already serialized as EvenIfNotCollidable and the mesh had NavCollision; merely setting the flag/UpdateComponentData did not fix it. BuildParkNavigation now unregisters/re-registers RideNavOnly components and prepares mesh navigation collision before updating navigation. The combined change restores the grass link; separate causality of the two operations has not been established. Physical collision remains disabled, no landscape-wide navigation export. Both build and finish helpers accept the isolated approach review.

Review rebuild/fresh reload passes with 1807.003cm scene-to-path link. Backed-up main repair preserves actor GUIDs and route samples; fresh main read confirms identical 52-point approach, 40/40 new-surface probes, hidden noncolliding 236-tile connection and looping traffic settings. Main backup: work/map-backups/pre-nav-export-repair-20260913-024002/PiedmontWorld.umap. Native main scene visit passes: real25s hold, both poses released, all3participants walk, minimum829.85cm, all3cleaned. Verification scripts now invalidate their old success before starting; repair saves only after scene connectivity and GUID checks.

Editor build succeeds. Updated main mixed-traffic bidirectional ride is running separately; no main ride success or packaged acceptance claimed here. Installed056 remains unchanged. Full game, scenery, animation polish and performance remain unfinished.

Main mixed ride completed successfully: 2026-09-13-main-wider-approach-ride.json, actual PiedmontWorld plus candidate-actor guard, 2legs/48394.89cm/0wipeouts/55.63cm maximum centerline error. All39915ground samples paved,3281lit/0unlit tunnel samples,20moving cars and25walking people observed, scooter visit started with3participants. One traffic realization; does not establish all-seed congestion, rendered appearance or performance. Preparing057 from this checkpoint; installed056 retained until staged acceptance. — 2026-09-13 [codex-maclaptop]


## Mac057 installed — 2026-09-13 [codex-maclaptop]
UAT succeeded in148.86s. Packaged main ride passed48559.66cm both directions,0wipeouts,6044/6044paved samples,905lit/0unlit tunnel samples,20moving cars/25walking people, actual approach actor verified. Packaged scooter visit released all3people with minimum821.91cm walking and clean teardown. Easy/Hard finish guard, ordered crossings, win stats/frozen clock/save checks passed. Packaged12.01s protected opening completed; three1280x720images copied to work/build057-opening-review and inspected. Text is readable and bungalow materials appear; hand-to-handlebar alignment, simplified rider/porch/street and terrain remain visibly unfinished. These are incremental playtest results, not final art or general performance acceptance.
Installed strict ad-hoc signed0.57.0 at /Volumes/Adam Assets/Unreal/Builds/BattleForTheA/Mac/TheBattleOfATL.app; desktop shortcut verified. Preserved previous app as TheBattleOfATL-build056.app. Installed executable health/phone/cardinal hunt/two death resets/recollection/two checkpoints passes. No process left live after these checks. Source checkpoint49ac1fb; unrelated preexisting local changes remained present during packaging. All detailed reports use2026-09-13-build057 prefix. Next priority is visible rider hand placement and broader art/animation quality. Full objective stays incomplete.


## Mounted hand grip and braking pose — 2026-09-13 [codex-maclaptop]
Installed057 opening showed wrists aimed toward bars but fingers left open and wrists inheriting forearm orientation. BattleBike::PoseRider now orients palms independently, places wrists behind the handlebar so palms contact it, and curls finger/thumb joints. Existing raised right-arm pistol pose retained. Three native close left/front/right captures include a full braking pose; visible wrist separation corrected, fingers curled around bar. Images preserved under work/rider-grip-closeup; ordinary opening frame under work/rider-grip-opening. Model remains visibly low-poly and exact skin/bar contact across dynamic motion is not proven.
Torso brake lean now responds proportionally to brake input and blends with exponential time response rather than snapping instantly8degrees. Static review RefreshRiderPose still sets requested endpoint pose directly. Editor build succeeds; both native arcade and realistic WASD/arrows/brake/steering/remount/P-toggle checks pass (2026-09-13-rider-grip-handling.json). No full dynamic rendered-motion acceptance or packaged update yet. Test opening script accepts --editor and --grip; closeup camera/brake override is gated by nonshipping BattleOpeningGripReview. Installed057 unchanged, no process left live. Next visible work includes proper steering of handlebars/forks with wheel steering and broader character/environment quality. Full game unfinished.


## Complete visible steering assembly — 2026-09-13 [codex-maclaptop]
BattleBike now groups fork, front wheel, stem and handlebars under SteeringAssembly at the head tube; rotation follows the inclined steerer(-3,0,21) at existing smoothed20degree steering range. Front wheel spin is separate from assembly steering. Mounted hand targets/orientation/finger curl axes follow the actual steering transform rather than fixed rider coordinates. Initial full-lock views exposed unreachable outer-hand targets. A20degree proportional upper-body swivel and8degree proportional additional lean keep the hips over the saddle and preserve arm lengths. Intermediate measured shortfall3.3524cm is retained in steering-reach-short report; final six wrist world-space comparisons across fullleft/straightbraking/fullright have maximum0.111cm error. All three final closeups inspected and copied to work/steering-grip-final. No claim of exact skin contact or complete dynamic wheel-ground/animation acceptance.
Editor build and both native arcade/realistic arrow/WASD/steer/brake/gears/remount/P-toggle checks pass (2026-09-13-steering-assembly-handling.json). Opening grip fixture now exercises steering limits and gates six wrist-reach samples. Unreleased alongside prior grip/posture change; installed057 unchanged. No live process. Full character/environment quality and game scope remain unfinished; larger scenery/character assets remain needed.


## Starting street frontage — 2026-09-13 [codex-maclaptop]
Native survey found terrain protruding up to6.10cm above the old road edge near the bungalow. Added49.97m continuous frontage across first20m of alternate approach and first30m of direct route, with original route XY retained. Three original meshes use existing world-mapped asphalt/concrete materials: crowned4.5m road,18cm curbs,80cm walking strips beyond curb, skirts down into ground. Native terrain samples at101stations fit raised profile; maximum lift30.75cm, central upper envelope limits profile variation to8% before5m end tapers. No Landscape or main-map edits. Source/profile and assets retained in SourceAssets/Terrain/StartingStreet and Content/BattleForTheA/Environment/StartingStreet.
BattleTutorial loads new surfaces at runtime while retaining old road underneath/elsewhere. Bike start traces finished surface before intro to avoid spawning under the raised road. Initial trace failures were three stair overlaps, not missing road: moved fictional bungalow80cm back and added landing.405 central probes now match exported road/sidewalk to within0.00007cm; transition endpoints not included in that assertion. First render exposed inward curb face winding reversed; fixed vertical face winding and added outer skirts. Final opening0/2inspected: opaque continuous curb, clean asphalt edge and steps/sidewalk connection, readable UI. Images work/frontage-final; rejected prior view/report retained. Surface art remains an incremental improvement, not a complete neighborhood.
Editor build succeeds. Native direct tutorial ride7689.6cm/max30.3cmerror and alternate25022.4cm/max28cmerror pass: protected untimed practice, dismount/remount, actualWAD movement, one gate start/countdown. No market closure rerun or whole-world performance claim. Installed057 remains unchanged; frontage and recent grip/steering changes await next package. No live process. Next major visual priority is character/environment fidelity using available assets, beyond this frontage patch.

## Market decision reaffirmed and photo reference — 2026-09-13 [codex-maclaptop]

Elliott reaffirmed the 12th Street entrance market beside Billy’s: block park access during test riding, with market activity extending into adjacent park paths. Farmers Market Vendor zombies must remain park-only (spawn and pursuit); homeless-themed and punk-rocker variants may appear throughout the playable world. Note-only update; no implementation or verification claim. Existing market acceptance records above remain authoritative.

Additional historical photo reference: Ted Eytan, Green Market Piedmont Park Atlanta Georgia 12, May 21, 2011: https://www.flickr.com/photos/taedc/5743665419/ . Also retain Thomas Cizauskas, March 19, 2016: https://www.flickr.com/photos/cizauskas/25815935651 . Official market context: https://piedmontpark.org/green-market/ . Photos are visual references, not imported game assets.

## Detailed Ellison riding prototype — 2026-09-13 [codex-maclaptop]

Added opt-in nonshipping -BattleDetailedRider using installed City Sample m_tal_nrw body, crewneck, jeans, loafers, m_001 face and AfroFade hair cards. Inspector records 150 native bones and +Y forward reference basis. Cycling IK maps pelvis/spine/limbs/fingers to native City bones; detailed wrist target moves from bike-local X26 to31cm and palm alignment derives from reference finger geometry. Default Casual rider remains active until full migration.

Editor build succeeded. Normal protected12s opening rendered; frame0/2 inspected. Initial three-angle closeup failed visually despite six accurate wrist targets: paused intro manual poses updated body without publishing new follower render data, leaving sleeves/face/hair separated in right-turn view. Forcing LOD0 did not solve this and was removed. RefreshRiderPose now calls RefreshFollowerComponents and explicitly updates attached socket transforms; all three repeated steering/braking captures inspected with parts attached. Detailed body/followers retain AlwaysTickPoseAndRefreshBones. This is preview validation, not all gameplay character acceptance; thumbs, moving animation and performance still need review.

Evidence: Tests/Results/2026-09-13-detailed-rider-rig.json, -opening.json, -grip-sync.json. Rejected -grip.json and -grip-lod.json now explicitly passed=false with functional_passed=true retained. Six final wrist errors0.0002cm. Images preserved work/detailed-rider-opening and work/detailed-rider-grip-sync. Opening harness fails closed if detailed asset-load log absent. Native default arcade/realistic controls and remount regression pass in -legacy-handling.json. No installed update;057 unchanged.

Next: detailed crash physics/get-up (BattlePlayerCrash currently Hips and Casual PA; BattlePlayerRecoveryBlend hardcodes old fit bones/clips), modular part transfer through recovery, on-foot body/arms continuity, visibility propagation on dismount/first-person/death, then actual moving and performance review before making detailed rider default. Existing City male retargeted get-ups under /Game/BattleRetarget/City/Male may support migration after verification. Full game objective remains unfinished.

## Detailed rider crash/recovery integration — 2026-09-13 [codex-maclaptop]

The opt-in detailed rider now selects its own mesh physics asset, uses pelvis for physical tracking and recovery fit, and selects existing /Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_{F,B,L,R} clips. Legacy Casual physics and scaled get-ups retained. Disabled physics-component post-process animation. AttachDetailedRiderParts transfers existing clothing/face followers and hair socket attachment to physical display, then recovery pose, then back to the hidden bike rider before recovery-component destruction. EndPlay restores attachment before cleanup. Dismount/remount/camera/death reset visibility propagates to children.

Editor build succeeds. Tests/Results/2026-09-13-detailed-crash-initial.json passes actual keyboard car collision, faster on-foot clock, health/ammo preservation, recovery/remount and actor cleanup. Added three in-recovery screenshot captures for detailed preview; -recovery.json passes and three ground/crouching/standing frames plus remounted view inspected. -death.json passes mid-crash death/checkpoint control; -legacy-regression.json passes original Casual collision/recovery/remount after shared changes. These are rendered editor-game tests, not packaged. Installed057 unchanged.

Quality limits: first detailed run fit RMS96.953cm (against car), next35.797cm; death-run candidate104.981cm. No claim of final car-contact transition quality or continuous animation acceptance from these snapshots. Detailed garments are transferred intact in reviewed frames, but garment/face ground clearance still needs broader review. On-foot ABattleRider body/first-person arms remain old meshes, so preview remains opt-in. Next is appearance/animation continuity on foot, including manual draw, run/jump/crouch/swim, followed by broader collision and Mac performance review before promotion. No original scope removed.

## Detailed first-person arms — 2026-09-13 [codex-maclaptop]

Exported installed City body/crewneck to SourceAssets/Rider/DetailedArms and derived skinned hands/sleeves using FBX SDK;19624hand polygons retained,6266sleeve polygons retained/6480torso polygons removed. UVs, normals, skinning and vertex colors retained. Source export explicitly enables vertex colors. Initial NullRHI FBX export hit native MeshObject assertion; observed failed process98804 did not exit on TERM and was terminated with KILL; rendering-enabled commandlet export succeeds. Original saved game assets not changed by export. Imported two new derivative meshes with source skeletons.

Added opt-in detailed FirstPersonArms and sleeve follower, owner-only/noncolliding; sleeve visibility propagates through swimming/death guards. Shared BattleDetailedBone maps legacy control names to native City bones for bike and FPS IK. Existing free-arm swing and deliberate gun draw behavior preserved. On-foot full Body remains Casual pending full migration.

Visual diagnosis: early foot/color/final trials passed controls but hands stayed gray. Preserving vertex colors (including explicit export flag) did not solve appearance. Source skin atlas inspected; guessed MID atlas override also did not help and was removed. Runtime GetMaterial(0) returned WorldGridMaterial. Importer modified struct loop copies without writing slots[i] back; corrected array assignment restores saved material. Explicit source material binding retained for hands and sleeves. Fresh runtime log now reports M_BodySynthesized; no experimental atlas scalar overrides remain. Rejected trial reports are marked passed=false with functional_passed retained; atlas trial shared image paths were overwritten and removed from report rather than misattributed.

Tests/Results/2026-09-13-detailed-arms-verified.json passes hands-free dismount, G draw/holster, protected fire,520cm/s walk,850cm/s run,30.7cm arm swing, jump/crouch clearance/remount. Five captures inspected (idle/run/walk-b/jump/crouch): skin/clothing restored, moving arms visible. Full pistol grip is not accepted from flash-obscured firing snapshot; cuffs, long guns, swimming, complete on-foot body and performance remain. Images preserved work/detailed-arms-verified. Original mode foot regression run recorded separately. Installed057 unchanged; source prototype remains opt-in. Full game unfinished.


## Detailed on-foot body and airborne root fix — 2026-09-13 [codex-maclaptop]

Integrated six dedicated Epic GameAnimationSample clips retargeted to City Ellison: idle/walk/run/crouch idle/crouch locomotion/fall. Scripts/retarget_detailed_locomotion.py creates copies in external staging; copied six assets only after verifying destination skeleton SHA256 matches source. Retarget/copy reports retained. Optional -BattleDetailedRider now initializes City body, four clothing/face followers and hair before the base skeleton cache; default FPS hides the full body from owner while preserving detailed view arms. -BattleFootBodyReview enables a diagnostic third-person camera.

Initial controls passed but rendered jump lost the body, so initial report is rejected visually. Fixed player-only airborne sequence root travel: CharacterMovement owns the jump displacement, while NPC stationary/get-up clips retain authored roots. New native jump has pelvis offset(-20.118,3.514,-3.185)cm from capsule; audit rejects >130cm displacement and now checks the correct native hand bone. Editor compile and detailed third-person, detailed FPS and original-character control audits pass. Reviewed third-person jump/walk/crouch and FPS run/idle/jump/crouch; body/clothes remain attached, FPS has no full-body clipping and detailed running hand/sleeve is visible. Reports Tests/Results/2026-09-13-detailed-foot-{body,root,fps,legacy}.json; images uniquely preserved under work/detailed-foot-body-initial, detailed-foot-root and detailed-foot-fps.

Not final visual acceptance: current 520cm/s normal movement selects running under shared NPC blend thresholds; cadence/reference speeds need player tuning. Need continuous launch/landing blending, unobstructed foot inspection, close weapon grips, swimming and performance validation. Full-body review uses camera-attached FPS weapon and is not weapon-posture acceptance. Installed057 unchanged; preview remains opt-in, not shipped. Full game goal remains unfinished. No live process after three tests.


## Detailed player cadence — 2026-09-13 [codex-maclaptop]

Measured retargeted root travel before removing world displacement: walk169.922cm/s, run386.186cm/s, crouch231.715cm/s. Existing detailed player traveled520/850cm/s using140/350cm/s crowd assumptions, selecting run even during normal movement. Detailed preview now walks200cm/s, sprints520cm/s, crouches150cm/s; walk/run blend spans240–440cm/s. Playback uses each clip's measured accumulated XY root distance per duration, giving grounded walk~1.177x and sprint~1.346x instead of old run2.429x. Clips without travel retain fallback reference speeds. Ordinary NPC/legacy playback thresholds and player speeds unchanged. Empty-hand sway normalized to new detailed walk speed.

Editor compile succeeds. Native rendered detailed control audit passes walk200/run520/hand swing29.7cm, jump/crouch clearance and remount; two walking frames and run frame inspected with distinct poses. Original-character controls regression passes. Reports Tests/Results/2026-09-13-detailed-cadence-{source,body,legacy}.json; body report includes measured clip speeds, preserved captures work/detailed-cadence-body. No continuous animation/foot-slide or broad balance/performance acceptance yet. Slower detailed foot travel intentionally affects timer pressure; full route balance still needs playtesting. Installed057 unchanged; detailed preview remains opt-in. No live process. Full goal unfinished.


## Detailed airborne transition blend — 2026-09-13 [codex-maclaptop]

Detailed player now snapshots the previous local body pose on airborne/grounded state changes and smoothstep-blends it into the new sampled pose over0.18s. Snapshot includes final previous pose, allowing interrupted transitions to start from current appearance. Physics capsule still owns jump travel. Existing NPC/legacy path unchanged. Functional audit now captures first grounded contact and later settled pose; detailed reports require both airborne and landing transitions to complete.

Editor build and native rendered detailed control test pass walk200/run520,29.6cm hand swing, jump/crouch/remount, and both transition completions. Land-contact and land-settle images inspected: body/clothing attached; initial airborne limbs settle into standing. Evidence Tests/Results/2026-09-13-detailed-transition-body.json and uniquely preserved work/detailed-transition-body. This is blend infrastructure, not final impact animation: no dedicated takeoff/landing clip yet, feet obscured by HUD, continuous quality still unaccepted. Suitable Epic landing source clips exist under external GameAnimationSample/Content/Characters/UEFN_Mannequin/Animations/Jump (e.g. M_Neutral_Jump_F_Land_Stand_Light_Rfoot); retarget and review for physical landing weight next. Installed057 unchanged, detailed preview still opt-in. No live process. Full goal remains active.


## Authored stopping/running landing — 2026-09-13 [codex-maclaptop]

Retargeted two Epic landing copies to City Ellison in /Game/BattleRetarget/Ellison/CityLanding, preserving existing assets. Scripts/retarget_detailed_landing.py and profile_detailed_landing.py retain reproduction; retarget/copy reports include paths/hash and identical skeleton check. Body-action playback now accepts optional start/end times; default callers keep full duration and existing blend behavior. Detailed player chooses running landing only with movement input and >240cm/s speed; releasing movement selects stopping landing despite airborne momentum. Crouching/swimming cancels landing overlay; jumping starts the existing fall sequence and clears action.

First stand test correctly failed selection because retained airborne speed selected run. Corrected input-aware selection passed both modes, but images then exposed0.5s airborne lead-in. Native AnimPose foot/root profile confirms rootZ488cm at start,0 at0.5s, pelvis compression after contact. Playback now uses0.5–1.4s stopping and0.5–1.15s running ranges, including timed fades, with existing0.18s state transition blend. Source files remain untrimmed. Rejected selection/timing reports explicitly false with preserved images.

Fresh native rendered stand and held-W/Shift run tests pass clip selection, both transition completions, walk200/run520, hand swing, jump/crouch clearance/remount. Impact recovery frames inspected: lowered pelvis/bent body, clothes attached, no repeated airborne lead-in. Original-character controls regression passes. Tests/Results/2026-09-13-detailed-landing-impact-{stand,run}.json and detailed-landing-legacy.json; unique images work/detailed-landing-impact-stand and -run. Source profile2026-09-13-landing-profile.json. First grounded screenshot still starts from pre-blend falling pose by design. Continuous movement, actual soles hidden by HUD, run phase matching, takeoff, weapon grip/swim/performance remain unaccepted. Installed057 unchanged; detailed character opt-in/uninstalled. No live process. Full game goal active.


## Unobstructed pistol grip and thumb pose — 2026-09-13 [codex-maclaptop]

Separated native foot audit drawn-weapon screenshot from actual firing by0.15s to remove muzzle-flash obstruction. Diagnostic rendered drawn frame temporarily hides HUD, then restores it before firing; normal gameplay HUD unchanged. Initial muzzle-free frame still hid lower grip behind help panel; clean render exposed upright support thumb. Detailed pistol view arms now align thumb_01-to-thumb_03 direction along the pistol's forward direction with a small upward angle, blended by draw/holster weight. Only detailed pistol/non-melee path changed; hand proportions unchanged.

Editor builds and native detailed rendered draw/fire/holster plus walk/sprint/jump/landing/crouch/remount pass. Clean final drawn frame inspected: thumb follows weapon instead of standing upright; body/clothing intact. Reports Tests/Results/2026-09-13-detailed-pistol-{grip,grip-clean,thumb}.json, clean initial report visually rejected, unique captures work/detailed-pistol-grip-initial, detailed-pistol-grip-clean, detailed-pistol-thumb. Resting pistol only: ADS/reload/recoil/long-gun poses and full palm/finger fit remain unaccepted. Free M1911 pack still externally staged at GameAnimationSample/Content/M1911, not migrated; current pistol still existing model. Installed057 unchanged. No live process. Full goal active.


## Aim/reload review and visible gripping hand — 2026-09-13 [codex-maclaptop]

Extended native foot controls audit: clean aimed frame after0.5s RMB, then fire, GiveWeapon(0,1) fixture, actual R reload, midpoint capture, blocked firing during reload, conserved9+1rounds to10/0reserve, and firing back to9 before existing remount assertions. HUD stays hidden only across diagnostic drawn/aimed/reload captures. Original mode passes expanded audit too.

Aimed frame shows sights near center and hands attached. Initial reload render rejected: gun and grip fell mostly below viewport. Detailed reload now raises gun8cm instead of lowering8cm, support-hand drop10cm instead of18cm. This exposed incorrect fixed right-hand orientation; detailed pistol right-hand branch now follows weapon rotation delta transformed into arm-mesh space, blended with draw weight. Fresh native detailed test passes all controls and landing checks; reload midpose inspected with right fingers following tilted grip, rather than projecting sideways. Source normal/legacy pose unchanged; original controls regression passes.

Reports Tests/Results/2026-09-13-detailed-pistol-{aim,reload,reload-visible,reload-hand}.json and pistol-aim-reload-legacy.json; rejected render reports explicitly false with functional_passed=true. Unique captures preserved under corresponding work directories (initial reload in detailed-pistol-reload-initial). Not full reload acceptance: support hand remains partly below frame, actual magazine exchange absent, continuous motion and other weapons unverified. Free M1911 pack remains externally staged, suitable next asset integration. Installed057 unchanged; detailed preview opt-in/uninstalled. No live process. Full game goal active.


## Free M1911 native integration — 2026-09-13 [codex-maclaptop]

Inspected externally staged free Xeradev M1911 pack and migrated28-package dependency closure into Content/M1911, refusing existing files and recording SHA256s. Scripts/inspect_m1911.py records static bounds, native18-bone rig and six material slots. Dependencies self-contained under /Game/M1911; demo map excluded. Provenance remains Design/FAB-ASSET-QUEUE.md. Source rig has Mag bone but no separate slide bone; no slide animation claimed.

Detailed preview now renders rigged M1911 through a poseable child of existing Weapon component, preserving gameplay gun transform/shot plumbing. Source fires toward-X, corrected with180yaw; source geometry is23.27cm long, native scale1. Existing static mesh cleared only for successful detailed pistol initialization. AimZ adjusted to-6.4cm for source sight height. Explicit child visibility follows drawn pistol/non-melee state. Magazine pulls14cm over first35% of reload, holds then reinserts by90%; support-hand target follows magazine world transform. Detailed M1911 reload raised16cm (other detailed weapons retain8cm) to keep withdrawn magazine visible. Source materials all six correctly assigned, not WorldGrid.

Editor build and native detailed draw/aim/reload/walk/sprint/jump/crouch/remount pass. Audit checks magazine below-15cm in componentZ during withdrawal and restored within0.1cm of original-1.6719cm after reload, ammo conservation and firing lockout. Native original-mode regression passes. Initial draw/aim/reload frames inspected; final reload frame shows detailed surfaces and visible magazine in support hand after framing adjustment. Evidence Tests/Results/2026-09-13-m1911-{inspect,copy,first,reload-frame,legacy}.json; unique images work/m1911-first and m1911-reload-frame. Material inspection was rerun in actual game to confirm migrated rig assignments.

Still not full visual acceptance: continuous exchange, exact finger contact, muzzle/FX placement, other weapons, interruptions and broad performance need checks. Same magazine withdraws/reinserts; no discarded-magazine or slide action implemented. Installed057 unchanged; detailed preview opt-in/uninstalled. No live process. Full game goal active.


## M1911 reload interruption checks — 2026-09-13 [codex-maclaptop]

Added optional --interruptions / -BattleReloadInterruptAudit to native foot controls. After baseline reload, give two reserve rounds and interrupt three separate reloads: actual G holster, actual2 weapon switch (then1 back), and MountBike/Dismount. Assertions preserve9magazine+2reserve without completing reload, verify detailed pistol hidden while holstered/shotgun selected, restore Mag componentZ to-1.6719 within0.1cm, and validate fresh rider after remount. Resume existing walk/sprint/jump/crouch/remount test afterward. Editor build and native detailed interruption audit pass; no production fix required. Report Tests/Results/2026-09-13-m1911-interruptions.json, preserved native log work/m1911-interruptions-native.log. Functional test only, no rendered interruption acceptance.

Located older Scripts/validate_swimming.py and swimming-validation.json: legacy PIE PiedmontExplorer API/coordinates, predating current BattleRider detailed body and weapon. Do not treat old pass as current swimming acceptance. Need fresh actual-world shoreline/swim/return check for detailed body, weapon hiding and bike persistence before promoting preview. Installed057 unchanged. No live process. Full goal active.


## Lake entry and remount; rendered lake rejected — 2026-09-13 [codex-maclaptop]
Current bike previously auto-returned to a path after water impact. New normal shoreline entry leaves the bike at the most recent dry bank and possesses a swimmer. Remote/open-water entries without a dry point within 600cm or safe spawn still use legacy recovery; full water requirement remains incomplete. Native detailed-rider real W-input entry, 522.52cm swim, return and remount pass with zero bike drift and 45cm hand stroke. Drawing/firing/remounting blocked in water. Fixed remount overlap query to honor existing bike movement-ignore actors: invisible Lake Clara Meer solid shore was the blocker. Build succeeds.
Rendered review REJECTED: swimming region looks like grass, lake surface absent, HUD still gives walking/draw-gun instructions. Tests/Results/2026-09-13-detailed-swimming-return.json separates functional success from visual rejection. Read-only lake inventory confirms hazard polygon and water spline share XY bounds; lake Z differs by only 1cm. Water mesh component visible but no materials in commandlet inventory, lake static mesh component hidden; diagnose native water rendering/terrain next rather than treating movement test as lake acceptance. No map edits or package installation. Desktop057 unchanged. New Scripts/test_mac_swimming.py and inspect_swim_lake.py preserve reproducible evidence. Legacy controls regression, other banks, full lake circuit, physics variants and broad performance remain pending.


## Visible lake restored and swimming HUD — 2026-09-13 [codex-maclaptop]
Rebuilt native Water render data with rendering enabled in isolated PiedmontSwimLakeReview; actual game screenshot then showed reflective water where previous test showed grass. Repeated same water-only refresh/ticks/rebuild/save on main map after verified backup work/map-backups/pre-water-render-20260913-051948.umap. Main SHA256 5afdef9bc711cd6143a4a4b884387c987b52efbf128c69b00c6e2824bfb9b12f. No guessed surface overlay or terrain replacement. Script rebuild_swim_lake_review.py defaults to isolated map; explicit -BattleRebuildMainLake backs up and updates main.
Main-map native detailed rendered entry/swim529.36cm/return/remount passes with zero bike drift and45cm stroke; inspected swimming image shows water and readable context-specific HUD. Original rider native regression passes491.75cm swim/44.26cm stroke/zero drift. HUD labels swimming, preserves1.25x clock display, explains bike stays at bank, replaces draw/run/crouch instructions with swim and shore-remount controls. Build passes. Tests/Results/2026-09-13-swimming-main-water.json and swimming-original-rider.json. Visual acceptance is limited to water visibility and HUD at one shoreline; full lake circuit, other banks, underwater view, final stroke animation and performance remain unverified. Installed057 unchanged.


## Parked-bike guidance and three shoreline returns — 2026-09-13 [codex-maclaptop]
Watch radar now uses a cyan parked-bike ring and readable distance/cardinal-direction label with dark backing; label follows the player’s own parked bike. Grounded dry-bank capture uses eight120cm water probes to leave room for capsule settling. Remount explicitly clears movement velocity/inputs before walking movement resumes.
Swim audit now selects shoreline edge, holds W normally, releases it and waits for processing, uses actual E remount binding, and checks2s mounted idle with speed<5cm/s. Earlier direct-call same-frame release/remount runs spuriously coasted because queued release had not been processed; these do NOT establish a production input-timing bug. Experimental global input-sampling change was removed. Final native edge1(rendered),45(rendered),90(nullRHI) passes, swim522.82/527.45/517.05cm with zero parked drift and stable keyboard remount. Inspected final marker/water/HUD image. Reports2026-09-13-swim-bike-marker.json,swim-shore45-keyboard.json,swim-shore90-keyboard.json. Arcade and realistic native steering/remount regression passes in2026-09-13-swim-remount-handling.json. No map edits or installation this turn; installed057 unchanged. Full lake circuit, distant exit/walkback, bridge/long airborne entries, final swimming animation and performance remain incomplete.


## Swimming bridge clearance and extended exploration — 2026-09-13 [codex-maclaptop]
New reproducible native exploration route attempts a full inner-shore circuit, alternate-bank exit and outer-shore walkback. Initial standing-height swim capsule stopped after149m at Lake crossing Rails near(-3261,-129). Isolated rendered repro confirms bridge obstruction. APiedmontExplorer now uses35cm swim capsule half-height, compensates body offset to preserve visual position, and restores original standing capsule only after clearance test on exit. Uncrouches before entering swim. Short rendered bridge traversal22.54m plus bank return and real E remount passes; actual body-through-bridge visual frame/first-person camera polish and original-rider regression still pending.
Extended nullRHI run after fix traversed the full lake loop, exited another bank and walked back, but stopped at routepoint325/343 on a mature tree at(-13960,2898) after642.12m total route travel. Bike drift0 and alternate_shore=true. This is an intended obstacle requiring driver detour, not grounds for removing tree collision. Added west-bank detour to route (now345points); rerun remains live at note time in shellsession67511, report target2026-09-13-swim-exploration-walkaround.json/log work/swim-exploration-walkaround.log. Poll existing session before assuming completion or restarting. Old343point fixture preserved as swim-exploration-before-detour.json. Source compilation passes; no packaging/map changes, installed057 unchanged.
Follow-up found: BattleTrouble.cpp stun expiry restores MOVE_Swimming while surface locomotion uses MOVE_Flying; fix and test stunned swimmer recovery. Full exploration return not yet accepted, camera/animation quality and crouched entry/low-ceiling exits still need coverage. References/lake-bridge-visual-reference.md records official Conservancy arched bridge description; current authored linear12cm-crown bridge still needs visual fidelity work.


## Full lake exploration return verified; swimming stun fix — 2026-09-13 [codex-maclaptop]
Prior live session67511 completed successfully. Native nullRHI route345/345points passes67933.34cm travel: full inner-shore circuit, alternate-bank exit, outside-shore walk around retained tree, original-bank E remount and2s stopped idle. Bike drift0,alternate_shore=true. Report2026-09-13-swim-exploration-walkaround.json identifies fixture and one-route scope. No pending exploration process. This is functional acceptance of that circuit, not rendered full-lake/performance/all-banks acceptance.
BattleTrouble restores MOVE_Flying after swimming stun, matching surface locomotion. New optional --taser swim audit applies actual ApplyTaser, checks exact10s penalty, disabled movement/blocked draw/remount during stun, restored swim mode and movement, then shore/E remount. Detailed rendered and original rider tests pass. HUD now shows current stun label/recovery timer; removed obsolete duplicate get-up banner, which was wrong in water. Final tased render inspected in2026-09-13-swim-taser-hud.json; first render rejected. Original on-foot regression passes walking/sprint/jump/crouch clearance/weapon draw/reload/remount after capsule change; native log preserved. Build passes; no live process, map edits or installation. Desktop057 unchanged. Camera/animation polish, crouched water entry/blocked low-ceiling exit and broad gameplay/performance still pending; full goal remains incomplete.


## Deep-water entry without automatic path return — 2026-09-13 [codex-maclaptop]
Removed the old two-second bike/rider path teleport when lake entry has no nearby recorded dry position. EnterLake now prefers a clear recent bank and otherwise searches shoreline/island edges for dry, supported, unblocked bike placement. The swimmer starts with the surface capsule immediately; failed entry attempts pause and retry rather than teleporting the rider to a path. Completely blocked shoreline/retry duration and all airborne entry positions remain unverified.
Native deep-drop fixture clears dry-location history, falls into water and verifies swimmer/bank separation beyond600cm, movement, zero parked drift, return and real E remount. Final report2026-09-13-deep-bank-support.json passes485.39cm swim and625.87cm initial bank distance. Earlier deep-water-entry report also passes. Ordinary shoreline regression passes522.96cm with zero drift; rendered water/HUD inspected. Geography audit now verifies persistent swimmer separation instead of obsolete automatic path return; editor saved-map frame/island test passes. Build succeeds.
Rendered deep-bank view prompted support diagnostics: simple and complex visibility traces agree at center and wheel-offset positions; nominal tire line sits2.0–2.77cm above that ground. These are support diagnostics, not exact wheel-mesh contact or all-slope visual acceptance. Native keyboard car collision/recovery/E remount regression passes, health70/ammo10 retained; no new broad crash-animation acceptance claimed. Reports2026-09-13-shore-entry-regression.json,geography-swimmer-entry.json,crash-after-water-entry.json. No map edits, packaging or installation this turn; desktop057 unchanged. Full requested game, scenery/animation polish, broad balance and performance acceptance remain incomplete.


## Surface-height swimming view and possession reset fix — 2026-09-13 [codex-maclaptop]
Normal first-person swimming now targets an eye28cm above the surface rather than99cm, with visible procedural alternating hand strokes, slower idle sculling, relaxed fingers and palm-plane alignment. Detailed review camera remains separate. Stunned first-person swim phase pauses. Swimming HUD uses a compact single-line controls strip and no gun crosshair; stun recovery retains its instructions. These are procedural poses, not authored swimming clips or final animation acceptance.
New --first-person audit captures stroke phases and treading, checks surface mode/camera height/visible moving hands, released-input idle, return and E remount. Initial rendered idle failures were real despite a separate redundant test key press being removed. Diagnostic showed MOVE_Falling(3), braking0 and200cm/s persistent velocity with released keys/zero acceleration. Engine ACharacter::Restart resets movement during possession. BattleWater now establishes BeginSurfaceSwimming AFTER PC->Possess, preventing that reset from overwriting surface mode. Final rendered stop shows MOVE_Flying(5), braking1200 and speed0. This corrects a hole in previous water-entry acceptance, which checked travel/remount but not idle mode.
Final detailed report2026-09-13-swim-view-possession-fixed.json passes camera28cm, moving hands, released-key stop, fixed bike and remount; treading frame inspected. Original-rig functional report2026-09-13-swim-view-original.json passes but VISUAL ACCEPTANCE REJECTED for exposed rough wrist/cuff geometry. Detailed on-foot regression2026-09-13-foot-after-swim-view.json passes deliberate draw, aim/reload, walking/running/jump/crouch and remount; native log preserved in work/foot-after-swim-view-native.log. First-person taser report2026-09-13-swim-taser-first-person.json passes exact penalty, immobilization, recovery and remount; tased frame inspected. Build passes. Source only, desktop057 unchanged.
Next: fix original FPS arm cuff/skin geometry exposed in swimming; inspect Scripts/extract_fps_arms.cpp and SourceAssets/Rider/FPSArms.fbx, compare original source before changing weights. Do not claim the functional original-rig pass is visual acceptance. Whole-game art, authored animations, all-bank/bridge camera behavior, continuous motion and broad balance/performance remain incomplete.


## Legacy swimming arm edge correction — 2026-09-13 [codex-maclaptop]
FBX SDK boundary inspection changes the diagnosis of the apparent torn cuffs: Casual_Body source is closed; FPSArms retains1140polygons with24open boundary edges around shoulders, minimum36.80cm from either hand bone. No missing wrist faces found. Source FBX unchanged. Reproducible read-only utility Scripts/inspect_arm_boundaries.cpp and report2026-09-13-fps-arm-boundaries.json preserve source hash/bounds.
Legacy first-person swimming rig now eases its root behind the camera to(-24,0,-165), with shorter/narrower camera-space wrist reach appropriate to its arm length. Returns to prior(22,0,-175) off water; detailed rig unchanged. Initial(-12) revision still exposed an edge late in the stroke and was rejected; final late-stroke/treading frames show that cut edge removed. Native final swim/idle/return/E remount passes, eye28cm and hand stroke16.07cm. Report2026-09-13-swim-legacy-reach.json. On-foot original draw/aim/reload/walk/run/jump/crouch/remount regression passes (2026-09-13-foot-legacy-shoulders.json), native log preserved work/foot-legacy-shoulders-native.log. This fixes the observed camera/shoulder artifact, not original blocky hand style or full continuous animation/art acceptance. Build succeeds; no asset/map changes or installation, desktop057 unchanged. Full goal remains incomplete.


## Upgraded Ellison enabled for normal startup — 2026-09-13 [codex-maclaptop]
Detailed City body/clothing/face/hair, FPS arms and M1911 are now the default, including production compilation. BattleLegacyRider explicitly selects the original rig for comparison. Existing BattleDetailedRider remains compatible but no longer required. Detailed arms load only after detailed body initialization succeeds, preventing mixed fallback rigs. M1911 directory is explicitly cooked; City/BattleRetarget are already listed by packaging script. Compact armed-controls strip exposes more weapon view.
Extended inventory runner supports editor/rendered captures and verifies actual loaded assets. Both flagged and unflagged runs pass actual crate count20, weapon selection, shotgun/SMG damage, ammo/reloads, death persistence, rifle35degree zoom and possession cycles. Final report2026-09-13-default-character-inventory.json; shotgun/reload frames inspected, weapons/grips remain rough. Frozen T-pose targets are fixture actors with ticking deliberately disabled, not NPC animation acceptance.
Unflagged default foot, opening and swim checks pass; actual loaded detailed body/arms/pistol verified in reports. Opening1 rendered frame inspected; unique captures preserved work/default-character-opening-captures. Explicit legacy foot comparison passes. Reports2026-09-13-default-character-{foot,opening,swim,legacy}.json. Native logs preserved. Build passes. Config candidate version0.58.0-dev; build058 packaging is next, installed057 unchanged. Full game art, long-gun animation, broad balance/performance and packaged default-character acceptance remain unfinished.


## Mac058 packaged; release gates still running — 2026-09-13 [codex-maclaptop]
UAT BuildCookRun succeeds143.60s from compiled source d7db05a. Candidate staged app contains the upgraded default body/arms/M1911; packaged inventory, foot controls, rendered swim/idle/remount, rendered opening, both handling modes and actual car crash/recovery/remount pass. Packaged treading/opening1 and recovery frame inspected; this is narrow functional/visual evidence, not whole-game quality acceptance. Scripts/test_mac_swimming.py now supports packaged executable and sandbox capture copying.
Krog route with crowds, scooter scene and realistic handling is still progressing; at90s it had traveled10484cm on leg0. Its existing process must be observed rather than restarted. Easy/Hard finish tests are queued after it in the same driver. Installed057 has not been moved or replaced. Remaining release steps: require route/finish success, preserve057, finalize/sign058, run installed health/checkpoint check, then update installed manifest. Full requested game remains unfinished.


## Mac058 installed — 2026-09-13 [codex-maclaptop]
Existing route driver completed successfully; no restart. Packaged Krog48576.94cm/2legs,0wipeouts,6787/6787paved samples,500lit/0unlit tunnel samples. Observed max23moving cars and25walking people; forced three-participant scooter scene visited. Easy and Hard finish guards/statistics/audit-save checks pass. These are a selected route and trigger fixtures, not a full free-play run or performance acceptance.
Preserved signed057 at /Volumes/Adam Assets/Unreal/Builds/BattleForTheA/Mac/TheBattleOfATL-build057.app. Finalized and signed058 at TheBattleOfATL.app, desktop link retained. Verified0.58.0 metadata and strict deep signature. Installed health audit passes cardinal phone hunt, recollection, two death resets, health100 and two ordered checkpoints. Installation report2026-09-13-build058-installed.json; native installed-health log preserved. Current installed manifest updated; no pending route/finish/install process.
Next substantive work remains character/combat/world visual polish, especially long-gun presentation, broader scenarios/balance and performance. The full requested game is not complete. Previous source progress entries describe their historical state; their compiled changes are now part of058.
