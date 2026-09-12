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
