"""Rebuild water render data in an isolated map with rendering enabled."""
import unreal,json,shutil,hashlib,datetime
from pathlib import Path
r=Path(unreal.Paths.project_dir()).resolve();main='-BattleRebuildMainLake' in unreal.SystemLibrary.get_command_line();target='/Game/PiedmontRide/Maps/PiedmontWorld' if main else '/Game/PiedmontRide/Maps/PiedmontSwimLakeReview'
backup=None
if main:
 source=r/'Content/PiedmontRide/Maps/PiedmontWorld.umap';backup=r/'work/map-backups'/('pre-water-render-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S')+'.umap');backup.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,backup);assert hashlib.sha256(source.read_bytes()).digest()==hashlib.sha256(backup.read_bytes()).digest()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if not main:
 assert not unreal.EditorAssetLibrary.does_asset_exist(target),'Review already exists; inspect before replacing'
 assert unreal.EditorLoadingAndSavingUtils.save_map(w,target)
 assert unreal.EditorLoadingAndSavingUtils.load_map(target)
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.WaterBodyLake):assert unreal.PiedmontWorldTools.refresh_water_body(a)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
for i in range(60):unreal.PiedmontWorldTools.tick_scene_review()
unreal.PiedmontWorldTools.rebuild_water_zones()
unreal.PiedmontWorldTools.finish_editor_asset_loading()
for i in range(60):unreal.PiedmontWorldTools.tick_scene_review()
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),target)
(r/('Tests/Results/2026-09-13-water-rebuild-main.json' if main else 'Tests/Results/2026-09-13-water-rebuild.json')).write_text(json.dumps({'map':target,'rendering_enabled':True,'saved':True,'visual_accepted':False,'backup':str(backup) if backup else None})+'\n')
