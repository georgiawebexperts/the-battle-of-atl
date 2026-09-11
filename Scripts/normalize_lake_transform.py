"""Preserve world shoreline while removing the reflected water actor transform."""
import unreal, pathlib, json
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
lakes=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.WaterBodyLake)]
assert len(lakes)==1
lake=lakes[0];sp=lake.get_component_by_class(unreal.WaterSplineComponent)
pts=[sp.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(sp.get_number_of_spline_points())]
original=[(v.x,v.y,v.z) for v in pts]
area=sum(p.x*q.y-q.x*p.y for p,q in zip(pts,pts[1:]+pts[:1]))/2
if area<0:pts.reverse()
lake.set_actor_scale3d(unreal.Vector(1,1,1));sp.clear_spline_points(False)
for v in pts:sp.add_spline_point(v,unreal.SplineCoordinateSpace.WORLD,False)
for i in range(len(pts)):sp.set_spline_point_type(i,unreal.SplinePointType.LINEAR,False)
sp.set_closed_loop(True,True)
# Recovery uses the separate PiedmontWaterHazard and solid shore actors.
# Avoid the Water renderer reusing serialized physics hull vertex ordering.
lake.get_component_by_class(unreal.WaterBodyComponent).set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
assert unreal.PiedmontWorldTools.refresh_water_body(lake)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
unreal.PiedmontWorldTools.rebuild_water_zones()
unreal.PiedmontWorldTools.finish_editor_asset_loading()
now=[sp.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(sp.get_number_of_spline_points())]
error=max(min(((v.x-x)**2+(v.y-y)**2+(v.z-z)**2)**.5 for x,y,z in original) for v in now)
assert len(now)==131 and error<.001
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontWorld')
(root/'work/water-review/normalized.json').write_text(json.dumps({'world_vertex_max_error_cm':error,'points':len(now),'previous_world_signed_area':area,'new_world_signed_area':abs(area),'scale':[1,1,1],'water_body_collision':False,'recovery_owner':'PiedmontWaterHazard and solid shore','map_saved':True},indent=2)+'\n')
