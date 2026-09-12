# Krog and DeKalb traffic integration

2026-09-12 [codex-maclaptop]

Mapped approach preparation uses retained OpenStreetMap data, shared Krog/DeKalb junction node, current active landscape, and the same one-third geography scale as the game. Network: SourceAssets/Terrain/KrogTraffic/network.json. Seven road fragments,147 centreline samples within4500cm of the junction. Tunnel-tagged road ways are excluded until their floor profile and headroom are matched.

Native survey:625 grid samples plus147 road samples. Grid hits450 terrain,49 route pavement/concrete and126 tunnel-shell surfaces. Road samples hit131 terrain,11 route asphalt and5 tunnel-shell surfaces. Topmost surface minus proposed terrain+14cm ranges from-14.01 to+328.38cm. This does not prove a drivable road: shell roofs can intercept top-down traces.

Next: inspect shell/floor geometry at the five approach samples; derive a road floor profile matching the tunnel entrance, generate junction surfaces and markings, then establish traffic lanes and controlled crossings. Verify bike passage and car support in a review map before installing. Preserve the tunnel sidewalk, shell and98Estoria finish route. Rare fallen scooter/helper scene remains part of the requested final stretch.

No main map or installed054 change in this survey.

2026-09-12 [codex-maclaptop] Layered native traces at five shell-intercepted sites reveal underlying route asphalt/tunnel road/terrain at925–957cm versus roof1267cm. Authored wide roof overlaps mapped DeKalb approach. Candidate portal setback500gamecm clears900cm-wide DeKalb envelope: roof minimum88.24cm, columns221.99cm, zero overlapping source triangles; tunnel road OBJ byte-identical. bake_krog_structure.py now accepts isolated output/setback, default unchanged. Candidate SourceAssets/Terrain/KrogPortalCandidate contains three structure OBJs and manifest (other manifest chunks reference original KrogRoute files). Not imported/rendered/installed; next review collision and entrance appearance, then road joins. Desktop054 unchanged.
