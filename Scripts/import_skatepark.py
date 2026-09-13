import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());base=root/'SourceAssets/Skatepark';dest='/Game/BattleForTheA/Skatepark';rows=[]
name='M_SkateConcrete';m=unreal.load_asset(dest+'/M_SkateConcreteDetailed') or unreal.load_asset(dest+'/'+name)
if not m:
 m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew())
 c=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(.36,.35,.32));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 r=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant);r.set_editor_property('r',.88);unreal.MaterialEditingLibrary.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
 m.set_editor_property('two_sided',True);unreal.MaterialEditingLibrary.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
for part in ['Concrete','Berm']:
 options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
 options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;options.static_mesh_import_data.remove_degenerates=False
 t=unreal.AssetImportTask();t.filename=str(base/(part+'.obj'));t.destination_path=dest;t.destination_name='SM_Skate'+part;t.automated=True;t.save=True;t.replace_existing=True;t.options=options;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
 mesh=unreal.load_asset(dest+'/SM_Skate'+part);assert mesh
 body=mesh.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);body.set_editor_property('double_sided_geometry',True)
 mesh.set_material(0,m if part=='Concrete' else unreal.load_asset('/Game/PiedmontRide/Materials/M_Grass'));unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
 b=mesh.get_bounding_box();rows.append({'asset':mesh.get_path_name(),'min':str(b.min),'max':str(b.max),'collision':str(body.get_editor_property('collision_trace_flag'))})
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(39000,74000,0));a.static_mesh_component.set_static_mesh(unreal.load_asset(dest+'/SM_SkateConcrete'));a.static_mesh_component.set_collision_profile_name('BlockAll')
for xyz in [(38050,73650,470),(38050,74400,510),(40100,73500,950),(39000,74000,650)]:
 p=unreal.Vector(*xyz);hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,300),p-unreal.Vector(0,0,150));assert hit and hit[1]==a and abs(hit[0].z-p.z)<4,('Wrong imported surface',xyz,str(hit))
rows.append({'surface_probes':4,'asymmetric_axis_check':'passed'})
(root/'work/skatepark-import.json').write_text(json.dumps(rows,indent=2)+'\n');unreal.SystemLibrary.quit_editor()
