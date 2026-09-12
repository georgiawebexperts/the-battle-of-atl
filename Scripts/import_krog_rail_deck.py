"""Import static rail study and verify the existing road remains clear."""
import unreal,json,runpy
from pathlib import Path
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/KrogRailContext';dest='/Game/BattleForTheA/Environment/KrogRail'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogBuildingsReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);unreal.PiedmontWorldTools.finish_editor_asset_loading()
for a in ea.get_all_level_actors():
 if a.actor_has_tag('KrogRailReview'):ea.destroy_actor(a)
colors={'Ballast':(.12,.11,.095),'Retaining':(.24,.23,.21),'Sleepers':(.075,.043,.025),'Steel':(.2,.21,.22)}
for row in json.loads((folder/'deck-manifest.json').read_text())['surfaces']:
 name=row['material'];path=dest+'/M_KrogRail_'+name;mat=unreal.load_asset(path)
 if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_KrogRail_'+name,dest,unreal.Material,unreal.MaterialFactoryNew())
 unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
 c=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,0,0);c.constant=unreal.LinearColor(*colors[name],1);unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 rough=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant,0,100);rough.r=.5 if name=='Steel' else .95;unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
 opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/row['file']);task.destination_path=dest;task.destination_name='SM_KrogRail_'+name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+task.destination_name);assert mesh;mesh.set_material(0,mat)
 mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Krog railway '+name);a.tags=[unreal.Name('KrogRailReview'),unreal.Name('RideBarrier')];a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll')
runpy.run_path(str(root/'Scripts/style_krog_rail.py'))
unreal.PiedmontWorldTools.finish_editor_asset_loading();failures=[];minimum=10000;checked=0
probes=json.loads((root/'SourceAssets/Terrain/KrogTraffic/car-lane-probes.json').read_text())['samples']
for row in probes:
 x,y,z=row['xyz'];start=unreal.Vector(x,y,z+10)
 floor=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+150),unreal.Vector(x,y,z-150))
 if not floor or floor[1].actor_has_tag('KrogRailReview') or abs(floor[0].z-z)>.25:failures.append({'xyz':[x,y,z],'kind':'floor'})
 ceiling=unreal.PiedmontWorldTools.trace_world_surface(start,unreal.Vector(x,y,z+220))
 if ceiling and ceiling[1].actor_has_tag('KrogRailReview'):failures.append({'xyz':[x,y,z],'kind':'rail overhead','gap':ceiling[0].z-z})
 checked+=1
report={'passed':not failures,'road_probes':checked,'minimum_required_overhead_cm':220,'failures':failures,'main_map_changed':False,'scope':'Road floor and overhead reserve only. Full bike route and visual acceptance pending.'}
(root/'Tests/Results/2026-09-12-krog-rail-deck-clearance.json').write_text(json.dumps(report,indent=2)+'\n');assert not failures,len(failures)
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogRailReview')
