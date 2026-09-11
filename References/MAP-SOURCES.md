# Piedmont/BeltLine game map sources

Retrieved 2026-09-10.
- Official park reference: https://piedmontpark.org/maps/
- Official BeltLine map: https://beltline.org/map/
- Actual path/water geometry: OpenStreetMap via Overpass, bbox south33.779 west-84.379 north33.797 east-84.364.
- Attribution: © OpenStreetMap contributors, https://www.openstreetmap.org/copyright (ODbL). Preserve attribution in game and accompanying materials.
- Coordinates JSON uses local centimetres, X east/Y north. Flat ground assumption; no authentic elevation or buildings. This initial area includes the park and adjacent BeltLine, not the whole BeltLine.


2026-09-10 lake crossing reconstruction: OSM ways 102679938 and 146304988. Deck heights are authored from the bank elevations, not bridge survey data. Official location guide identifies the Clara Meer bridge: https://piedmontpark.org/wp-content/uploads/2018/02/Photogenic-Piedmont-Park-white.pdf . Final appearance matching remains pending.

## 14th Street start source — 2026-09-11 [codex-maclaptop]

Official Conservancy entrance map: https://piedmontpark.org/maps/?location=19 and current printable map https://piedmontpark.org/wp-content/uploads/2026/06/2026_PPCPrintableMap.pdf. OpenStreetMap Overpass snapshots fourteenth-street-osm.json and fourteenth-gate-osm.json locate gate node 5674178517 at the eastern 14th Street terminus on path way 61491566. Gate identity is inferred by matching that location to the Conservancy map; no surveyed architectural dimensions are claimed. SourceAssets/Terrain/battle-start.json records the existing path projection and inward heading. Gate model/reference-matching remains unfinished. OSM contributors, ODbL.


## Monroe connector / Eastside source — 2026-09-11 [codex-maclaptop]

The connector follows shared OSM nodes on Northeast Trail way 182302109 and crossing way 1396654821 to node 2396740017 on Eastside way 741964053. The retained broader snapshot eastside-krog-corridor-osm.json covers south 33.751, west -84.371, north 33.783, east -84.359. Attribution: © OpenStreetMap contributors, ODbL. Official Eastside access reference: https://beltline.org/parks-trails/eastside-trail/ ; Northeast endpoint: https://beltline.org/parks-trails/northeast-trail/ . Trail width and pavement lift are authored arcade values. These sources establish alignment/access, not architectural/photo fidelity.
