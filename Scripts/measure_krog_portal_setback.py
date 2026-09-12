"""Find where the full authored portal width clears the mapped DeKalb road."""
import json,sys
from pathlib import Path
from shapely.geometry import LineString
from shapely.ops import unary_union
sys.path.insert(0,str(Path(__file__).resolve().parent))
from route_height_profiles import RouteHeightProfiles
r=Path(__file__).resolve().parents[1];h=RouteHeightProfiles(r/'SourceAssets/Terrain/krog-height-profiles.json');p=h.profiles[0];network=json.loads((r/'SourceAssets/Terrain/KrogTraffic/network.json').read_text())
# Network is ESU; tunnel centreline is ENU.
road=unary_union([LineString([(q[0],-q[1]) for q in row['points_cm']]).buffer(450,join_style=2) for row in network['roads'] if row['tags']['name']=='DeKalb Avenue Northeast'])
def section(s):
 q=h.line.interpolate(s);a=h.line.interpolate(s-1);b=h.line.interpolate(s+1);dx=b.x-a.x;dy=b.y-a.y;l=(dx*dx+dy*dy)**.5;nx=-dy/l;ny=dx/l
 return LineString([(q.x+nx*v,q.y+ny*v) for v in [-660,380]])
lo=p['bridge_start_cm'];hi=p['bridge_end_cm'];clear=None
for delta in range(0,2501,10):
 if section(lo+delta).distance(road)>=80:clear=lo+delta;break
assert clear is not None
out={'author':'2026-09-12 [codex-maclaptop]','original_portal_station_cm':lo,'candidate_portal_station_cm':clear,'setback_game_cm':clear-lo,'remaining_tunnel_cm':hi-clear,'road_half_width_cm':450,'minimum_clearance_cm':80,'candidate_cross_section_enu':list(section(clear).coords),'main_map_changed':False,'scope':'Geometric proposal, not installed; verify shell and columns against road envelope and preserve floor/sidewalk before adoption'}
(r/'Tests/Results/2026-09-12-krog-portal-setback.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out))
