"""Retain every sourced path as an editable Unreal spline, with OSM provenance."""
import unreal,json,pathlib
p=pathlib.Path(unreal.Paths.project_dir());data=json.loads((p/'SourceAssets/Terrain/park-path-network.json').read_text());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if w.get_name()!='PiedmontWorld':raise RuntimeError('Wrong map for park paths')
bridge_file=p/'SourceAssets/Terrain/LakeBridge/manifest.json'
bridge=json.loads(bridge_file.read_text()) if bridge_file.exists() else {}
wetland_file=p/'SourceAssets/Terrain/WetlandBridges/manifest.json'
wetlands=json.loads(wetland_file.read_text())['bridges'] if wetland_file.exists() else []
drive_file=p/'SourceAssets/Terrain/ParkDriveBridge/manifest.json'
if drive_file.exists():wetlands+=json.loads(drive_file.read_text())['bridges']
wetland_heights={b['osm_id']:b['centerline_cm'] for b in wetlands}
existing={a.get_actor_label():a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline)};rows=[]
for i,path in enumerate(data['paths']):
 label='OSM path '+str(path['osm_id'])+' part '+str(i)
 a=existing.get(label) or ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector())
 a.set_actor_label(label);a.set_folder_path('Piedmont/Path centerlines')
 a.set_editor_property('osm_way_id',str(path['osm_id']));a.set_editor_property('width_cm',path['width_game_cm']);a.set_editor_property('bridge',path['tags'].get('bridge')=='yes');a.set_editor_property('ride_validated',False)
 points=bridge.get('centerline_cm',path['points_cm']) if path['osm_id']==102679938 else bridge.get('spur_cm',path['points_cm']) if path['osm_id']==146304988 else path['points_cm']
 points=wetland_heights.get(path['osm_id'],points)
 a.set_centerline([unreal.Vector(*v) for v in points])
 count=a.centerline.get_number_of_spline_points()
 rows.append({'osm_id':path['osm_id'],'points':count,'pass':count==len(points)})
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(unreal.Vector(-12000,-25000,45000),unreal.Rotator(pitch=-55,yaw=75,roll=0))
(p/'Scripts/park-splines-built.json').write_text(json.dumps({'status':'passed' if all(r['pass'] for r in rows) else 'failed','path_count':len(rows),'scope':'Spline point import only; rideability unverified','paths':rows},indent=2))
