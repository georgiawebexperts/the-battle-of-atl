# Sleeping encounters and bench fires

2026-09-11 [codex-maclaptop]

## User requirements

Keep the punk character. Add rough-clothed sleeping characters on the ground and park benches. Occasionally, approaching one wakes them; they briefly chase Ellison, stagger, then fall asleep again. This must be intermittent rather than happening on every approach.

Add a rare scene of a character attempting to ignite a park bench, with no more than one or two burning benches at a time. Represent the MAA apartment complex Elliott identifies under the fictional in-game name **The Fancy Roach Motel**. A nearby bench is a requested candidate fire location. User confirmed MAA Piedmont Park. Official reference: https://www.maac.com/georgia/atlanta/maa-piedmont-park/ — 250 10th Street NE. Reference its large building shape and street frontage. Exact footprint/geographic placement still needs mapping.

## Implementation and acceptance still required

- Authored sleeping, waking, staggering and settling animations with correct ground/bench contact. Avoid the existing whole-body tilt substitute.
- Short, bounded pursuit and long repeat cooldown; occasional activation. Combat interruption, death, run reset and despawn must not leave stuck actors.
- Native Mac fire/smoke and ignition animation, collision and damage behavior consistent with game hazards. No more than two live fire scenes.
- Use actual bench transforms from ABattleParkFurniture; reserve seats and keep sleepers/fire actors clear of normal walking traffic.
- Identify and map the apartment building after clarification; fictional signage must be readable and placed on the correct building.
- Native gameplay and visual checks needed before acceptance. Nothing in this file denotes completed implementation.
