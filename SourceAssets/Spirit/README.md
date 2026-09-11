# Spirit scene source assets

2026-09-11 [codex-maclaptop]

The six OBJ files are original project geometry authored by `Scripts/prepare_spirit_scene.py`: thirteen ivory/lavender flowers, curved stems and pointed leaves, pollen centers, a muted cloth tie and a small shallow stone. Geometry uses centimetres with OBJ Y reflected for Unreal's importer. `Scripts/import_spirit_memorial.py` imports static meshes and creates the six opaque, rough, two-sided materials. The memorial has no lettering or quest marker. It is independent of the encounter actor.

`route.json` derives the approach and chase line from the retained OSM Eastside Trail network and existing Murder K checkpoint, with the retained USGS terrain beneath the memorial. The arrangement's fictional placement is 2,200 game centimetres before the checkpoint (66 real metres at the game's scale), 260 cm to the right of the southbound centerline. This is not a surveyed memorial site. A small base collision protects the flowers without covering the riding line. The authored stone is embedded slightly into the terrain.

The whole spirit presentation remains disabled until the bear mesh, motion, spectral material, audio, route timing and full native chase are implemented and reviewed. These files contain no bear model. Native memorial screenshots and a separate bike traversal verify only the scope reported by their respective tests.
