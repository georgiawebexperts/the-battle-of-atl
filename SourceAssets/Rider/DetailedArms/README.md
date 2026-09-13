# Detailed first-person arm derivatives

2026-09-13 [codex-maclaptop]

Sources are the installed City Sample Crowd m_tal_nrw_body and m_tal_nrw_crewneck assets, exported without altering the originals. Retain the source pack license for all derivatives. sources.json records exact paths/materials.

Scripts/export_detailed_arms_source.py requires Unreal commandlet rendering (-AllowCommandletRendering -RenderOffscreen); -nullrhi asserts in the FBX exporter. It explicitly exports vertex colors. Scripts/extract_detailed_arms.cpp uses the bundled FBX2020.2 SDK to keep arm-weighted polygons while preserving skinning, normals, UV sets, vertex colors and material indices. Body contains19624 exposed-hand polygons, all retained; shirt retains6266 and removes6480 torso polygons. Body has1UV set/10648source vertex colors; shirt2UV sets/7083colors.

Scripts/import_detailed_arms.py creates /Game/BattleForTheA/Rider/Detailed/SK_DetailedHands and SK_DetailedSleeves with original skeletons/materials. Reimport these owned derivatives only with -RefreshDetailedArms. Struct material slots must be assigned back into the array before setting materials; merely changing loop values leaves WorldGridMaterial. Runtime preview also binds source materials explicitly. No experimental atlas overrides retained.

Native opt-in flag: -BattleDetailedRider. Review: python3 Scripts/test_mac_foot_controls.py --editor --detailed-rider --review --report <filename.json>. Requires visual inspection in addition to functional pass. Full-body on-foot migration and final grip/cuff/performance work remain; not default or installed.
