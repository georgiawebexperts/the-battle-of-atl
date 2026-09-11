# Meadow and Oak Hill frisbee groups

2026-09-11 [codex-maclaptop]

Build026 uses the existing difficulty table: two groups on Easy, three on Medium,
five on Hard. Each has two rigged, collidable visitors, a moving disc and a bag of
24 spare discs. An owned launcher receives up to eight reserve rounds per visit,
with an eight-second cooldown and the existing 32-round reserve cap. Supplies
work on bike or foot, respect line of sight, and do not unlock the launcher.
Full ammo, death, countdown, pause and ended runs cannot spend supplies.

The groups alternate throws, then miss every fourth throw onto a nearby path.
After a short reaction, a player walks to retrieve the disc and returns to the
throwing position. Procedural hand targets animate release and catch; existing
original throw/impact sounds have local attenuation. These are social discs,
not the damaging launcher projectiles. No spoken NPC banter is included yet.

Placement uses the Conservancy's map to identify the Meadow east/southeast of
Lake Clara Meer and Oak Hill south of the lake. Reference:
https://piedmontpark.org/wp-content/uploads/2021/06/Map-2021-3.pdf
The five world ESU lawn seed points are (4500,5000), (-6500,8500), (6200,8000),
(-10500,7600), and (4000,1800) cm. These are authored gameplay locations inside
those areas, not surveyed locations of actual frisbee players. Each station is
placed inward from the nearest existing park path, and both player positions
must pass real grass-ground, slope and water exclusion checks.

Native audit exercises actual spawned populations, disc cycles/retrieval,
automatic bike pickup and on-foot collection, supply conservation, capacity,
cooldown, walls, pause, death and countdown. It uses a controlled player placement
and no hostile spawns. Human input, visual animation quality and audio acceptance
remain separate checks. The rest of V3's park life, scenery, and complete game
loop remain unfinished.


The first real grass placement exposed negative-scale landscape capsule failures.
Build026 transposes the retained Krog heightmap and imports a positive-scale,
yaw -90 landscape, preserving its sampled geometry and triangle diagonals.
See GEO-PIPELINE.md and the terrain-sweep report. Dismount now computes the extra
vertical capsule clearance required by the ground normal; flat-ground clearance
remains 90 cm. Retrieving visitors use a short sweep to steer around blocking
pawns. The owned parked bike is ignored by the bag's collection sight trace,
while world walls still block it.

The final group audit also remounts, holds W in fifth gear across the real Meadow,
and requires substantial travel, more than three seconds on grass, speed above
1000 cm/s, upright orientation and no wipeouts. This complements the broad
capsule-sweep comparison and the separate route/water regressions.
