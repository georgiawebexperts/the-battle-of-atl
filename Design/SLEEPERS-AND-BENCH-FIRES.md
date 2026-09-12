# Sleeping encounters and bench fires

2026-09-11 [codex-maclaptop]

## User requirements

Keep the punk character. Name the farmer character type **Farmers Market Vendor** (user request); retain the underlying Farmer asset names. Add rough-clothed sleeping characters on the ground and park benches. Occasionally, approaching one wakes them; they briefly chase Ellison, stagger, then fall asleep again. This must be intermittent rather than happening on every approach.

Add a rare scene of a character attempting to ignite a park bench, with no more than one or two burning benches at a time. Represent the MAA apartment complex Elliott identifies under the fictional in-game name **The Fancy Roach Motel**. A nearby bench is a requested candidate fire location. User confirmed MAA Piedmont Park. Official reference: https://www.maac.com/georgia/atlanta/maa-piedmont-park/ — 250 10th Street NE. Reference its large building shape and street frontage. Exact footprint/geographic placement still needs mapping.

## Implementation and acceptance still required

- Authored sleeping, waking, staggering and settling animations with correct ground/bench contact. Avoid the existing whole-body tilt substitute.
- Short, bounded pursuit and long repeat cooldown; occasional activation. Combat interruption, death, run reset and despawn must not leave stuck actors.
- Native Mac fire/smoke and ignition animation, collision and damage behavior consistent with game hazards. No more than two live fire scenes.
- Use actual bench transforms from ABattleParkFurniture; reserve seats and keep sleepers/fire actors clear of normal walking traffic.
- Identify and map the apartment building after clarification; fictional signage must be readable and placed on the correct building.
- Native gameplay and visual checks needed before acceptance. Nothing in this file denotes completed implementation.

## 10th Street traffic and intermittent scooter scene (2026-09-11)

User requests the wide 10th Street park frontage, its bike lane, cars as moving obstacles, and Peachtree Road Race scenery. Model the Monroe trail crossing and crossings approaching Krog/tunnel from map evidence. Add a rare fallen scooter rider with bystanders assisting at the intersection immediately before the tunnel; preserve a navigable avoidance route. These additions are not yet implemented.

Sources: Beltline January 2025 construction update confirms the 10th/Monroe intersection and bike-lane connection (https://beltline.org/blog/atlanta-beltline-design-and-construction-updates-january-2025/). Atlanta Track Club confirms the 2026 finish chute along 10th enters the park near Park Tavern (https://www.atlantatrackclub.org/press-center). Race set dressing and live vehicle gameplay need a coherent fictional event layout, not cars driven through a packed real-race course.

## Sleeping rig inspection — 2026-09-11 [codex-maclaptop]

Native commandlet inspection (`Scripts/audit_sleep_rigs.py`) confirms the imported Mixamo reference has 65 bones and continuous hip → thigh → calf → foot chains. Punk and Farmer each have 62 bones, with Foot_L/Foot_R parented to Root independently of their legs. City Sample male has 150 bones with conventional pelvis/thigh/calf/foot chains. Evidence: `Tests/Results/2026-09-11-sleep-rig-audit.json`.

Use City Sample as the first sleeping retarget development target; preserve Punk's authored locomotion. Direct chain retargeting onto Punk/Farmer is not accepted without a separate solution for their independently animated feet. This inspection does not establish animation compatibility or visual acceptance, and sleeping encounters remain unimplemented.

## Wake clip candidate — 2026-09-11 [codex-maclaptop]

Rendered four sample times for each existing City Sample male F/B/L/R recovery on a transient flat floor. Right-side `/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_R` starts in the same side-lying orientation as the Mixamo sleeping candidate; left-side faces the opposite direction. R start, intermediate and standing frames inspected with outfit/face/hair. Use R for development of the sleep-to-wake blend; this is not continuous transition or contact acceptance. Evidence: `Tests/Results/2026-09-11-sleep-wake-selection.json`. Need explicit pose alignment/blending, clear standing space, bounded pursuit, and authored settle/stagger motion before releasing the encounter.

## Native runtime foundation — 2026-09-11 [codex-maclaptop]

Added opt-in `APiedmontPedestrian::BeginSleeping()` and `WakeFromSleep()` for the male City body. Full-body sequence sampling bypasses locomotion blending, loops sleep, holds the wake endpoint until the state finishes, then restores normal pedestrian AI. Damage and bike impacts clear sleeping state; wake checks standing-capsule overlap. Editor target compiles successfully. No callers or map placements yet: proximity activation, short chase, authored stagger/settle, bench seating, lying collision volume and live transition/interruption acceptance remain required. This is runtime foundation, not the finished encounter.

## Native state checks — 2026-09-12 [codex-maclaptop]

`Scripts/test_native_sleep.py` now passes in the uncooked native game: sleeping head height 8.605 cm above capsule floor, blocked wake under a test roof, wake after removing the roof, completion to standing, repeat sleep, light bike-impact interruption/re-entry guard and death interruption. Initial clearance failure was a forced test spawn inside tutorial `IronAndRoof`; test now chooses a nonoverlapping standing location. Evidence: `Tests/Results/2026-09-12-native-sleep-runtime.json`. Rendered transition quality, ordinary walker regression after floor alignment, lying collision, chase and stagger/settle behavior, placement and packaged checks remain incomplete.

## Bounded chase implementation — 2026-09-12 [codex-maclaptop]

Opt-in `WakeAndChase(Target)` requires a valid nearby target (600 cm), line of sight and standing clearance. After the get-up clip, phase 3 follows navigation at 260 cm/s for at most six seconds, with 900 cm self/1200 cm target distance limits from the sleeping origin. Failed paths, missing targets, swimming, run end, damage and bike impacts cancel behavior and restore ordinary movement speed. Existing sleep/wake/clearance/impact/death native audit still passes after the change. Actual pursuit movement/timeout/target-loss cases are not yet verified. Ending pursuit currently pauses normal AI for two seconds; authored stagger/settle and return to sleep are still pending, so this is not placed in the world.

## Native pursuit check — 2026-09-12 [codex-maclaptop]

`Scripts/test_native_sleep_chase.py` passed: reachable target distance decreased from 364.742 to 77.992 cm, pursuit ended after its configured window with movement stopped and 135 cm/s normal speed restored, and destroying the target during a second pursuit cancelled cleanly. Evidence: `Tests/Results/2026-09-12-native-sleep-chase.json`. This does not verify presentation, distance-limit exits, failed paths, rare activation, stagger/settle or complete encounters; no world deployment.

## Settle runtime candidate — 2026-09-12 [codex-maclaptop]

Added opt-in `BeginSettling()`/phase 4. Checks a 100 cm sphere corridor to the baked landing offset and seven ground samples (near-level ground within 5 cm). Plays StumbleToSleep, rechecks space at the end, then relocates the actor by the baked mesh-local anchor and rotates 36.510763 degrees before switching to normal sleep. Refreshes the pose in that handoff frame. If an obstacle blocks the final handoff, holds the authored down pose and retries; this is not a reserved collision volume. Editor builds pass. Live settling/re-wake/interruptions, continuous collision during the fall and dynamic-obstacle behavior remain unverified. It is not connected automatically to chase completion or placed in the world yet.

## Landing handoff verification — 2026-09-12 [codex-maclaptop]

Isolated native flat-platform test now passes blocked fall start, a newly blocked landing, hold/retry until clearance, landing location, head-position continuity and waking at the new position. Initial handoff jumped 18.8942 cm because the directly imported sleeping clip and evaluated/baked transition yielded different runtime poses. `SleepingBaked` uses the transition's baking path; native handoff head gap is now 0.0041 cm. Basic sleep/wake/impact/death audit also passes with this loop. Evidence: `Tests/Results/2026-09-12-native-sleep-settle.json`. Continuous rendered motion, all-limb contact, real park terrain and complete encounter/chase integration remain pending.

## Connected return-to-sleep path — 2026-09-12 [codex-maclaptop]

Added opt-in `bReturnToSleepAfterChase`. Pursuit termination tries the authored settle sequence; if space is unsuitable, phase 5 navigates back toward SleepOrigin and retries there. Return attempts are limited to ten seconds, then stop and pause ordinary AI if no usable space/path exists. `Scripts/test_native_sleep_cycle.py` passed in the native park world: first pursuit verifies approach/timeout; a second pursuit with return enabled loses its target and reaches sleeping phase again. Distance472.171→77.983cm. Evidence: `Tests/Results/2026-09-12-native-sleep-cycle.json`. This does not establish every timeout/distance-limit/fallback path or rendered quality. Proximity randomness, cooldown, initial placement, rough-clothing variation, bench sleepers and packaging remain pending.

## Occasional activation — 2026-09-12 [codex-maclaptop]

`bAmbientSleeper` now initializes a male City sleeper and enables return-to-sleep. Per approach: enter within450cm, leave beyond750cm to re-arm,18% chance. Successful wake starts90sec cooldown; failed chance/blocked wake starts30sec cooldown. Remaining nearby never rerolls, including after cooldown expiry. Trigger gates tutorial, countdown, run end, dead/swimming sleeper, dead player and vertical separation>140cm; WakeAndChase retains LOS/standing checks. Standalone C++ checks cover entry/exit thresholds, boundary jitter, staying nearby, cooldown/re-entry and disabled eligibility. Native editor build passes41.55sec. Live automatic activation and park placements remain unverified; no ambient sleepers are placed yet.

## Initial ground site review — 2026-09-12 [codex-maclaptop]

Native editor scan tested1,994 path-side samples. One candidate met navigation proximity, level-ground/fall-corridor checks, water exclusion and clearance from all path centerlines: [1517.605213,-18065.288175,-178.127952], yaw180, beside OSM path503237947 part147. Earlier coarse candidate was rejected by stronger path-corridor screening. Created isolated local `/Game/PiedmontRide/Maps/PiedmontSleeperReview` with an ambient sleeper at that site (+90cm capsule height), tagAmbientSleeperReview. Main map unchanged. Scripts and reports reproduce the review map; live initialization/trigger, runtime furniture overlap, visuals and final placement remain pending.
