"""Fresh-process verification of persisted market dressing and aisle clearance."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=[a for a in ea.get_all_level_actors() if unreal.Name('TwelfthStreetMarket') in a.tags]
expected=json.loads((root/'Tests/Results/2026-09-12-twelfth-market-install.json').read_text())
assert sorted(a.get_actor_label() for a in actors)==sorted(expected['actors'])
counts={};samples=[]
for a in actors:
 c=a.get_component_by_class(unreal.StaticMeshComponent);assert c
 mesh=c.get_editor_property('static_mesh');assert mesh
 name=mesh.get_name();counts[name]=counts.get(name,0)+1
 if name in ['SM_MarketStall_Metal','SM_MarketStall_Wood']:assert str(c.get_collision_profile_name())=='BlockAll'
for kind in ['Canvas','Metal','Wood','Leaf','Tomato','Cloth']:assert counts.get('SM_MarketStall_'+kind)==10
assert counts.get('SM_MarketSign')==10
layout=json.loads((root/'SourceAssets/Terrain/TwelfthMarket/layout.json').read_text())
for row in layout['stalls'][::2]:
 # The midpoint between each opposing pair is the aisle.
 other=layout['stalls'][layout['stalls'].index(row)+1];x=(row['center_xy'][0]+other['center_xy'][0])/2;y=(row['center_xy'][1]+other['center_xy'][1])/2
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1500),unreal.Vector(x,y,-1500));assert hit
 assert hit[1] not in actors,'Stall blocks aisle ground probe'
 samples.append({'xy':[x,y],'ground_actor':hit[1].get_actor_label(),'z':hit[0].z})
(root/'Tests/Results/2026-09-12-twelfth-market-persisted.json').write_text(json.dumps({'passed':True,'actor_count':len(actors),'mesh_counts':counts,'aisle_ground_samples':samples,'scope':'Fresh process verifies saved parts and collision profiles; five aisle ground probes, not player traversal or full art acceptance'},indent=2))
