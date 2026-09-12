"""Transient gate material identity comparison, without saving world or assets."""
import unreal,json,pathlib,uuid
root=pathlib.Path(unreal.Paths.project_dir()).resolve();candidate='-WeatheredAsphaltReview' in unreal.SystemLibrary.get_command_line();out=root/'work/gate-materials'/uuid.uuid4().hex;out.mkdir(parents=True)
if candidate:exec((root/'Scripts/create_weathered_asphalt.py').read_text())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();rows=[]
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.SkyLight):
  s=a.get_component_by_class(unreal.SkyLightComponent);s.set_editor_property('real_time_capture',False);s.recapture_sky()
 for kind,color in [('Asphalt',(1,0,1)),('Concrete',(0,1,1))]:
  if a.get_actor_label()!=f'Park pavement SM_Park_{kind}_2_16':continue
  c=a.static_mesh_component;m=c.get_material(0)
  row={'actor':a.get_actor_label(),'material':m.get_path_name() if m else None}
  if isinstance(m,unreal.Material):
   for name,prop in [('base',unreal.MaterialProperty.MP_BASE_COLOR),('emissive',unreal.MaterialProperty.MP_EMISSIVE_COLOR),('offset',unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)]:
    node=unreal.MaterialEditingLibrary.get_material_property_input_node(m,prop);row[name]=node.get_class().get_name() if node else None
  rows.append(row)
  if candidate:
   if kind=='Asphalt':c.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWeatheredCandidate'))
   continue
  diag=unreal.AssetToolsHelpers.get_asset_tools().create_asset('GateDiagnostic'+kind+uuid.uuid4().hex,'/Game/BattleForTheA/Review',unreal.Material,unreal.MaterialFactoryNew())
  diag.set_editor_property('used_with_nanite',True)
  diag.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
  node=unreal.MaterialEditingLibrary.create_material_expression(diag,unreal.MaterialExpressionConstant3Vector);node.set_editor_property('constant',unreal.LinearColor(*color));unreal.MaterialEditingLibrary.connect_material_property(node,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR);unreal.MaterialEditingLibrary.recompile_material(diag);c.set_material(0,diag)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector(-18026,-4229,770));cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(cam.get_actor_location(),unreal.Vector(-17126,-4729,240)),False)
cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,val in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',70)]:cap.set_editor_property(prop,val)
for _ in range(32):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
unreal.RenderingLibrary.export_render_target(world,tex,str(out),'identity.png')
(root/'Tests/Results'/('2026-09-12-gate-weathered.json' if candidate else '2026-09-12-gate-materials.json')).write_text(json.dumps({'materials':rows,'image':str(out/'identity.png'),'map_saved':False,'legend':'Weathered lit asphalt candidate; original concrete' if candidate else 'magenta=asphalt, cyan=concrete; transient unlit replacements'},indent=2))
