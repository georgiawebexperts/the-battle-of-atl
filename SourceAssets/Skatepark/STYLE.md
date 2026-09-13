# Skatepark surface and guidance

2026-09-13 [codex-maclaptop]

Scripts/style_skatepark.py authors 69 triangles of original off-white bank lines and directional arrows, sampled from the existing Concrete.obj triangulated height with a 1.2cm visual offset. The runtime paint mesh has no collision, navigation effect or shadow. Existing concrete collision geometry is unchanged.

M_SkateConcreteDetailed duplicates the existing project M_ParkConcreteWorld procedural aggregate and slab-joint shader. M_SkatePaint is an original plain rough paint material. No new purchased or third-party assets. Scripts/import_skatepark.py preserves the detailed material on reimport.

This is a playable adaptation, not a faithful architectural reconstruction of the real Fourth Ward skatepark.
