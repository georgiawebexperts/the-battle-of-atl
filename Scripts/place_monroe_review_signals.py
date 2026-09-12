"""Survey roadside signal supports for both Monroe approaches in the review map."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontMonroeExtendedCrossingReview');unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for existing in ea.get_all_level_actors():
 if existing.actor_has_tag('MonroeSignalReview'):ea.destroy_actor(existing)
gate=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('MonroeCrossingReview'));rows=[]
for car in ea.get_all_level_actors():
 if not car.actor_has_tag('MonroeCarLaneReview'):continue
 points=list(car.get_editor_property('route'));stop=car.get_editor_property('crossings')[0].get_editor_property('stop_distance');distance=0
 for a,b in zip(points,points[1:]):
  length=math.hypot(b.x-a.x,b.y-a.y)
  if distance+length>=stop:
   t=(stop-distance)/length;p=[a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.z+(b.z-a.z)*t];dx,dy=(b.x-a.x)/length,(b.y-a.y)/length;break
  distance+=length
 selected=None;attempts=[]
 for side in [650,750,850,950,1050]:
  for forward in [200,100,0]:
   x,y=p[0]-dy*side+dx*forward,p[1]+dx*side+dy*forward;zs=[]
   for ox,oy in [(0,0),(-20,-20),(20,-20),(-20,20),(20,20)]:
    hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+ox,y+oy,2000),unreal.Vector(x+ox,y+oy,-2000))
    if not hit or not isinstance(hit[1],unreal.Landscape):break
    zs.append(hit[0].z)
   attempts.append({'xy':[x,y],'terrain_samples':len(zs),'height_spread':max(zs)-min(zs) if zs else None})
   if len(zs)==5 and max(zs)-min(zs)<8:selected=[x,y,min(zs)];break
  if selected:break
 assert selected,(car.get_actor_label(),attempts)
 yaw=math.degrees(math.atan2(dy,dx))
 signal=ea.spawn_actor_from_class(unreal.BattleTrafficSignal,unreal.Vector(*selected),unreal.Rotator(yaw=yaw));signal.set_actor_label(car.get_actor_label()+' signal');signal.tags=list(signal.tags)+[unreal.Name('MonroeSignalReview')];signal.set_editor_property('crossing',gate)
 rows.append({'car':car.get_actor_label(),'signal_xyz':selected,'signal_yaw':yaw,'stop_xyz':p,'heading_xy':[dx,dy],'attempts':attempts})
assert len(rows)==2
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-monroe-signal-support.json').write_text(json.dumps({'signals':rows,'main_map_changed':False,'visual_accepted':False},indent=2)+'\n')
