"""Live PIE checks for the real park shoreline and nearest-path recovery."""
import unreal,json,pathlib,time,traceback,math
p=pathlib.Path(unreal.Paths.project_dir());editor=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
job=globals().get('WORLD_JOB',{})
if job.get('action')=='stop':
 editor.editor_request_end_play()
else:
 points=json.loads((p/'SourceAssets/Terrain/lake-test-points.json').read_text())
 ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
 starts=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PlayerStart)]
 start=starts[0] if starts else ea.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(*points['safe_start']))
 start.set_actor_label('Development start - pending 12th Street Gate');start.set_actor_location_and_rotation(unreal.Vector(*points['safe_start']),unreal.Rotator(yaw=points['safe_yaw']),False,True);editor.save_current_level()
 cases=[('path_ride',2.3,'safe_start','safe_yaw',150,True),('nearest_path_recovery',7,'water_drop',None,0,False),('solid_shore',3,'shore_start','shore_yaw',500,True),('airborne_water_entry',2.7,'water_drop',None,0,False),('island_footing',1.3,'island_stand',None,0,False)]
 if job.get('recovery_only'):cases=[c for c in cases if c[0]=='nearest_path_recovery']
 report={'status':'running','timing':'Bike simulation seconds; movement caps frame delta at 0.12 s. This does not certify frame rate.','cases':[]};index=-1;active=None;rows=[];elapsed=0.;last=None;started=time.monotonic()
 def tick(delta):
  global index,active,rows,elapsed,last,handle
  try:
   if time.monotonic()-started>240:raise RuntimeError('Park test timed out')
   worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
   if not worlds:return
   w=worlds[0];bike=unreal.GameplayStatics.get_player_pawn(w,0)
   if not isinstance(bike,unreal.PiedmontBike):return
   ride=bike.get_editor_property('ride');now=unreal.GameplayStatics.get_time_seconds(w)
   if last is None:last=now
   elapsed+=min(now-last,.12);last=now
   if active:
    v=bike.get_actor_location();rows.append({'position':[v.x,v.y,v.z],'speed':ride.get_editor_property('speed'),'recovery':ride.get_editor_property('recovery'),'grounded':ride.get_editor_property('grounded'),'grass':ride.get_editor_property('grass'),'reason':str(ride.get_editor_property('last_crash'))})
    if elapsed<active[1]:return
    water=any(r['recovery']>0 and 'Water' in r['reason'] for r in rows);end=rows[-1];name=active[0]
    if name=='path_ride':okay=not any(r['recovery']>0 for r in rows) and math.dist(rows[0]['position'][:2],end['position'][:2])>250 and end['grounded']
    elif name in ['solid_shore','airborne_water_entry']:okay=water
    elif name=='nearest_path_recovery':
     e=end['position'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(*e),unreal.Vector(e[0],e[1],e[2]-200),2.)
     on_path=bool(hit and (hit[1].actor_has_tag('RidePath') or hit[1].actor_has_tag('RideDirt')))
     distance=math.dist(e[:2],points['water_drop'][:2]);previous=math.dist(points['safe_start'][:2],points['water_drop'][:2])
     okay=water and end['recovery']==0 and on_path and distance<previous*.75
     end['recovered_on_path']=on_path;end['distance_from_water_entry_cm']=distance;end['prior_start_distance_cm']=previous
    else:okay=not any(r['recovery']>0 for r in rows) and end['grounded']
    report['cases'].append({'name':name,'pass':okay,'water_crash':water,'start':rows[0],'end':end,'recovery_samples':rows if name=='nearest_path_recovery' else []})
   bike.validation_key('W',False);index+=1
   if index>=len(cases):
    report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';(p/'Scripts/park-ride-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);editor.editor_request_end_play();return
   active=cases[index];bike.reset_ride();bike.set_actor_location_and_rotation(unreal.Vector(*points[active[2]]),unreal.Rotator(yaw=points[active[3]] if active[3] else 0),False,True);ride.set_editor_property('speed',active[4]);ride.set_editor_property('gear',3);ride.set_editor_property('recovery',0);rows=[];elapsed=0
   if active[5]:bike.validation_key('W',True)
   report['active']=active[0];(p/'Scripts/park-ride-validation.json').write_text(json.dumps(report,indent=2))
  except Exception:
   report['status']='error';report['error']=traceback.format_exc();(p/'Scripts/park-ride-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);editor.editor_request_end_play()
 handle=unreal.register_slate_post_tick_callback(tick);editor.editor_request_begin_play()
