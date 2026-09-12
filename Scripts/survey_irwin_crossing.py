"""Measure installed trail/terrain support around the Irwin/Lake crossing."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());data=json.loads((root/'SourceAssets/Terrain/IrwinTraffic/network.json').read_text());x,y,z=data['crossing_xyz'];assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading();rows=[];labels={}
for dx in range(-1000,1001,100):
 for dy in range(-500,501,100):
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+dx,y+dy,z+1500),unreal.Vector(x+dx,y+dy,z-1500));label=hit[1].get_actor_label() if hit else None;labels[label]=labels.get(label,0)+1;rows.append({'xy':[x+dx,y+dy],'height':hit[0].z if hit else None,'actor':label})
center=next(r for r in rows if abs(r['xy'][0]-x)<.01 and abs(r['xy'][1]-y)<.01)
r={'site_xyz':data['crossing_xyz'],'center_hit':center,'surface_counts':labels,'samples':rows,'main_map_changed':False,'scope':'Native surface survey only, no road or traffic installed.'};(root/'Tests/Results/2026-09-12-irwin-crossing-survey.json').write_text(json.dumps(r,indent=2)+'\n')
