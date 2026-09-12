"""Native road placement review; leave game map unchanged."""
import unreal,json,pathlib,sys
root=pathlib.Path(unreal.Paths.project_dir());sys.path.insert(0,str(root/'Scripts'))
from battle_geography import require_converted_world
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();require_converted_world(world);ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
folder=root/'SourceAssets/Terrain/TenthStreet';data=json.loads((folder/'surfaces.json').read_text());dest='/Game/BattleForTheA/Environment/TenthStreet';rows=[]
for row in data['surfaces']:
 name='SM_'+pathlib.Path(row['file']).stem;opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/row['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 mesh=unreal.load_asset(dest+'/'+name);assert mesh;b=mesh.get_bounding_box();bounds=[[b.min.x,b.min.y,b.min.z],[b.max.x,b.max.y,b.max.z]];error=max(abs(bounds[i][j]-row['bounds_cm'][i][j]) for i in range(2) for j in range(3));assert error<.1
 mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_Asphalt'));mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label(name);actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll');rows.append({'asset':mesh.get_path_name(),'bounds_error_cm':error})
unreal.PiedmontWorldTools.finish_editor_asset_loading()
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.DirectionalLight):a.set_actor_rotation(unreal.Rotator(pitch=-35,yaw=-120),False);a.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(40)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8);cap.texture_target=tex;cap.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR;cap.capture_every_frame=False;cap.capture_on_movement=False;cap.always_persist_rendering_state=True;cap.fov_angle=75
out=root/'work/tenth-street-review';out.mkdir(parents=True,exist_ok=True)
for name,pos,target in [('apartment-frontage',[-16500,15300,3800],[-19000,12300,350]),('monroe',[14800,14500,3000],[11900,11700,-300])]:
 cam.set_actor_location(unreal.Vector(*pos),False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(unreal.Vector(*pos),unreal.Vector(*target)),False)
 for _ in range(16):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png')
(out/'result.json').write_text(json.dumps({'map_saved':False,'meshes':rows,'scope':'Static native import/placement review. Traffic, paint, sidewalks and crossing signals absent. Width and gameplay collision acceptance pending.'},indent=2)+'\n')
