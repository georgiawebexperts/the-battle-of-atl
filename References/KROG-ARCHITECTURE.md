# Krog architecture and railway context

2026-09-12 [codex-maclaptop]

The Krog Codex archive associated with Georgia State University describes Krog as a two-lane underpass below Hulsey Yard and identifies its DeKalb Avenue/Wylie Street connection. Use the architecture and route information as reference; do not copy its art or assume its photographs/scans are licensed game assets.

- Archive article: https://krogcodex.org/a-brief-history-of-the-krog-street-tunnel/
- Archive photo reference: https://krogcodex.org/wp-content/uploads/2023/04/KrogInt4-1024x683.jpg
- Scan index: https://krogcodex.org/3d-scans/ (no reusable asset/license verified)
- Official visitor reference: https://discoveratlanta.com/things-to-do/krog-street-tunnel/

Article text inspected; photo links located and an archive photo opened in a hidden browser tab, but the photograph was not visually inspected in this work session. Do not claim photo-matched dimensions. No remote photos imported into the game.

Railway data fetched from the OpenStreetMap Overpass API on2026-09-12 using bbox33.7500,-84.3700,33.7560,-84.3550 and railway rail/light_rail/subway ways, geometry included. Retained raw response: krog-railways-osm.json. Attribution: OpenStreetMap contributors, ODbL. This is map alignment, not a railway survey or verified current operating status.

Scripts/prepare_krog_rail_context.py projects the retained geometry into the existing one-third-scale world ESU frame. SourceAssets/Terrain/KrogRailContext/network.json contains7 clipped segments,6 crossing the authored tunnel centreline. Crossings are39.0,189.3,427.1,2619.1,2773.7 and2909.7gamecm from the candidate north portal sample. Rail elevations, deck support and terrain integration remain unverified; no tracks installed. Existing corridor reference contains no building ways, so it cannot establish nearby building footprints.
