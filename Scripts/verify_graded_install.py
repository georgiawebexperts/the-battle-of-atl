"""Fresh-process checks of the saved graded world, without rebuilding navigation."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();unreal.PiedmontWorldTools.finish_editor_asset_loading()
actors=ea.get_all_level_actors();assert 'TenthStreetGraded_v1' in [str(t) for t in world.get_world_settings().tags]
land=[a for a in actors if isinstance(a,unreal.Landscape)];assert len(land)==1 and land[0].get_actor_scale3d().y>0
roads=[a for a in actors if 'BattleTenthStreet' in [str(t) for t in a.tags]];assert len(roads)==6
for a in roads:
 assert '/TenthStreetGraded/' in a.static_mesh_component.static_mesh.get_path_name()
 assert str(a.static_mesh_component.get_collision_profile_name())==('NoCollision' if 'Paint' in a.get_actor_label() else 'BlockAll')
paths=[a for a in actors if isinstance(a,unreal.PiedmontPathSpline)];assert not any('fixture' in a.get_actor_label().lower() for a in paths)
assert len([a for a in paths if 'TenthStreetGraded_v1' in [str(t) for t in a.tags]])==23
failures=[]
probes=json.loads((root/'SourceAssets/Terrain/TenthStreetGraded/collision-samples.json').read_text())['samples']
for p in probes:
 x,y,z=p['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+150),unreal.Vector(x,y,z-150))
 if not hit or abs(hit[0].z-z)>8:failures.append(p)
previous=json.loads((root/'Tests/Results/2026-09-11-graded-native-collision.json').read_text());routes=[]
for row in previous['navigation']['routes']:
 length=unreal.PiedmontWorldTools.park_route_length(unreal.Vector(*row['start']),unreal.Vector(*row['end']));routes.append({'label':row['label'],'length_cm':length})
passed=not failures and len(routes)==23 and all(r['length_cm']>=0 for r in routes)
report={'passed':passed,'fresh_map_reload':True,'road_actors':len(roads),'adjusted_routes':23,'collision_probes':len(probes),'collision_failures':len(failures),'saved_navigation_routes':routes,'fixture_absent':True,'desktop_updated':False}
(root/'Tests/Results/2026-09-11-graded-install.json').write_text(json.dumps(report,indent=2)+'\n');assert passed
