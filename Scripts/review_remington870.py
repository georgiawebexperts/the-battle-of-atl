"""Render imported shotgun art in a transient main-world study; never save the map."""
import unreal,pathlib,json,uuid
root=pathlib.Path(unreal.Paths.project_dir()).resolve();out=root/'work/remington870-review'/uuid.uuid4().hex;out.mkdir(parents=True)
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);level.load_level('/Game/PiedmontRide/Maps/PiedmontWorld');world=unreal.EditorLevelLibrary.get_editor_world()
row=json.loads((root/'work/remington870-import.json').read_text())[0];mesh=unreal.load_asset(row['path']);assert mesh
# Hovering prop study, away from terrain, to inspect silhouette and material fidelity.
origin=unreal.Vector(-18000,-4000,1600);actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,origin);actor.static_mesh_component.set_static_mesh(mesh)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,origin);cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for k,v in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',55)]:cap.set_editor_property(k,v)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
images=[]
for name,offset in [('side',(0,-130,10)),('rear',(-90,-65,22)),('front',(90,-65,22))]:
 loc=origin+unreal.Vector(*offset);cam.set_actor_location(loc,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(loc,origin),False)
 for _ in range(32):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png');images.append(str(out/(name+'.png')))
(root/'work/remington870-review.json').write_text(json.dumps({'asset':row,'images':images,'main_map_saved':False,'equipped':False,'visual_accepted':False},indent=2)+'\n')
print('SHOTGUN_REVIEW '+str(out));unreal.SystemLibrary.quit_editor()
