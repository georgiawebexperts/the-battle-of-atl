"""In-engine integration tests. Uses actual PlayerController key events, collision and game ticks."""
import unreal,pathlib,json,traceback,time
root=pathlib.Path(unreal.Paths.project_dir())
report={'timing':'Bike simulation seconds; does not certify frame rate','build':'0.3.0-dev','status':'running','cases':[]}
# name, duration, start, gear, initial speed, held keys
cases=[('gear_up',.4,(0,4000,100),3,0,['Up']),('gear_down',.4,(0,4000,100),3,0,['Down']),('handlebar_camera',.4,(0,4000,100),1,0,['Tab']),('chase_camera',.4,(0,4000,100),1,0,['LeftShift']),('low_gear_acceleration',2.5,(0,4000,100),1,0,['W']),
 ('high_gear_acceleration',2.5,(0,4000,100),7,0,['W']),
 ('top_speed',19,(21000,0,100),7,0,['W']),
 ('coasting',3,(21000,0,100),4,800,[]),
 ('braking',2,(21000,0,100),4,800,['SpaceBar']),
 ('grass_speed',14,(21000,3000,100),7,700,['W']),
 ('uphill_ramp',5,(4200,0,100),3,500,['W']),
 ('solid_wall',2.5,(1000,-2000,100),4,700,['W']),
 ('shoreline',2.5,(12000,0,100),4,700,['W']),
 ('lean_crash',2,(0,4000,100),7,1400,['Right']),
 ('recovery',6,(12000,0,100),4,700,['W']),
 ('brake_pressure',.22,(21000,0,100),4,800,['SpaceBar']),
 ('grass_braking',3,(21000,3000,100),4,800,['SpaceBar']),
 ('downhill_coasting',2,(8700,0,582),7,400,[]),
 ('airborne_water_entry',3,(16000,0,600),4,500,[]),
 ('gentle_turn',1,(0,4000,100),3,400,['Right'])]
if globals().get('WORLD_JOB',{}).get('land_only'):
 cases=[c for c in cases if c[0] not in {'shoreline','recovery','airborne_water_entry'}]
 report['scope']='17 retained land/bike cases; water respawn superseded by separate swimming integration'
index=-1;active=None;last_world_time=None;elapsed=0;rows=[];started=time.monotonic()
def snapshot(p,m):
 l=p.get_actor_location()
 f=p.get_editor_property('rider').get_bone_location_by_name('Foot_L',unreal.BoneSpaces.COMPONENT_SPACE)
 return {'brake_pressure':m.get_editor_property('brake_pressure'),'speed':m.get_editor_property('speed'),'gear':m.get_editor_property('gear'),'lean':m.get_editor_property('lean'),'grass':m.get_editor_property('grass'),'grounded':m.get_editor_property('grounded'),'recovery':m.get_editor_property('recovery'),'crashes':m.get_editor_property('crashes'),'reason':str(m.get_editor_property('last_crash')),'location':[l.x,l.y,l.z],'first_person':p.get_editor_property('first_person'),'pedal_foot':[f.x,f.y,f.z]}
def check(name,r):
 peak=max(x['speed'] for x in r);end=r[-1]['speed'];crashed=any(x['recovery']>0 for x in r)
 if name=='brake_pressure':return .4<r[-1]['brake_pressure']<.9 and r[0]['brake_pressure']<.3
 if name=='grass_braking':
  pavement=next(x for x in report['cases'] if x['name']=='braking')
  return end<5 and any(x['grass'] for x in r) and r[-1]['location'][0]-r[0]['location'][0]>(pavement['end']['location'][0]-pavement['start']['location'][0])*1.15
 if name=='downhill_coasting':return peak>500 and r[-1]['location'][2]<r[0]['location'][2]-100 and not crashed
 if name=='airborne_water_entry':return crashed and any('Water' in x['reason'] for x in r)
 if name=='gentle_turn':return not crashed and max(abs(x['lean']) for x in r)>5 and abs(r[-1]['location'][1]-r[0]['location'][1])>50
 if name=='gear_up':return r[-1]['gear']==4
 if name=='gear_down':return r[-1]['gear']==2
 if name=='handlebar_camera':return r[-1]['first_person']
 if name=='chase_camera':return not r[-1]['first_person']
 if name=='low_gear_acceleration':return peak>250 and max(x['pedal_foot'][2] for x in r)-min(x['pedal_foot'][2] for x in r)>15
 if name=='high_gear_acceleration':return 100<peak<260
 if name=='top_speed':return 1300<peak<1500 and not crashed and r[-1]['location'][0]-r[0]['location'][0]>10000
 if name=='coasting':return 0<end<790 and not crashed and r[-1]['location'][0]-r[0]['location'][0]>1500
 if name=='braking':return end<5
 if name=='grass_speed':return any(x['grass'] for x in r) and 650<peak<900
 if name=='uphill_ramp':return max(x['location'][2] for x in r)>180 and not crashed
 if name=='solid_wall':return crashed and any('Impact' in x['reason'] for x in r)
 if name=='shoreline':return crashed and max(x['location'][0] for x in r)<13110
 if name=='lean_crash':return crashed and any('Traction' in x['reason'] for x in r)
 if name=='recovery':return crashed and r[-1]['recovery']==0
 return False
def tick(dt):
 global index,active,last_world_time,elapsed,rows,handle
 try:
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0]
  if unreal.GameplayStatics.get_game_mode(w).get_editor_property('start_countdown')>0:return
  p=unreal.GameplayStatics.get_player_pawn(w,0)
  if not p or 'PiedmontBike' not in p.get_class().get_name():return
  m=p.get_editor_property('ride');now=unreal.GameplayStatics.get_time_seconds(w)
  if last_world_time is None:last_world_time=now
  elapsed+=min(now-last_world_time,.12);last_world_time=now
  if active is not None:
   rows.append(snapshot(p,m))
   if elapsed<active[1]:return
   result={'name':active[0],'pass':check(active[0],rows),'start':rows[0],'end':rows[-1],'peak_speed':max(x['speed'] for x in rows),'max_height':max(x['location'][2] for x in rows),'crash_seen':any(x['recovery']>0 for x in rows)}
   report['cases'].append(result)
  for key in ['W','Left','Right','A','D','SpaceBar','Up','Down','Tab','LeftShift']:p.validation_key(unreal.Name(key),False)
  index+=1
  if index>=len(cases):
   report['status']='passed' if all(x['pass'] for x in report['cases']) else 'failed'
   (root/'Scripts/v2-validation.json').write_text(json.dumps(report,indent=2))
   p.reset_ride();unreal.unregister_slate_post_tick_callback(handle);return
  active=cases[index];elapsed=0;rows=[]
  p.reset_ride();p.set_actor_location_and_rotation(unreal.Vector(*active[2]),unreal.Rotator(),False,True)
  m.set_editor_property('gear',active[3]);m.set_editor_property('speed',active[4]);m.set_editor_property('recovery',0)
  for key in active[5]:p.validation_key(unreal.Name(key),True)
  report['active']=active[0]
  (root/'Scripts/v2-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/v2-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle)
handle=unreal.register_slate_post_tick_callback(tick)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_begin_play()
