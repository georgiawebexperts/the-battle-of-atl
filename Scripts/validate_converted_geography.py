"""Reload the saved map and compare against retained pre-conversion observations."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert 'BattleGeography_ESU_v1' in [str(t) for t in world.get_world_settings().tags]
baseline=json.loads((root/'Tests/Results/2026-09-11-coordinate-conversion.json').read_text())
errors=[];seams=[]
for row in baseline['baseline_samples']:
    x,y,z=row['position'];p=unreal.Vector(x,-y,z)
    hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,6000),p-unreal.Vector(0,0,6000))
    error=abs(hit[0].z-row['surface'][2]) if hit else 1e9
    if hit and hit[1].get_actor_label()==row['actor'] and error<=.1:continue
    neighbors=row['neighbors_2cm']
    seam=hit and neighbors and hit[1].get_actor_label() in [n['actor'] for n in neighbors] and min(n['z'] for n in neighbors)-.1<=hit[0].z<=max(n['z'] for n in neighbors)+.1 and error<3.1
    (seams if seam else errors).append({'source_cm':row['position'],'error_cm':error})
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
start=next(a for a in actors if isinstance(a,unreal.PlayerStart))
anchor=json.loads((root/'SourceAssets/Terrain/battle-start.json').read_text())
assert abs(start.get_actor_location().x-anchor['world_start_xy_cm'][0])<.2
assert abs(start.get_actor_location().y-anchor['world_start_xy_cm'][1])<.2
assert abs(start.get_actor_rotation().yaw-anchor['world_heading_yaw'])<.01
end=json.loads((root/'SourceAssets/Terrain/battle-park-exit.json').read_text())['world_xyz_cm']
length=unreal.PiedmontWorldTools.park_route_length(start.get_actor_location(),unreal.Vector(*end))
report={'passed':not errors and length>0,'reloaded_map':True,'collision_samples':len(baseline['baseline_samples']),'errors':errors,'measured_seams':seams,'start_location_heading':True,'start_to_eastside_nav_cm':length,'rendered_appearance_verified':False}
(root/'Tests/Results/2026-09-11-coordinate-reload.json').write_text(json.dumps(report,indent=2)+'\n')
assert report['passed'],report
