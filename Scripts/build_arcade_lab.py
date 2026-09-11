"""V3's isolated playable handling course; prior park/map assets are retained."""
import unreal,json,pathlib,time,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if globals().get('WORLD_JOB',{}).get('action') not in ['navigation','curb','water']:
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
if not any(a.get_actor_label()=='Single 25cm curb' for a in ea.get_all_level_actors()):
 curb=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(-2000,0,12.5));curb.set_actor_label('Single 25cm curb');curb.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'));curb.static_mesh_component.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_Concrete'));curb.set_actor_scale3d(unreal.Vector(2,6,.25));curb.tags=[unreal.Name('RidePath')]
if not any(a.get_actor_label()=='Lighting test shelter roof' for a in ea.get_all_level_actors()):
 for name,loc,scale in [('roof',(4000,-6500,410),(10,13,.2)),('north wall',(4000,-7140,205),(10,.2,4.1)),('south wall',(4000,-5860,205),(10,.2,4.1))]:
  a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*loc));a.set_actor_label('Lighting test shelter '+name);a.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'));a.static_mesh_component.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_Concrete'));a.set_actor_scale3d(unreal.Vector(*scale));a.tags=[unreal.Name('RideBarrier')]
 zone=ea.spawn_actor_from_class(unreal.PiedmontDarkZone,unreal.Vector(4000,-6500,180));zone.set_actor_label('Lighting shelter automatic lights');zone.get_editor_property('bounds').set_box_extent(unreal.Vector(500,630,230))
# Use the same explicit Water Body setup as the geographic park. Positive winding
# and a zone/static surface are required for a visible lake, not just a hazard.
lakes=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.WaterBodyLake)]
if lakes:
 lake=lakes[0];spline=lake.get_component_by_class(unreal.WaterSplineComponent);spline.clear_spline_points(False)
 for p in [(-1000,-800,0),(1000,-800,0),(1000,800,0),(-1000,800,0)]:spline.add_spline_point(unreal.Vector(*p),unreal.SplineCoordinateSpace.LOCAL,False)
 for i in range(4):spline.set_spline_point_type(i,unreal.SplinePointType.LINEAR,False)
 spline.set_closed_loop(True,True)
 zones=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.WaterZone)]
 waterzone=zones[0] if zones else ea.spawn_actor_from_class(unreal.WaterZone,unreal.Vector(8500,-9000,-5));waterzone.set_editor_property('zone_extent',unreal.Vector2D(6000,6000))
 component=lake.get_component_by_class(unreal.WaterBodyComponent);component.set_editor_property('affects_landscape',False);component.set_water_zone_override(waterzone)
 component.set_water_material(unreal.load_asset('/Water/Materials/WaterSurface/Water_Material_Lake'));component.set_water_info_material(unreal.load_asset('/Water/Materials/WaterInfo/DrawWaterInfo'));component.set_water_static_mesh_material(unreal.load_asset('/Water/Materials/WaterSurface/LODs/Water_Material_Lake_LOD'));component.set_water_body_static_mesh_enabled(True)
 unreal.PiedmontWorldTools.refresh_water_body(lake)
 # Give the test lake depth rather than leaving an opaque grass slab at its surface.
 old=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Arcade grass foundation']
 if old:
  for label,loc,scale in [('west',(-2750,0,-60),(205,220,1)),('east',(11250,0,-60),(35,220,1)),('north',(8500,1400,-60),(20,192,1)),('south',(8500,-10400,-60),(20,12,1)),('lakebed',(8500,-9000,-350),(20,16,1))]:
   a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*loc));a.set_actor_label('Arcade ground '+label);a.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'));a.static_mesh_component.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_Grass'));a.set_actor_scale3d(unreal.Vector(*scale));a.tags=[unreal.Name('RideGrass')]
  for a in old:ea.destroy_actor(a)

started=time.monotonic();nav_requested=False
def finish(dt):
 global handle,nav_requested
 try:
  if time.monotonic()-started<3:return
  if not nav_requested:
   if not unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(0,0,500),unreal.Vector(14000,12000,2500)):raise RuntimeError('Lab navigation setup failed')
   nav_requested=True;return
  if unreal.PiedmontWorldTools.is_park_navigation_building():return
  unreal.unregister_slate_post_tick_callback(handle);saved=level.save_current_level();(root/'Scripts/arcade-lab-built.json').write_text(json.dumps({'map':w.get_name(),'saved':bool(saved),'status':'built','scope':'V3 development course; full gameplay/presentation acceptance pending.'},indent=2))
 except Exception:
  (root/'Scripts/arcade-lab-built.json').write_text(json.dumps({'status':'error','error':traceback.format_exc()},indent=2));unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(finish)
