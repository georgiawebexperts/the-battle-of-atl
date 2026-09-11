"""Place ordered gameplay checkpoints on the trail nearest sourced landmarks."""
from pathlib import Path
import json,numpy as np
from pyproj import Transformer
root=Path(__file__).resolve().parents[1];base=root/'SourceAssets/Terrain'
meta=json.loads((base/'terrain-georeference.json').read_text());ox,oy=meta['origin_utm'];t=Transformer.from_crs(4326,32616,always_xy=True)
points=[]
for file in ['eastside-trail-network.json','krog-route-network.json']:
 for path in json.loads((base/file).read_text())['paths']:
  for p in path['points_cm']:
   if not points or np.linalg.norm(np.array(points[-1])-p)>1:points.append(p)
a=np.array(points);landmarks=json.loads((root/'References/checkpoint-landmarks-osm.json').read_text())['elements'];rows=[]
for source_id,name in [(741961704,'Murder K'),(5413435,'Krog Street Market')]:
 e=next(e for e in landmarks if e['id']==source_id);c=e['center'];x,y=t.transform(c['lon'],c['lat']);q=np.array([(x-ox)/.03,(y-oy)/.03]);i=int(np.argmin(np.linalg.norm(a[:,:2]-q,axis=1)))
 p=a[i];d=a[min(i+1,len(a)-1)]-a[max(0,i-1)];yaw=float(np.degrees(np.arctan2(-d[1],d[0])))
 rows.append({'name':name,'source_id':source_id,'source_type':e['type'],'landmark_center':c,'mainline_index':i,'world_xyz_cm':[p[0],-p[1],p[2]],'world_yaw':yaw,'policy':'Nearest installed trail sample to OSM landmark centre; gameplay checkpoint, not a surveyed plaza entrance.'})
assert rows[0]['mainline_index']<rows[1]['mainline_index']
(base/'battle-checkpoints.json').write_text(json.dumps({'source':'OpenStreetMap contributors, ODbL','checkpoints':rows},indent=2)+'\n')
header='#pragma once\n// Generated from sourced landmarks by prepare_battle_checkpoints.py.\nnamespace BattleCheckpoints {\nstruct FAnchor { const TCHAR* Name; double X,Y,Z,Yaw; };\ninline const FAnchor Anchors[] = {\n'
for row in rows:header+=' {TEXT("%s"),%.12f,%.12f,%.12f,%.12f},\n'%(row['name'],*row['world_xyz_cm'],row['world_yaw'])
header+='};\n}\n';(root/'Source/AuraPlayground/BattleCheckpoints.h').write_text(header);print(json.dumps(rows,indent=2))
