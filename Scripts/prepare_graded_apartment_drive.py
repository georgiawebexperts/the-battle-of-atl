"""A second isolated driving fixture across park/bike-lane/road transitions."""
import unreal
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontGradedDrive')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();unreal.PiedmontWorldTools.finish_editor_asset_loading()
route=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) and 'BattleConnector_0' in [str(t) for t in a.tags])
points=[]
for i in range(81):
 x=-17000;y=11400+3100*i/80
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1500),unreal.Vector(x,y,-1500));assert hit
 points.append(hit[0])
route.set_centerline(points);route.set_actor_label('Graded apartment crossing fixture')
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontGradedApartmentDrive')
