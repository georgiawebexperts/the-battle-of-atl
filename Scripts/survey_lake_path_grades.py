"""Read actual collision at mapped path centers; identify abrupt grades without saving map."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
network=json.loads((root/'SourceAssets/Terrain/park-path-network.json').read_text()); rows=[]; segments=[]
for path in network['paths']:
 previous=None
 for index,p in enumerate(path['points_cm']):
  x,y=p[0],-p[1]
  if not (-11000<x<1500 and -7000<y<4000):previous=None;continue
  top=1800;hit=None
  for _ in range(12):
   h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,top),unreal.Vector(x,y,-1600))
   if not h:break
   actor=h[1];label=actor.get_actor_label()
   if label.startswith('Park pavement') or isinstance(actor,unreal.Landscape):hit=h;break
   top=h[0].z-.5
  if not hit:previous=None;continue
  row={'osm_id':path['osm_id'],'index':index,'xyz':[x,y,hit[0].z],'actor':hit[1].get_actor_label(),'tags':path['tags']};rows.append(row)
  if previous:
   a=previous['xyz'];dist=math.hypot(x-a[0],y-a[1]);grade=(hit[0].z-a[2])/dist if dist else 0
   if dist>5:segments.append({'osm_id':path['osm_id'],'a':a,'b':row['xyz'],'distance_cm':dist,'grade':grade,'tags':path['tags'],'actors':[previous['actor'],row['actor']]})
  previous=row
segments.sort(key=lambda s:abs(s['grade']),reverse=True)
(root/'work/lake-path-grades.json').write_text(json.dumps({'map_saved':False,'samples':rows,'segments':segments,'count_over_25_percent':sum(abs(s['grade'])>.25 for s in segments)},indent=2)+'\n')
