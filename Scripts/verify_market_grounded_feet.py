"""Fresh-process saved market plate contact and aisle probes."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());main='-VerifyMainMarketFeet' in unreal.SystemLibrary.get_command_line();mapname='/Game/PiedmontRide/Maps/'+('PiedmontWorld' if main else 'PiedmontMarketFeetReview')
assert unreal.EditorLoadingAndSavingUtils.load_map(mapname);unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
market=[a for a in ea.get_all_level_actors() if unreal.Name('TwelfthStreetMarket') in a.tags];plates=[a for a in market if unreal.Name('MarketGroundPlate') in a.tags];assert len(plates)==80
metals=[a for a in market if a.static_mesh_component.static_mesh.get_name()=='SM_MarketMetalWithoutPads'];assert len(metals)==10
for a in metals:assert str(a.static_mesh_component.get_collision_profile_name())=='BlockAll'
for a in plates:assert str(a.static_mesh_component.get_collision_profile_name())=='NoCollision'
# Isolate ground, measuring actual saved plate bounds. No map save.
for a in market:a.set_actor_enable_collision(False)
gaps=[]
for a in plates:
 o,e=a.get_actor_bounds(False);h=unreal.PiedmontWorldTools.trace_world_surface(o+unreal.Vector(0,0,500),o-unreal.Vector(0,0,500));assert h;gaps.append(o.z-e.z-h[0].z)
(root/'work/market-plate-gaps.json').write_text(json.dumps(gaps))
assert max(abs(g+.1) for g in gaps)<.01, str(sorted(gaps)[:4]+sorted(gaps)[-4:])
for a in market:a.set_actor_enable_collision(True)
layout=json.loads((root/'SourceAssets/Terrain/TwelfthMarket/layout.json').read_text());aisle=[]
for a,b in zip(layout['stalls'][::2],layout['stalls'][1::2]):
 x=(a['center_xy'][0]+b['center_xy'][0])/2;y=(a['center_xy'][1]+b['center_xy'][1])/2
 h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1500),unreal.Vector(x,y,-1500));assert h and h[1] not in market
 start=h[0]+unreal.Vector(0,0,90);blocked=unreal.PiedmontWorldTools.trace_world_surface(start,start+unreal.Vector(0,0,60),45);assert not blocked
 aisle.append([x,y,h[0].z])
r={'passed':True,'main_map':main,'map':mapname,'map_saved':False,'plate_count':80,'metal_count':10,'minimum_plate_base_gap_cm':min(gaps),'maximum_plate_base_gap_cm':max(gaps),'aisle_clearance_probes':aisle,'scope':'Saved plate center base contact, no-collision plate profiles, metal collision profiles and five vertical45cm-radius aisle probes. Not full player traversal or packaged acceptance.'}
(root/'Tests/Results'/('2026-09-14-market-grounded-feet-main.json' if main else '2026-09-14-market-grounded-feet-saved.json')).write_text(json.dumps(r,indent=2)+'\n')
