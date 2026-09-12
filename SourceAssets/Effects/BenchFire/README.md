# Bench fire texture sources

2026-09-12 [codex-maclaptop]

Epic sample textures bundled with UE 5.8 NetworkPredictionExtras, Art/Effects/Proto/Shared/Textures/Fire/T_Fire_SubUV and Smoke/T_Smoke_SubUV. Exported from duplicated editor assets to TGA and reimported as standalone project textures. Use within this Unreal game project. No separate purchase was made.

Fire: 6×6 frames; smoke: 8×8 frames. `Scripts/reimport_bench_fire_textures.py` imports these pixels with streaming disabled for the small shared effect textures. The combined reimport/residency change resolved invisible sprite textures in native rendering; individual causes were not isolated.
