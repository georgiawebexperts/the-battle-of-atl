Quaternius Ultimate Modular Men, Casual.fbx humanoid rig.
Source: https://quaternius.com/packs/ultimatemodularcharacters.html
License: CC0 (see License.txt).
Downloaded 2026-09-10 for Piedmont Ride, Web Experts.

2026-09-10 [codex-maclaptop]: FPSArms.fbx derives from Casual.fbx using Scripts/extract_fps_arms.cpp and Unreal bundled FBX SDK 2020.2. Retains 1,140 arm/hand polygons and the original skin clusters/control-point indices; removes 2,016 body polygons. Influence ratios are normalized before filtering. Reuses Skin and Purple materials. Same CC0 license.

2026-09-11 [codex-maclaptop]: Animations.fbx downloaded from the same creator pack’s public Separate Skeletal Meshes and Animations folder (https://drive.google.com/drive/folders/12xgMMduFpyiYilhyrmrku02CrmRpTE9w), file ID 1jc4udKBV_1VpiSwLbQOT7OJfsNxtCxPD. Creator page https://quaternius.com/packs/ultimatemodularcharacters.html identifies the pack as CC0. Imported 24 authored clips onto Casual_Skeleton; idle, walk and run are integrated in build 029. Runtime translations/scales are retargeted to the existing mesh bind pose. These are stylized authored clips, not motion capture or final GTA-quality assets.

2026-09-11 [codex-maclaptop]: Swat.fbx downloaded from the same CC0 Ultimate Modular Men pack, public Individual Characters/FBX folder, file ID 1WPQ4W5iHj_GXuR_DoDEoi3XOkadEMfHO. Creator URL/license above. Full rigged model replaces the rejected primitive police uniform. Imported without animations; existing named-bone locomotion clips drive it. Imported material interfaces were empty, so Scripts/finish_police_materials.py assigns an authored navy/black/visor palette and reuses the existing Skin material. Source geometry remains unmodified.

2026-09-11 [codex-maclaptop]: Police material persistence verified in a fresh editor process. Unreal material-array iteration yielded temporary struct copies; finish_police_materials.py now retains edited structs in a Python list, asserts the actual mesh assignments, explicitly saves the mesh/materials, and enables skeletal-mesh usage. Skin is an existing material instance and inherits its parent usage.
