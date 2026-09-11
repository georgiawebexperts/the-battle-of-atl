"""Transient comparison of imported trees in the existing park lighting; never saves."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/epic-tree-review';out.mkdir(parents=True,exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
meshes=json.loads((root/'work/epic-tree-load-audit.json').read_text())['loaded']
actors=[];ground=[]
for i,row in enumerate(meshes):
 x=-17400+i*1300;y=-5200
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000),0);assert hit
 point,_=hit;mesh=unreal.load_asset(row['path']);b=mesh.get_bounds();scale=(650 if i<2 else 220)/row['height_cm']
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,point.z-(b.origin.z-b.box_extent.z)*scale))
 a.static_mesh_component.set_static_mesh(mesh);a.set_actor_scale3d(unreal.Vector(scale,scale,scale));actors.append(a);ground.append(point.z)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());c=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
t=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
c.set_editor_property('texture_target',t);c.set_editor_property('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR);c.set_editor_property('always_persist_rendering_state',True);c.set_editor_property('capture_every_frame',False);c.set_editor_property('capture_on_movement',False);c.set_editor_property('fov_angle',65)
for i,a in enumerate(actors):
 p=a.get_actor_location();height=650 if i<2 else 220;look=p+unreal.Vector(0,0,height*.45);pos=look+unreal.Vector(0,-height*1.8,height*.15)
 cam.set_actor_location(pos,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(pos,look),False)
 for j in range(24):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,t,str(out),'tree-'+str(i)+'.png')
(out/'manifest.json').write_text(json.dumps({'map_saved':False,'scope':'Editor material and silhouette review only; no gameplay or performance acceptance','trees':meshes},indent=2)+'\n')
