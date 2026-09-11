# Authored crowd locomotion

2026-09-11 [codex-maclaptop]

Build 029 replaces rigid hip-only leg swinging with imported Quaternius CC0 idle, walk and run rotations. Blend weights and cycle rate follow actual horizontal character speed; transition smoothing is independent of frame rate. Compressed animation samples are cached once per clip at 30 Hz and interpolated each frame across the crowd. First-person weapon arms remain a separate rig.

The animation-only FBX and existing mesh have different bind translations. Preserve the mesh's reference translations and scales while applying the authored rotations. Missing tracks begin from the reference pose. CharacterMovement owns world travel. Walking and idle get a bounded, smoothed support-foot height correction; it fades away during running. Full terrain foot IK, foot locking, jump/fall/swim animations, character art replacement and performance acceptance remain outstanding.

Existing weapon-hand and frisbee throw/catch adjustments overlay the authored pose only when needed. Native frisbee regression is required because disc release uses the animated hand socket. Bike crash/stab/death behavior is not overhauled by importing extra clips.

Run Scripts/test_mac_presentation.py --build 029 --width 1280 --locomotion against the staged Mac app. The controlled close-up character walks, runs and stops while a camera follows. The fixture requires actual travel, grounded recovery, knee articulation and a small idle foot gap. Inspect all four screenshots separately; passing numeric checks alone is not animation-quality acceptance. This fixture is development-only and has no physical user-input or audio acceptance.

The legacy FBX animation importer needs a full editor with Slate; invoking it as a commandlet asserted before saving. Scripts/import_character_animations.py explicitly saves all imported clips because AssetImportTask's returned paths listed only the final sequence. The saved 24 assets are usable; only idle/walk/run are integrated here. Character appearance remains the rejected stylized placeholder.
