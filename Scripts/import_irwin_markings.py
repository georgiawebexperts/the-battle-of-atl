"""Import non-colliding paint into the isolated Irwin crossing review."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/IrwinTraffic';dest='/Game/BattleForTheA/Environment/IrwinTraffic'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinCrossingReview');ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in ea.get_all_level_actors():
 if a.actor_has_tag('IrwinPaintReview'):ea.destroy_actor(a)
rows=[]
for kind in ['WhitePaint','YellowPaint']:
 name='SM_Irwin_'+kind;opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/('Irwin_'+kind+'.obj'));task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name);assert mesh
 mat=unreal.load_asset('/Game/BattleForTheA/Environment/TenthStreetGraded/M_'+kind);assert mat;mesh.set_material(0,mat);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Irwin '+kind);a.tags=[unreal.Name('IrwinPaintReview')];a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('NoCollision');rows.append(mesh.get_path_name())
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-irwin-paint-import.json').write_text(json.dumps({'assets':rows,'main_map_changed':False,'visual_accepted':False,'collision':'NoCollision'},indent=2)+'\n')
