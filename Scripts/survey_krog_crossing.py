"""Measure native surface support at the planned Krog/DeKalb approaches."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());data=json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text());x,y,z=data['crossing_xyz']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading();rows=[];counts={}
for dx in range(-1200,1201,100):
 for dy in range(-1200,1201,100):
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+dx,y+dy,z+1500),unreal.Vector(x+dx,y+dy,z-1500));label=hit[1].get_actor_label() if hit else None;counts[label]=counts.get(label,0)+1
  rows.append({'xy':[x+dx,y+dy],'height':hit[0].z if hit else None,'actor':label})
lanes=[]
for road in data['roads']:
 for p in road['points_cm']:
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(p[0],p[1],p[2]+1500),unreal.Vector(p[0],p[1],p[2]-1500))
  lanes.append({'osm_way':road['osm_way'],'xyz':p,'hit_z':hit[0].z if hit else None,'offset_cm':hit[0].z-p[2] if hit else None,'actor':hit[1].get_actor_label() if hit else None})
result={'junction_xyz':data['crossing_xyz'],'samples':rows,'road_samples':lanes,'surface_counts':counts,'main_map_changed':False,'scope':'Native topmost collision surfaces, not a drivable-road or tunnel-clearance acceptance test'}
(root/'Tests/Results/2026-09-12-krog-crossing-survey.json').write_text(json.dumps(result,indent=2)+'\n')
