"""Install the sourced lake crossing and preserve separate deck/rail collision."""
import unreal,pathlib,json,traceback
p=pathlib.Path(unreal.Paths.project_dir());collection=globals().get('WORLD_JOB',{}).get('collection','lake');base=p/('SourceAssets/Terrain/WetlandBridges' if collection=='wetlands' else 'SourceAssets/Terrain/LakeBridge');data=json.loads((base/'manifest.json').read_text());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);editor=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem);rows=[]
try:
 if unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()!='PiedmontWorld':raise RuntimeError('Wrong map')
 existing={a.get_actor_label():a for a in ea.get_all_level_actors()}
 for c in data['chunks']:
  name='SM_'+pathlib.Path(c['file']).stem;dest='/Game/PiedmontRide/Environment/Park/Bridges'
  options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False
  task=unreal.AssetImportTask();task.filename=str(base/c['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=options;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name)
  b=mesh.get_bounding_box();bounds=[[b.min.x,b.min.y,b.min.z],[b.max.x,b.max.y,b.max.z]];error=max(abs(bounds[i][j]-c['bounds_cm'][i][j]) for i in range(2) for j in range(3))
  if error>.1:raise RuntimeError('Imported axis mismatch')
  mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
  ns=editor.get_nanite_settings(mesh);ns.enabled=True;ns.set_editor_property('generate_fallback',unreal.NaniteGenerateFallback.ENABLED);ns.set_editor_property('fallback_target',unreal.NaniteFallbackTarget.PERCENT_TRIANGLES);ns.set_editor_property('fallback_percent_triangles',1.0);editor.set_nanite_settings(mesh,ns,True)
  matname='Bridge'+c['kind'];mat=unreal.load_asset('/Game/PiedmontRide/Materials/M_'+matname)
  if not mat:
   mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+matname,'/Game/PiedmontRide/Materials',unreal.Material,unreal.MaterialFactoryNew());color=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);color.set_editor_property('constant',unreal.LinearColor(*{'Deck':(.4,.37,.3),'Wood':(.22,.12,.06),'Rails':(.055,.07,.06)}[c['kind']]));unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR);unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
  mesh.set_material(0,mat);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
  label=c.get('label','Lake crossing '+c['kind']);actor=existing.get(label) or ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label(label);actor.set_folder_path('Piedmont/Bridges/'+('Wetlands' if collection=='wetlands' else 'Lake'));actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll');actor.tags=[unreal.Name('RideBridge'),unreal.Name('RidePath')] if c['kind']!='Rails' else [unreal.Name('RideBarrier')]
  rows.append({'kind':c['kind'],'axis_error_cm':error,'pass':True})
 for actor in ea.get_all_level_actors():
  if isinstance(actor,unreal.PiedmontPathSpline):
   way=actor.get_editor_property('osm_way_id')
   if collection=='wetlands':
    match=next((b for b in data['bridges'] if str(b['osm_id'])==way),None)
    if match:actor.set_centerline([unreal.Vector(*v) for v in match['centerline_cm']])
   elif way in ['102679938','146304988']:actor.set_centerline([unreal.Vector(*v) for v in data['centerline_cm' if way=='102679938' else 'spur_cm']])
 unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level();report={'status':'installed','meshes':rows}
except Exception:report={'status':'error','error':traceback.format_exc(),'meshes':rows}
(p/('Scripts/wetland-bridges-install.json' if collection=='wetlands' else 'Scripts/lake-bridge-install.json')).write_text(json.dumps(report,indent=2))
