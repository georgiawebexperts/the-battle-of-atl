# Fourth Ward Skatepark

2026-09-11 [codex-maclaptop]

Scope: rideable detour after Murder K, off the right of the southbound Eastside Trail near Freedom Parkway. Keep the connected world, add bowls/street ramps, skateboard traffic, sightseers and bonus time. This is a playable adaptation, not a surveyed reconstruction.

## References

- Official Beltline page and aerial: https://beltline.org/parks-trails/thomas-taylor-memorial-skatepark/ . Renamed Thomas Taylor Memorial Skatepark in 2024; keep the familiar Fourth Ward name in game signage.
- Official aerial inspected in browser: https://a-us.storyblok.com/f/1020195/1650x1100/018480fded/4th-ward-skate-park-2023_1650x1100.jpg . Two connected bowl forms, street area, adjoining lawn, overhead lights, Freedom Parkway bridge and trees. Reference only; not bundled as a game asset.
- Conservancy: https://www.h4wpc.org/skate-park/ . Location and relationship to Eastside Trail/Freedom Parkway.
- OSM way 182470291, retained in References/fourth-ward-skatepark.osm. OpenStreetMap contributors, ODbL. Footprint ESU bounds x37859..40760, y73143..74993 game cm. Nearest existing trail sample (41703,74168,475). Coordinates retain the established origin/1:3 geographic scale.

## Build 040 geometry

Original generated concrete surface at (39000,74000,0): two joined bowls, curved launch bank with plateau, lower manual pad and an east-facing entrance. The interior is adapted to bike dimensions. Ground sampling raises the deck enough that bowl surfaces remain above the existing terrain; a grass berm transitions to it. Existing Landscape and trail assets remain intact. Baked mesh collision uses all surface triangles; rendering and native riding must verify that import/cooking retains the surface.

Three single-use +30-second tokens: one in each bowl and one atop the launch bank. Existing +10 airtime rule applies on a qualifying clean landing. Tagged ramp momentum is retained briefly and released at a steep crest to produce real airborne movement. The visible bike eases into the ground/flight pitch; steering lean remains independent. The grassy berm retains grass speed behavior.

Still to add: authored skateboarding poses/boards and moving skater routes, spectators, benches/rail details, coping, finished concrete texture, lighting and more accurate surrounding landmark treatment. Full skatepark/game quality is not complete when the first collision tests pass.

Native validation found an OBJ Y-axis inversion that symmetric bounds had hidden. Corrected export Y/winding; four asymmetric surface probes now agree in editor and cooked game. First successful ride test measured 143.49 cm rise after takeoff, one clean +10 airtime award and single-use +30 bowl collection. Existing Hard J-hop/guard regression passed. Return testing exposed a berm overlap above the existing trail. Trimmed berm triangles within 230 cm of the retained trail centerline and moved the concrete join to its west edge. Final test now requires the actual BattleEastsideRoute collision actor, not merely a plausible height. Final native test passes actual Eastside Trail return, entry, four surface probes, pedal-only ramp launch, one +10 landing reward and single-use +30 bowl collection. Native aerial/entrance inspection shows a bare but aligned layout; concrete detail, surrounding scene and skatepark life remain unfinished.

Final aerial capture inspected after trail trim. The foot capture has an unsuitable camera angle after the aerial review fixture handoff; it is not accepted as a ground-level screenshot. Dedicated jump/riding animation quality and final art remain open. The new package is a development playtest, not the finished skatepark.
