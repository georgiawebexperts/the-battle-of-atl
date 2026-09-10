# Piedmont Ride measured geography

Author: Web Experts / www.webexperts.com

The Landscape uses public USGS 3DEP 1 m bare-earth elevation tiles from the
GA Statewide 2018 B18 DRRA survey. `terrain-georeference.json` records exact
tile URLs, UTM bounds, units, origin, sample grid and Unreal transform. The
2 m grid is scaled to one third in all three axes. `atlanta-height.r16` is
little-endian uint16, south-to-north rows. Keep this original height source
unchanged when adding road grading later.

Path and boundary data: © OpenStreetMap contributors, ODbL 1.0.
https://www.openstreetmap.org/copyright
Original responses are preserved in References. `park-path-network.json`
contains clipped source bends, resampled centerlines and original OSM IDs.
Road widths remain at a rideable human scale while geographic distances
are compressed. This derivative map data retains the ODbL attribution.

Reproduce with Tools/terrain-venv/bin/python and the scripts in this order:

1. prepare_real_terrain.py (requires network for USGS tile windows)
2. prepare_park_paths.py
3. audit_park_connectivity.py
4. bake_park_pavement.py
5. validate_park_pavement.py

Tools/terrain-requirements.txt records the local Python packages. The venv
is disposable and excluded from Git.

Pavement source meshes are split into Landscape-component-sized chunks and
surface classes. They are clipped to the same i00–i11 triangle diagonal as
Unreal's Landscape, placed 3 cm above the quantized R16 heights, with UVs at
one repeat per 2 game metres. Their OBJ coordinates are world centimetres,
Z up. Verify the OBJ import axis transform before accepting them in Unreal.

These are unfinished world-building sources. Bridges are explicitly deferred
because bare-earth elevation is not a bridge deck. The connectivity audit
contains unresolved gaps; near segments or projected bridge intersections
are not automatically safe routes. Source-mesh checks do not substitute for
Unreal collision checks or riding the full network. Lake exclusion, shoreline
barriers, bridge clearance, path spline actors and route acceptance remain
required before milestone 2 can pass.
