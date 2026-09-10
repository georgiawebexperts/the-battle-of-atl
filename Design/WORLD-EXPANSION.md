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
