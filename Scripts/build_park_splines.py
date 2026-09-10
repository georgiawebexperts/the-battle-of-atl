"""Retain every sourced path as an editable Unreal spline, with OSM provenance."""
import unreal,json,pathlib
p=pathlib.Path(unreal.Paths.project_dir());data=json.loads((p/'SourceAssets/Terrain/park-path-network.json').read_text());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if w.get_name()!='PiedmontWorld':raise RuntimeError('Wrong map for park paths')
existing={a.get_actor_label():a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline)};rows=[]
for i,path in enumerate(data['paths']):
 label='OSM path '+str(path['osm_id'])+' part '+str(i)
 a=existing.get(label) or ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector())
 a.set_actor_label(label);a.set_folder_path('Piedmont/Path centerlines')
 a.set_editor_property('osm_way_id',str(path['osm_id']));a.set_editor_property('width_cm',path['width_game_cm']);a.set_editor_property('bridge',path['tags'].get('bridge')=='yes');a.set_editor_property('ride_validated',False)
 a.set_centerline([unreal.Vector(*v) for v in path['points_cm']])
 count=a.centerline.get_number_of_spline_points()
 rows.append({'osm_id':path['osm_id'],'points':count,'pass':count==len(path['points_cm'])})
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(unreal.Vector(-12000,-25000,45000),unreal.Rotator(pitch=-55,yaw=75,roll=0))
(p/'Scripts/park-splines-built.json').write_text(json.dumps({'status':'passed' if all(r['pass'] for r in rows) else 'failed','path_count':len(rows),'scope':'Spline point import only; rideability unverified','paths':rows},indent=2))
