"""Create a review-map seating pad and authored bench anchor beside the Motel."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
install='BattleInstallMotelBench' in unreal.SystemLibrary.get_command_line()
assert not any(a.actor_has_tag('MotelBench') for a in ea.get_all_level_actors()),'Motel bench already installed'
x,y=-17470,11680;samples=[]
for dx in [-80,0,80]:
 for dy in [-130,0,130]:
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+dx,y+dy,5000),unreal.Vector(x+dx,y+dy,-5000));assert hit
  p,a=hit;assert isinstance(a,unreal.Landscape),a.get_actor_label();samples.append([p.x,p.y,p.z])
low=min(p[2] for p in samples);top=max(p[2] for p in samples)+2;depth=top-low+5
assert top-low<=35,(top,low)
mat=unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkConcreteWorld');mesh=unreal.load_asset('/Engine/BasicShapes/Cube');assert mat and mesh
pad=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,top-depth/2));pad.set_actor_label('Fancy Roach Motel seating pad');pad.tags=[unreal.Name('MotelBenchPad')]
pad.static_mesh_component.set_static_mesh(mesh);pad.static_mesh_component.set_material(0,mat);pad.static_mesh_component.set_collision_profile_name('BlockAll');pad.set_actor_scale3d(unreal.Vector(1.6,2.6,depth/100))
anchor=ea.spawn_actor_from_class(unreal.TargetPoint,unreal.Vector(x,y,top),unreal.Rotator(yaw=-90));anchor.set_actor_label('Fancy Roach Motel park bench');anchor.tags=[unreal.Name('AuthoredParkBench'),unreal.Name('MotelBench')]
for a in [pad,anchor]:a.set_folder_path('Midtown/FancyRoachMotel')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
target='/Game/PiedmontRide/Maps/PiedmontWorld' if install else '/Game/PiedmontRide/Maps/PiedmontMotelBenchReview'
assert unreal.EditorLoadingAndSavingUtils.save_map(world,target)
(root/('Tests/Results/2026-09-12-motel-bench-install.json' if install else 'Tests/Results/2026-09-12-motel-bench-candidate.json')).write_text(json.dumps({'anchor_xyz':[x,y,top],'yaw':-90,'pad_size_cm':[160,260,depth],'terrain_samples':samples,'max_step_down_cm':top-low,'saved_map':target,'main_map_changed':install,'runtime_validated':False},indent=2)+'\n')
