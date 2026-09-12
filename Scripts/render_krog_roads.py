"""Render the joined Krog/DeKalb road candidate after native support acceptance."""
import unreal,json,pathlib,os
level_floor=os.environ.get('BATTLE_KROG_LEVEL_FLOOR')=='1'
continuous_shell=os.environ.get('BATTLE_KROG_CONTINUOUS_SHELL')=='1'
buildings=os.environ.get('BATTLE_KROG_BUILDINGS')=='1'
rail=os.environ.get('BATTLE_KROG_RAIL')=='1'
assert sum([level_floor,continuous_shell,buildings,rail])<=1
suffix='rail' if rail else 'buildings' if buildings else ('continuous-shell' if continuous_shell else ('level-floor' if level_floor else 'road'))
root=pathlib.Path(unreal.Paths.project_dir());out=root/f'work/krog-{suffix}-review';out.mkdir(exist_ok=True)
assert json.loads((root/'Tests/Results/2026-09-12-krog-road-support.json').read_text())['passed']
map_name='PiedmontKrogRailReview' if rail else 'PiedmontKrogBuildingsReview' if buildings else ('PiedmontKrogShellReview' if continuous_shell else ('PiedmontKrogFloorReview' if level_floor else 'PiedmontKrogRoadReview'))
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/'+map_name)
sky_review=[]
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 if isinstance(actor,unreal.SkyLight):
  sky=actor.get_component_by_class(unreal.SkyLightComponent)
  sky_review.append({'actor':actor.get_actor_label(),'intensity':sky.get_editor_property('intensity'),'original_real_time_capture':sky.get_editor_property('real_time_capture')})
  # Commandlet rendering needs an explicit capture, as in the existing gate
  # and market review scripts. This transient change is never saved to a map.
  sky.set_editor_property('real_time_capture',False);sky.recapture_sky()
for material_path in ['/Game/PiedmontRide/Materials/M_ParkAsphaltWorld','/Game/PiedmontRide/Materials/M_ParkConcreteWorld']:
 material=unreal.load_asset(material_path);assert material
 material.set_editor_property('used_with_nanite',True)
 unreal.MaterialEditingLibrary.recompile_material(material)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
network=json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text())
clearance=[]
manifest=json.loads((root/'SourceAssets/Terrain/KrogPortalCandidate/manifest.json').read_text())
for point in manifest['roof_samples']:
 x,y,z=point[0],-point[1],point[2]
 floor=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+100),unreal.Vector(x,y,z-100));assert floor
 roof=unreal.PiedmontWorldTools.trace_world_surface(floor[0]+unreal.Vector(0,0,10),floor[0]+unreal.Vector(0,0,450));assert roof
 gap=roof[0].z-floor[0].z;clearance.append({'xyz':[x,y,floor[0].z],'headroom_cm':gap})
assert min(p['headroom_cm'] for p in clearance)>220
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',70)]:cap.set_editor_property(prop,value)
x,y,z=network['crossing_xyz'];views=[('approach',[x-600,y-1000,z+180],[x+150,y+800,z+130]),('wide',[x-1500,y-1100,z+1400],[x+100,y+600,z]),('portal',[x+100,y+150,z+170],[x+500,y+1300,z+140])];images=[]
if buildings:views.extend([('north',[x-600,y+500,z+180],[x-400,y-1500,z+260]),('south',[31500,120000,1250],[33000,121000,1200])])
if rail:views.append(('rail-detail',[30200,116200,1500],[30202,116627,1301]))
for name,location,target in views:
 loc=unreal.Vector(*location);cam.set_actor_location(loc,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(loc,unreal.Vector(*target)),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png');images.append(str(out/(name+'.png')))
(root/'Tests/Results'/f'2026-09-12-krog-{suffix}-render.json').write_text(json.dumps({'images':images,'main_map_changed':False,'transient_skylight_recapture':sky_review,'headroom_samples':clearance,'minimum_headroom_cm':min(p['headroom_cm'] for p in clearance),'visual_review':'pending'},indent=2)+'\n')
