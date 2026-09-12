"""Native road review; explicitly install with -BattleInstallTenthStreet."""
import unreal,json,pathlib,sys
root=pathlib.Path(unreal.Paths.project_dir());sys.path.insert(0,str(root/'Scripts'))
from battle_geography import require_converted_world
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();require_converted_world(world);ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
# Replace only actors owned by this road installer, making repeated runs safe.
for actor in ea.get_all_level_actors():
 if 'BattleTenthStreet' in [str(t) for t in actor.tags]:ea.destroy_actor(actor)
graded='-BattleReviewGradedTenth' in unreal.SystemLibrary.get_command_line()
assert not (graded and '-BattleInstallTenthStreet' in unreal.SystemLibrary.get_command_line()), 'Review candidate before installing terrain'
if graded:
 from battle_geography import import_source_landscape
 old=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.Landscape))
 old.set_actor_enable_collision(False)
 new=import_source_landscape(root/'SourceAssets/Terrain/atlanta-height-tenth-graded.r16',json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text()))
 assert new
 new.set_editor_property('landscape_material',old.get_editor_property('landscape_material'));new.tags=list(old.tags)
 label=old.get_actor_label();ea.destroy_actor(old);new.set_actor_label(label)
folder=root/'SourceAssets/Terrain'/('TenthStreetGraded' if graded else 'TenthStreet');data=json.loads((folder/'surfaces.json').read_text());dest='/Game/BattleForTheA/Environment/'+('TenthStreetGraded' if graded else 'TenthStreet');rows=[]
markings=folder/'markings.json'
if markings.exists():data['surfaces']+=json.loads(markings.read_text())['surfaces']
for kind,color in [('WhitePaint',(.8,.8,.76)),('YellowPaint',(.9,.58,.015))]:
 mat=unreal.load_asset(dest+'/M_'+kind)
 if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+kind,dest,unreal.Material,unreal.MaterialFactoryNew())
 lib=unreal.MaterialEditingLibrary;lib.delete_all_material_expressions(mat)
 node=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);node.set_editor_property('constant',unreal.LinearColor(*color));lib.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
 rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.9);lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
for row in data['surfaces']:
 name='SM_'+pathlib.Path(row['file']).stem;opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/row['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 mesh=unreal.load_asset(dest+'/'+name);assert mesh;b=mesh.get_bounding_box();bounds=[[b.min.x,b.min.y,b.min.z],[b.max.x,b.max.y,b.max.z]];error=max(abs(bounds[i][j]-row['bounds_cm'][i][j]) for i in range(2) for j in range(3));assert error<.1
 mesh.set_material(0,unreal.load_asset(dest+'/M_'+name.removeprefix('SM_TenthStreet_') if 'Paint' in name else '/Game/PiedmontRide/Materials/M_Concrete' if any(k in name for k in ['Separator','Sidewalk']) else '/Game/PiedmontRide/Materials/M_Asphalt'));mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label(name);actor.tags=[unreal.Name('BattleTenthStreet')]+([] if 'Paint' in name else [unreal.Name('RideBarrier' if 'Separator' in name else 'RidePath')]);actor.set_folder_path('Midtown/TenthStreet');actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('NoCollision' if 'Paint' in name else 'BlockAll');rows.append({'asset':mesh.get_path_name(),'bounds_error_cm':error})
if graded:
 import runpy
 runpy.run_path(str(root/'Scripts/review_graded_park_connections.py'))
unreal.PiedmontWorldTools.finish_editor_asset_loading()
if graded:runpy.run_path(str(root/'Scripts/validate_graded_road_world.py'))
original_lights=[]
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.DirectionalLight):
  light=a.get_component_by_class(unreal.DirectionalLightComponent);original_lights.append((a,a.get_actor_rotation(),light.get_editor_property('intensity')))
  a.set_actor_rotation(unreal.Rotator(pitch=-35,yaw=-120),False);light.set_intensity(40)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8);cap.texture_target=tex;cap.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR;cap.capture_every_frame=False;cap.capture_on_movement=False;cap.always_persist_rendering_state=True;cap.fov_angle=75
out=root/('work/tenth-street-graded-review' if graded else 'work/tenth-street-review');out.mkdir(parents=True,exist_ok=True)
for name,pos,target in [('apartment-frontage',[-16500,15300,3800],[-19000,12300,350]),('monroe',[14800,14500,3000],[11900,11700,-300])]:
 cam.set_actor_location(unreal.Vector(*pos),False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(unreal.Vector(*pos),unreal.Vector(*target)),False)
 for _ in range(16):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png')
# Review lighting and capture actor must never leak into the playable map.
for light,rotation,intensity in original_lights:
 light.set_actor_rotation(rotation,False);light.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(intensity)
ea.destroy_actor(cam)
owned=[a for a in ea.get_all_level_actors() if 'BattleTenthStreet' in [str(t) for t in a.tags]]
assert len(owned)==len(data['surfaces'])
installed='-BattleInstallTenthStreet' in unreal.SystemLibrary.get_command_line()
if installed:assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(out/'result.json').write_text(json.dumps({'map_saved':installed,'owned_actor_count':len(owned),'meshes':rows,'scope':'Static native import/placement review. Road, cycle track, separator, sidewalks and 10th lane paint present. Traffic, Monroe markings and crossing signals absent. Width and gameplay collision acceptance pending.'},indent=2)+'\n')
