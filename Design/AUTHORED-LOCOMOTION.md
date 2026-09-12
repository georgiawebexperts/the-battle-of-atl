# Authored crowd locomotion

2026-09-11 [codex-maclaptop]

Build 029 replaces rigid hip-only leg swinging with imported Quaternius CC0 idle, walk and run rotations. Blend weights and cycle rate follow actual horizontal character speed; transition smoothing is independent of frame rate. Compressed animation samples are cached once per clip at 30 Hz and interpolated each frame across the crowd. First-person weapon arms remain a separate rig.

The animation-only FBX and existing mesh have different bind translations. Preserve the mesh's reference translations and scales while applying the authored rotations. Missing tracks begin from the reference pose. CharacterMovement owns world travel. Walking and idle get a bounded, smoothed support-foot height correction; it fades away during running. Full terrain foot IK, foot locking, jump/fall/swim animations, character art replacement and performance acceptance remain outstanding.

Existing weapon-hand and frisbee throw/catch adjustments overlay the authored pose only when needed. Native frisbee regression is required because disc release uses the animated hand socket. Bike crash/stab/death behavior is not overhauled by importing extra clips.

Run Scripts/test_mac_presentation.py --build 029 --width 1280 --locomotion against the staged Mac app. The controlled close-up character walks, runs and stops while a camera follows. The fixture requires actual travel, grounded recovery, knee articulation and a small idle foot gap. Inspect all four screenshots separately; passing numeric checks alone is not animation-quality acceptance. This fixture is development-only and has no physical user-input or audio acceptance.

The legacy FBX animation importer needs a full editor with Slate; invoking it as a commandlet asserted before saving. Scripts/import_character_animations.py explicitly saves all imported clips because AssetImportTask's returned paths listed only the final sequence. The saved 24 assets are usable; only idle/walk/run are integrated here. Character appearance remains the rejected stylized placeholder.


2026-09-11 [codex-maclaptop]: First packaged City walker review passed structural checks for both variants (four clothing/head parts, head-attached hair, knee motion, forward toes and travel); captures revealed stiff male motion and a visible ground offset, so presentation was rejected despite numeric pass. MTN_N_Walk_InPlace also looks restrained in the native source pose preview. Candidate now selects MTN_N_Walk_F (added via recursive dependency migration) and aligns native mesh origin with CharacterMovement floor impact height instead of the legacy -88 cm offset. Legacy body placement/foot retargeting remains scoped to legacy rigs. Rebuild and fresh rendered acceptance are required; previous results are archived under work/build044-city-review.before-stride-fix.

2026-09-11 [codex-maclaptop]: Rebuild completed (BuildCookRun 133 s). Both native City fixtures exit 0 after forward-clip/floor-origin changes: male knee motion 68.235 degrees versus 22.016 before, idle ball-to-floor proxy .482 cm; female knee motion 56.208 degrees, proxy .826 cm. These are joint proxies, not measured shoe surfaces. Native male walk image shows full stride and arm swing; idle outfits/hair and forward shoes verified for both. Male run screenshot is obscured by other pedestrians, so it cannot support motion acceptance. Bike pedal regression still passes .9994 minimum forward dot, .59 cm maximum toe tilt. Full slope/foot-lock/quick-walk/reaction/performance acceptance is outstanding. No desktop installation changed.
