# Leafy grass surface

2026-09-11 [codex-maclaptop]

Source: [Leafy Grass](https://polyhaven.com/a/leafy_grass), Charlotte Baglioni / Poly Haven, CC0. Source URLs and SHA-256 digests are in source.json. The three original 2K JPEG maps are retained unmodified. This is a ground texture source, not a surveyed depiction of Piedmont Park.

The Unreal material samples diffuse, normal and roughness maps at a 200-cm world-space tile size, with a second diffuse sample at 4,700 cm for gentle larger-scale brightness variation. A cool green color multiplier moderates the source's dry yellow/brown appearance under the game lighting. The OpenGL normal map uses Unreal's green-channel flip on import and reduced normal strength. Geometry and collision are not modified.

Run Scripts/run_ground_material_import.py to import/build the material. It copies the import script into a temporary whitespace-free path because the commandlet's script-argument parser mishandles the project's space-containing path. The import preserves old disconnected material nodes: deleting the loaded graph triggered an Unreal 5.8 commandlet assertion. Rebuilding reconnects the outputs to a fresh graph; disconnected nodes do not contribute to the compiled shader. Editor graph cleanup remains possible later in a stable editor session.

This adds surface detail. It does not add grass blades, path-edge blending, biome painting, building coverage, better terrain topology or a complete landscape art pass. Native rendered review is required after changing material parameters.
