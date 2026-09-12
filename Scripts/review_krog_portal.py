"""Import isolated portal meshes and review transient replacements in the main world."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());source=root/'SourceAssets/Terrain/KrogPortalCandidate';out=root/'work/krog-portal-review';out.mkdir(exist_ok=True)
manifest=json.loads((source/'manifest.json').read_text());meshes={}
for kind in ['Shell','Columns']:
 chunk=next(c for c in manifest['chunks'] if c.get('structure')==kind);name='SM_KrogPortal_'+kind;dest='/Game/BattleForTheA/Environment/KrogPortalCandidate'
 options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
 options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;options.static_mesh_import_data.remove_degenerates=False
 task=unreal.AssetImportTask();task.filename=str(source/chunk['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=options
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name);assert mesh
 box=mesh.get_bounding_box();bounds=[[box.min.x,box.min.y,box.min.z],[box.max.x,box.max.y,box.max.z]];assert max(abs(bounds[i][j]-chunk['bounds_cm'][i][j]) for i in range(2) for j in range(3))<.1
 mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
 settings=mesh.get_editor_property('nanite_settings');settings.enabled=True;settings.position_precision=8;settings.generate_fallback=unreal.NaniteGenerateFallback.ENABLED;settings.fallback_target=unreal.NaniteFallbackTarget.PERCENT_TRIANGLES;settings.fallback_percent_triangles=1;settings.fallback_relative_error=0;mesh.set_editor_property('nanite_settings',settings)
 mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_Concrete'));unreal.EditorAssetLibrary.save_loaded_asset(mesh);meshes[kind]=mesh
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
network=json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text());points=[p for road in network['roads'] for p in road['points_cm']]
def floor(p):
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(p[0],p[1],p[2]+180),unreal.Vector(p[0],p[1],p[2]-250));return hit[0].z if hit else None
before=[floor(p) for p in points]
for kind,mesh in meshes.items():
 actors=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Krog route SM_KrogTunnel_'+kind];assert len(actors)==1
 actors[0].static_mesh_component.set_static_mesh(mesh)
unreal.PiedmontWorldTools.finish_editor_asset_loading();after=[floor(p) for p in points];assert all(a is not None and b is not None and abs(a-b)<.25 for a,b in zip(before,after))
checks=[]
for row in json.loads((root/'Tests/Results/2026-09-12-krog-clearance-survey.json').read_text())['sites']:
 x,y,z=row['sample']['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+1500),unreal.Vector(x,y,z-250));assert hit and 'Shell' not in hit[1].get_actor_label();checks.append({'xy':[x,y],'surface':hit[1].get_actor_label(),'z':hit[0].z})
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',70)]:cap.set_editor_property(prop,value)
x,y,z=network['crossing_xyz'];views=[('approach',[x-600,y-1000,z+180],[x+150,y+800,z+130]),('wide',[x-1500,y-1100,z+1400],[x+100,y+600,z]),('portal',[x+100,y+150,z+170],[x+500,y+1300,z+140])];images=[]
for name,location,target in views:
 loc=unreal.Vector(*location);cam.set_actor_location(loc,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(loc,unreal.Vector(*target)),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png');images.append(str(out/(name+'.png')))
(root/'Tests/Results/2026-09-12-krog-portal-native-review.json').write_text(json.dumps({'main_map_saved':False,'floor_samples_unchanged':len(points),'cleared_roof_sites':checks,'images':images,'visual_review':'pending','scope':'Transient portal replacement; no traffic, vehicle-width traversal or final scenery acceptance'},indent=2)+'\n')
