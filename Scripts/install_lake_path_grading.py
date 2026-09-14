"""Promote the verified lake grading review map, retaining the previous main map."""
import unreal,pathlib,json,hashlib,shutil
root=pathlib.Path(unreal.Paths.project_dir()).resolve();main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest()
assert before=='5afdef9bc711cd6143a4a4b884387c987b52efbf128c69b00c6e2824bfb9b12f','Main map changed; rebase candidate before promotion'
manifest=json.loads((root/'SourceAssets/Terrain/LakePathGrading/manifest.json').read_text());candidate=manifest['candidate_sha256']
for name in ['2026-09-13-lake-drive-61853018-arcade-lookahead180-braked-min35000.json','2026-09-13-lake-drive-61853018-real-lookahead260-review.json']:
 r=json.loads((root/'Tests/Results'/name).read_text());assert r['passed'] and r['terrain_candidate_sha256']==candidate,name
visual=json.loads((root/'Tests/Results/2026-09-13-lake-third-patch-visual.json').read_text());assert visual['geometry_passed'] and visual['candidate_sha256']==candidate
for name in ['2026-09-13-lake-path-grading-native.json','2026-09-13-lake-grading-navigation.json']:assert json.loads((root/'Tests/Results'/name).read_text())['passed']
backup=root/'work/map-backups'/('PiedmontWorld-before-lake-'+before[:12]+'.umap');backup.parent.mkdir(parents=True,exist_ok=True)
if not backup.exists():shutil.copy2(main,backup)
assert hashlib.sha256(backup.read_bytes()).hexdigest()==before
exec(compile((root/'Scripts/review_lake_path_grading.py').read_text(),'review_lake_path_grading.py','exec'), {'__name__':'__main__','INSTALL_VERIFIED_LAKE_MAIN':True})
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.WaterBodyLake):assert unreal.PiedmontWorldTools.refresh_water_body(a)
unreal.PiedmontWorldTools.finish_editor_asset_loading();unreal.PiedmontWorldTools.rebuild_water_zones()
for _ in range(60):unreal.PiedmontWorldTools.tick_scene_review()
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
after=hashlib.sha256(main.read_bytes()).hexdigest();assert after!=before
(root/'Tests/Results/2026-09-13-lake-main-promotion.json').write_text(json.dumps({'source_before_sha256':before,'main_after_sha256':after,'terrain_candidate_sha256':candidate,'backup':str(backup),'saved_main_map':True,'desktop_installed':False,'pending':'Verify main-map gameplay and packaged content before desktop update.'},indent=2)+'\n')
