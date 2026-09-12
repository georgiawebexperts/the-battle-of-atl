"""Import and render landmark; save only with -BattleInstallLandmark."""
import unreal,json,pathlib,sys
root=pathlib.Path(unreal.Paths.project_dir());sys.path.insert(0,str(root/'Scripts'))
from battle_geography import require_converted_world
folder=root/'SourceAssets/Terrain/FancyRoachMotel';data=json.loads((folder/'geometry.json').read_text());dest='/Game/BattleForTheA/Environment/FancyRoachMotel'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();require_converted_world(world);ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any('FancyRoachMotel' in [str(t) for t in a.tags] for a in ea.get_all_level_actors()),'Existing landmark requires explicit update path.'
colors={'Brick':(.32,.105,.05),'Stucco':(.7,.71,.68),'Trim':(.07,.085,.10),'Glass':(.075,.15,.20),'WarmGlass':(.35,.25,.12),'Roof':(.075,.08,.085)}
rows=[]
for row in data['meshes']:
 kind=row['material'];name='SM_'+pathlib.Path(row['file']).stem
 opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
 opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/row['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name);assert mesh
 b=mesh.get_bounding_box();actual=[[b.min.x,b.min.y,b.min.z],[b.max.x,b.max.y,b.max.z]];error=max(abs(actual[i][j]-row['bounds'][i][j]) for i in range(2) for j in range(3));assert error<.1,(name,actual,row['bounds'])
 mat=unreal.load_asset(dest+'/M_'+kind)
 if not mat:
  mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+kind,dest,unreal.Material,unreal.MaterialFactoryNew());lib=unreal.MaterialEditingLibrary
  c=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*colors[kind]));lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
  rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.18 if 'Glass' in kind else .8);lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
  lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
 mesh.set_material(0,mat);mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label('Fancy Roach Motel '+kind);actor.tags=[unreal.Name('FancyRoachMotel')];actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll');rows.append({'asset':mesh.get_path_name(),'axis_error_cm':error})
exec(compile((root/'Scripts/style_fancy_roach_motel.py').read_text(),'style_fancy_roach_motel.py','exec'))
sign=ea.spawn_actor_from_class(unreal.TextRenderActor,unreal.Vector(*data['sign']['position_world_cm']));sign.set_actor_rotation(unreal.Rotator(yaw=data['sign']['yaw']),False);text=sign.get_component_by_class(unreal.TextRenderComponent);text.set_text(data['sign']['text']);text.set_world_size(34);text.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER);text.set_vertical_alignment(unreal.VerticalTextAligment.EVRTA_TEXT_CENTER);text.set_text_render_color(unreal.Color(255,233,188,255));sign.tags=[unreal.Name('FancyRoachMotel')]
unreal.PiedmontWorldTools.finish_editor_asset_loading()
# Transient facade-review lighting; restore before any requested map save.
original_lights=[]
for light in ea.get_all_level_actors():
 if isinstance(light,unreal.DirectionalLight):
  original_lights.append((light,light.get_actor_rotation(),light.get_component_by_class(unreal.DirectionalLightComponent).get_editor_property("intensity")))
  light.set_actor_rotation(unreal.Rotator(pitch=-35,yaw=-120),False)
  light.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(40)
text.set_text_material(unreal.load_asset(dest+'/M_LandmarkLettering'))
out=root/'work/fancy-roach-review';out.mkdir(parents=True,exist_ok=True)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());capture=cam.get_component_by_class(unreal.SceneCaptureComponent2D);texture=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8);capture.texture_target=texture;capture.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR;capture.capture_every_frame=False;capture.capture_on_movement=False;capture.always_persist_rendering_state=True;capture.fov_angle=70
for name,pos,target in [('sign-close',[-18400,13000,720],[-18600,12000,650]),('street',[-18200,14200,600],[-18600,12000,1050]),('aerial',[-16000,14000,4300],[-19000,10900,600])]:
 cam.set_actor_location(unreal.Vector(*pos),False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(unreal.Vector(*pos),unreal.Vector(*target)),False)
 for _ in range(16):unreal.PiedmontWorldTools.tick_scene_review();capture.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,texture,str(out),name+'.png')
installed='-BattleInstallLandmark' in unreal.SystemLibrary.get_command_line()
if installed:
 for light,rotation,intensity in original_lights:
  light.set_actor_rotation(rotation,False);light.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(intensity)
 ea.destroy_actor(cam)
 sign.set_actor_label('The Fancy Roach Motel sign')
 for a in ea.get_all_level_actors():
  if 'FancyRoachMotel' in [str(t) for t in a.tags]:a.set_folder_path('Midtown/FancyRoachMotel')
 assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(out/'result.json').write_text(json.dumps({'map_saved':installed,'meshes':rows,'scope':'Static native Metal preview with diagnostic light restored before map save. Procedural brick/stucco and emissive text verified; entrances, street and bench scene remain incomplete.'},indent=2)+'\n')
