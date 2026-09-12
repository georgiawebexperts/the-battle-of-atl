"""Render saved instanced canopy review without changing main world."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());out=root/'work/detailed-canopy-review';out.mkdir(parents=True,exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontDetailedCanopyReview')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector())
c=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
t=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',t),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',85)]:c.set_editor_property(prop,value)

views=[('gate-oblique',[-17400,-5200,600],[-14000,-5200,100]),('park-aerial',[-16000,16000,18000],[-5000,-2000,-400]),('lakeshore',[-6000,2000,450],[0,-4000,450])]
# Snap shoreline camera onto a mapped path and native pavement at riding height.
best=None
for path in json.loads((root/'SourceAssets/Terrain/park-path-network.json').read_text())['paths']:
 for a,b in zip(path['points_cm'],path['points_cm'][1:]):
  ax,ay=a[0],-a[1];dx,dy=b[0]-ax,-b[1]-ay;fraction=max(0,min(1,((-6000-ax)*dx+(2000-ay)*dy)/max(1,dx*dx+dy*dy)));x,y=ax+fraction*dx,ay+fraction*dy;d=(x+6000)**2+(y-2000)**2
  if best is None or d<best[0]:best=(d,x,y)
_,x,y=best;hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000),0);assert hit
views[-1]=('lakeshore',[x,y,hit[0].z+170],[0,-4000,hit[0].z+170])
for name,position,target in views:
    pos=unreal.Vector(*position)
    cam.set_actor_location(pos,False,False)
    cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(pos,unreal.Vector(*target)),False)
    for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
    unreal.RenderingLibrary.export_render_target(world,t,str(out),name+'.png')
images=[out/(name+'.png') for name,_,_ in views]
assert all(p.is_file() and p.stat().st_size>1000 for p in images),'Missing rendered images'
(out/'manifest.json').write_text(json.dumps({'images':[str(p) for p in images],'scope':'Static saved instanced canopy, no gameplay or performance acceptance','main_map_changed':False},indent=2)+'\n')
print('DETAILED_CANOPY_RENDER_COMPLETE')
