"""Import baked pavement, checking world axes before any placement."""
import unreal,json,pathlib,traceback
p=pathlib.Path(unreal.Paths.project_dir());base=p/'SourceAssets/Terrain/ParkPavement';manifest=json.loads((base/'manifest.json').read_text());job=globals().get('WORLD_JOB',{});rows=[]
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);existing={a.get_actor_label():a for a in ea.get_all_level_actors()}
chunks=manifest['chunks'] if job.get('all') else [next(c for c in manifest['chunks'] if c['file']=='Park_Concrete_2_15.obj')]
try:
 for c in chunks:
  name='SM_'+pathlib.Path(c['file']).stem;dest='/Game/PiedmontRide/Environment/Park/Paths'
  options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
  settings=options.static_mesh_import_data;settings.combine_meshes=True;settings.auto_generate_collision=False;settings.remove_degenerates=False
  task=unreal.AssetImportTask();task.filename=str(base/c['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=options
  unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
  mesh=unreal.load_asset(dest+'/'+name)
  if not mesh:raise RuntimeError('No imported mesh '+name)
  box=mesh.get_bounding_box();bounds=[[box.min.x,box.min.y,box.min.z],[box.max.x,box.max.y,box.max.z]]
  error=max(abs(bounds[i][j]-c['bounds_cm'][i][j]) for i in range(2) for j in range(3))
  rows.append({'name':name,'bounds':bounds,'expected':c['bounds_cm'],'axis_error_cm':error,'pass':error<.1})
  if error>=.1:raise RuntimeError('OBJ axis/scale mismatch; placement stopped')
  if job.get('place'):
   body=mesh.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
   editor=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
   nanite=editor.get_nanite_settings(mesh);nanite.set_editor_property('enabled',True);nanite.set_editor_property('position_precision',8);nanite.set_editor_property('generate_fallback',unreal.NaniteGenerateFallback.ENABLED);nanite.set_editor_property('fallback_target',unreal.NaniteFallbackTarget.PERCENT_TRIANGLES);nanite.set_editor_property('fallback_relative_error',0.0);nanite.set_editor_property('fallback_percent_triangles',1.0);editor.set_nanite_settings(mesh,nanite,True)
   material=unreal.load_asset('/Game/PiedmontRide/Materials/M_'+c['material'])
   if not material:
    material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+c['material'],'/Game/PiedmontRide/Materials',unreal.Material,unreal.MaterialFactoryNew())
    color=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant3Vector);color.set_editor_property('constant',unreal.LinearColor(.19,.16,.11));unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
    rough=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.95);unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS);unreal.MaterialEditingLibrary.recompile_material(material);unreal.EditorAssetLibrary.save_loaded_asset(material)
   mesh.set_material(0,material);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
   label='Park pavement '+name
   actor=existing.get(label) or ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
   actor.set_actor_label(label);actor.set_folder_path('Piedmont/Pavement');actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll');actor.tags=[unreal.Name('RideDirt' if c['material']=='Gravel' else 'RidePath')]
   rows[-1]['placed']=True;rows[-1]['collision_complexity']=str(editor.get_collision_complexity(mesh))
  (p/'Scripts/park-pavement-progress.json').write_text(json.dumps({'completed':len(rows),'total':len(chunks),'last':name}))
 if job.get('place'):unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
 result={'status':'passed','scope':'import bounds only','meshes':rows}
except Exception:result={'status':'error','error':traceback.format_exc(),'meshes':rows}
(p/'Scripts/park-pavement-import.json').write_text(json.dumps(result,indent=2))
