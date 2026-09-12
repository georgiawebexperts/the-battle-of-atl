"""Isolated car/terrain review; omits other candidate approach-road branches."""
import sys
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/KrogTraffic'
sys.path.insert(0,str(root/'Scripts'))
from battle_geography import import_source_landscape
import json
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogLaneReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
old=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.Landscape)];assert len(old)==1
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
land=import_source_landscape(folder/'atlanta-height-dekalb-crowned-candidate.r16',meta);assert land
land.set_editor_property('landscape_material',old[0].get_editor_property('landscape_material'));land.tags=list(old[0].tags)
label=old[0].get_actor_label();assert ea.destroy_actor(old[0]);land.set_actor_label(label)
assert unreal.PiedmontWorldTools.refresh_landscape_collision(land)
actors=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Krog DeKalb road approaches'];assert len(actors)==1
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False
opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
path='/Game/BattleForTheA/Environment/KrogTraffic';name='SM_DeKalb_Crowned_Road'
task=unreal.AssetImportTask();task.filename=str(folder/'DeKalb_Crowned_Road.obj');task.destination_path=path;task.destination_name=name
task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(path+'/'+name);assert mesh
mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWorld'))
mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
actors[0].static_mesh_component.set_static_mesh(mesh)
actors[0].set_actor_label('DeKalb crowned road review')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontDeKalbCrownedReview')
