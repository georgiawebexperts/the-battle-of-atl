# Geographic source and Unreal world coordinates

2026-09-11 [codex-maclaptop]

The retained DEM, source network JSON points and baked OBJ files use east/north/up centimetres. Their original values are preserved so source geometry does not need repeated lossy conversion. Unreal placement uses east/south/up: north is negative Y. `battle_geography.py` is the placement boundary. Convert geographic positions with `source_to_world`/`source_vector`, headings with `source_yaw_to_world`, and already-baked source geometry with `place_source_geometry`. Do not reflect the local geometry of an ordinary Unreal-authored character or prop; convert only its geographic placement.

The map carries `BattleGeography_ESU_v1` on WorldSettings. `convert_park_coordinates.py` migrates the legacy map exactly once and refuses a second conversion. It recreates Landscape collision, compares 1,511 pre/post surfaces, rebuilds navigation, and saves only after checks. `validate_converted_geography.py` repeats the comparisons after reopening. Ten measured seams can select either pre-existing surface within 2 cm horizontally and under 3.1 cm vertically; these remain explicit in the evidence rather than being discarded.

`terrain-georeference.json` retains the old `unreal_location_cm` / `unreal_scale` fields as source-space baking metadata for compatibility. Actual actor placement is `world_location_cm` / `world_scale`. The R16 south-to-north row order stays unchanged and the Landscape uses negative Y scale. Engine Landscape collision supports mirrored transforms, but a previously cooked landscape needs collision recreation after its determinant changes.

Current landscape, lake, pavement, bridge, spline, connector and start installers use the new placement convention. The V3 park validator now converts its source fixtures too. The complete authored map remains the primary game asset; `build_measured_landscape.py` creates a new map and must not be run casually over it. Older V2 validation reports and source-space draft scripts are historical evidence, not acceptance of the converted game.

Game radar offsets use world east/south coordinates directly for a north-up screen. `prepare_battle_exit.py` generates the native exit header from the converted source endpoint; start JSON retains both source and world placement. Packaged geography tests verify the saved start/exit convention, radar cardinal axes, island exclusion and real timed water return. These checks do not establish rendered appearance, all bridge ride-throughs, 60 FPS or the full game requirements.
