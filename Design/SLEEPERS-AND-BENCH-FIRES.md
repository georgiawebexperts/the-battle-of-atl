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
