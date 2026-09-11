"""V3's isolated playable handling course; prior park/map assets are retained."""
import unreal,json,pathlib,time,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if globals().get('WORLD_JOB',{}).get('action')!='navigation':
 level.new_level('/Game/BattleForTheA/Maps/ArcadeBikeLab')
 w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();w.get_world_settings().set_editor_property('default_game_mode',unreal.BattleLabMode)
 cube=unreal.load_asset('/Engine/BasicShapes/Cube');mats={n:unreal.load_asset('/Game/PiedmontRide/Materials/M_'+n) for n in ['Asphalt','Grass','Concrete','Safety']}
 def box(name,location,scale,material='Concrete',tag='RidePath',pitch=0):
  a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*location),unreal.Rotator(pitch=pitch));a.set_actor_label(name);a.static_mesh_component.set_static_mesh(cube);a.static_mesh_component.set_material(0,mats[material]);a.set_actor_scale3d(unreal.Vector(*scale));a.tags=[unreal.Name(tag)];return a
 box('Arcade grass foundation',(0,0,-60),(260,220,1),'Grass','RideGrass')
 box('Crowd turning plaza',(0,4000,-10),(200,80,.2),'Asphalt')
 for y in [0,-2500,-4500,-6500]:box('Test lane '+str(y),(0,y,-10),(230,12,.2),'Asphalt')
 for x in [-7000,0,7000]:box('Connecting path '+str(x),(x,-1000,-10),(8,130,.2),'Asphalt')
 box('Uphill — never tip',(0,-2500,160),(10,12,.4),pitch=20)
 box('Hilltop',(1000,-2500,335),(10,12,.3))
 box('Downhill — never tip',(2000,-2500,160),(10,12,.4),pitch=-20)
 for i in range(6):box('Stair '+str(i),(-1000+i*160,-4500,(i+1)*18/2),(1.6,12,(i+1)*.18))
 box('Stair landing',(400,-4500,54),(12,12,1.08))
 for i in range(6):box('Descending stair '+str(i),(1080+i*160,-4500,(6-i)*18/2),(1.6,12,(6-i)*.18))
 for i in range(8):box('Root bump '+str(i),(-800+i*200,-6500,10),(.2,12,.2),'Concrete')
 box('Solid wall bounce',(6500,0,140),(.3,12,2.8),'Safety','RideBarrier')
 lake=ea.spawn_actor_from_class(unreal.WaterBodyLake,unreal.Vector(8500,-9000,-5));lake.set_actor_label('Arcade two-second water return');lake.tags=[unreal.Name('RideWater')]
 spline=lake.get_component_by_class(unreal.WaterSplineComponent);spline.clear_spline_points(False)
 for p in [(-1000,-800,0),(-1000,800,0),(1000,800,0),(1000,-800,0)]:spline.add_spline_point(unreal.Vector(*p),unreal.SplineCoordinateSpace.LOCAL,False)
 spline.set_closed_loop(True,True);unreal.PiedmontWorldTools.refresh_water_body(lake)
 hazard=ea.spawn_actor_from_class(unreal.PiedmontWaterHazard,unreal.Vector(8500,-9000,-5));hazard.set_editor_property('polygon',[unreal.Vector(x,y,0) for x,y in [(-1000,-800),(-1000,800),(1000,800),(1000,-800)]])
 # Navigation/return splines use the actual flat test-lane surfaces; elevated tests ride above them.
 for y in [0,1000,3000,5000,7000,-2500,-4500,-6500]:
  a=ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector());a.set_editor_property('osm_way_id','lab-'+str(y));a.set_centerline([unreal.Vector(x,y,3) for x in range(-10000,10001,200)])
 for x in [-7000,0,7000]:
  a=ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector());a.set_editor_property('osm_way_id','lab-link-'+str(x));a.set_centerline([unreal.Vector(x,y,3) for y in range(-6500,7001,200)])
 start=ea.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(-7000,4000,100));start.set_actor_label('V3 handling start')
 sun=ea.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,3000),unreal.Rotator(pitch=-35,yaw=-40));sun.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);sun.light_component.set_editor_property('intensity',4)
 ea.spawn_actor_from_class(unreal.SkyAtmosphere,unreal.Vector());sky=ea.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,1000));sky.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);sky.light_component.set_editor_property('real_time_capture',True)
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
started=time.monotonic();nav_requested=False
def finish(dt):
 global handle,nav_requested
 try:
  if time.monotonic()-started<3:return
  if not nav_requested:
   if not unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(0,0,500),unreal.Vector(14000,12000,2500)):raise RuntimeError('Lab navigation setup failed')
   nav_requested=True;return
  if unreal.PiedmontWorldTools.is_park_navigation_building():return
  unreal.unregister_slate_post_tick_callback(handle);saved=level.save_current_level();(root/'Scripts/arcade-lab-built.json').write_text(json.dumps({'map':w.get_name(),'saved':bool(saved),'status':'built','scope':'V3 handling course. FPS, nitro and complete milestone acceptance pending.'},indent=2))
 except Exception:
  (root/'Scripts/arcade-lab-built.json').write_text(json.dumps({'status':'error','error':traceback.format_exc()},indent=2));unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(finish)
