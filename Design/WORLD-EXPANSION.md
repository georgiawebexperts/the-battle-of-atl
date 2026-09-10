# Piedmont Ride — world expansion

2026-09-10 [codex-maclaptop], based on Elliott's requested additions.
Status: approved feature direction; implementation pending. This document supersedes conflicting behavior in V2-SPEC.md, especially automatic water respawn and the tunnel being only an endpoint. Numeric tuning below is an initial implementation choice, not a user-specified balance value.

## World and progression

One continuous Atlanta world with consistent traffic, terrain, collision, injury, lighting and vehicle rules. Keep the existing timed pickup race as an activity in that world. Swimming and hill exploration are supported, not restricted to the race's direct route. During an active timed run, its timer continues while swimming or walking; free exploration has no race countdown.

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

Occasional fictional gun and knife attackers can emerge from plausible places and threaten the player. This is intermittent danger, not constant combat; some full runs should contain no hostile encounter. No weapon or player combat system is requested.

Use a central encounter director with a shared cooldown and at most one active hostile encounter. Initial tuning target: zero or one hostile encounter in a typical ten-minute outing, with at least five quiet minutes between encounters and a protected start/restart period. Use discrete eligible encounter opportunities with explicit no-encounter outcomes; do not roll a spawn chance every frame. Values remain data-driven for playtesting.

Attackers spawn out of view at valid locations, then visibly emerge with audio/animation cues. Do not spawn an attacker already intersecting the rider or place unavoidable hits at a blind corner. Gunshots use line-of-sight, a visible aiming delay and collision against cover; shots can miss. A bullet hit is fatal: stop control and the active run, show failure, and restart from the original start with a fresh objective/timer, not the last checkpoint. Preserve saved best scores and settings. Prevent multiple hit callbacks from restarting more than once.

Knife attacks are also rare, use close-range windup and contact checks, and produce blood effects, impact animation and injury. Initial design treats stabbing as survivable injury with a movement/recovery penalty; Elliott specified guaranteed death for bullet hits but did not specify knife lethality. Keep damage configurable. Do not imply ordinary walkers or dog owners are hostile.

Integrate the earlier Hard-only Kroger pursuer into this shared encounter budget rather than stacking it with unrelated attacks. Difficulty can change traffic and challenge, but rarity remains a requirement on every setting. In exploration mode, death restarts the outing; the race-specific pickup/timer reset applies only when racing.

## Horn, lights and environment

Add a bike horn (initial binding H) with audible directional sound and sensible nearby NPC reactions. The horn does not guarantee clearance, stun actors, or remove collisions.

Fit a real headlamp and rear light. Turn lights on automatically in dark areas and the Krog Street Tunnel, using authored darkness zones plus time-of-day state. Add hysteresis so thresholds do not flicker; tunnel lighting activates before visibility drops. Headlamp illuminates the route and obstacles rather than being only an emissive mesh. Return to ambient-light behavior after leaving the tunnel. Avoid relying on unsupported automatic screen-luminance readings.

Trees and park benches must match Piedmont Park's species, forms, materials and placement. Preserve the existing requirement for willow oaks, magnolias/hardwoods and geographically grounded prop placement. Obtain usable visual references before claiming a match; the photos referenced by the original spec have not been attached. Keep trees and benches solid and leave path navigation clearance. Use the real hills instead of flattening the park for prop placement.

## Implementation order and acceptance

1. Revisit the bike laboratory to separate rider/bike and test crash, swimming, bank exits, walking, stationary bike persistence, return and remount. Preserve the passing gear/steer/brake/terrain regressions. Water respawn tests are superseded, not proof of this new behavior.
2. Finish bridges, connectivity and bank access in the real park. Test full lake exploration and riding representative hills; ensure the abandoned bike stays at the original entry point across streaming.
3. Build recognizable trees, benches and landmarks, and extend the BeltLine through a traversable Krog Tunnel. Introduce horn and automatic light components with bike and tunnel integration tests.
4. Build race/free-exploration state management and death/restart semantics, retaining existing race rules where not superseded.
5. Implement ordinary traffic, skaters, owner/leash/dog groups and 50 mph hyperbikes. Then add rare hostile encounters with deterministic test seeds, cooldown/no-spawn tests, cover/miss tests and fatal-hit restart validation.
6. Polish animations, blood/impact effects, swimming cameras, audio and performance. Keep the eventual post-tunnel destination as a separately defined extension.

Acceptance examples: swim a complete lake circuit, exit on a different bank, find the original bike unmoved, walk back and remount; crash at several speeds without mesh penetration; ride uphill and downhill; see a fast hyperbike early enough to evade; encounter a coherent dog-owner leash pair and a skater; sound the horn; enter and exit the tunnel with stable automatic lights; complete several ordinary runs with no attack; survive a configured knife injury; take one bullet hit and restart the run exactly once from its beginning.
