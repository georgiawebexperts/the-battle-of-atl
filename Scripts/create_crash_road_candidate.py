"""Isolate a double-sided road collision candidate without changing the main mesh."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
source='/Game/BattleForTheA/Environment/TenthStreetGraded/SM_TenthStreet_Road'
dest='/Game/BattleForTheA/Environment/CrashReview/SM_TenthRoadDoubleSided'
mesh=unreal.load_asset(source);assert mesh
assert not unreal.EditorAssetLibrary.does_asset_exist(dest)
before=mesh.get_editor_property('body_setup').get_editor_property('double_sided_geometry')
copy=unreal.EditorAssetLibrary.duplicate_asset(source,dest);assert copy
copy.get_editor_property('body_setup').set_editor_property('double_sided_geometry',True)
assert unreal.EditorAssetLibrary.save_loaded_asset(copy,False)
(root/'Tests/Results/2026-09-12-crash-road-candidate.json').write_text(json.dumps({'source':source,'candidate':dest,'source_double_sided':before,'candidate_double_sided':True,'main_changed':False},indent=2)+'\n')
