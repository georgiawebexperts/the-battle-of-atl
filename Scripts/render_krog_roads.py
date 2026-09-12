"""Render the joined Krog/DeKalb road candidate after native support acceptance."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/krog-road-review';out.mkdir(exist_ok=True)
assert json.loads((root/'Tests/Results/2026-09-12-krog-road-support.json').read_text())['passed']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
for material_path in ['/Game/PiedmontRide/Materials/M_ParkAsphaltWorld','/Game/PiedmontRide/Materials/M_ParkConcreteWorld']:
 material=unreal.load_asset(material_path);assert material
 material.set_editor_property('used_with_nanite',True)
 unreal.MaterialEditingLibrary.recompile_material(material)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
network=json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text())
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',70)]:cap.set_editor_property(prop,value)
x,y,z=network['crossing_xyz'];views=[('approach',[x-600,y-1000,z+180],[x+150,y+800,z+130]),('wide',[x-1500,y-1100,z+1400],[x+100,y+600,z]),('portal',[x+100,y+150,z+170],[x+500,y+1300,z+140])];images=[]
for name,location,target in views:
 loc=unreal.Vector(*location);cam.set_actor_location(loc,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(loc,unreal.Vector(*target)),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png');images.append(str(out/(name+'.png')))
(root/'Tests/Results/2026-09-12-krog-road-render.json').write_text(json.dumps({'images':images,'main_map_changed':False,'visual_review':'pending'},indent=2)+'\n')
