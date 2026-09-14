"""Import source candidate into an isolated review map; never install to PiedmontWorld."""
import unreal,pathlib,json,sys,hashlib,array
root=pathlib.Path(unreal.Paths.project_dir());sys.path.insert(0,str(root/'Scripts'))
from battle_geography import import_source_landscape,require_converted_world
main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();require_converted_world(world);ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
review='/Game/PiedmontRide/Maps/PiedmontLakePathGradingReview'
old=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.Landscape));old.set_actor_enable_collision(False)
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());candidate=root/'SourceAssets/Terrain/LakePathGrading/atlanta-height-lake-paths-candidate.r16'
new=import_source_landscape(candidate,meta);assert new
new.set_editor_property('landscape_material',old.get_editor_property('landscape_material'));new.tags=list(old.tags)
label=old.get_actor_label();ea.destroy_actor(old);new.set_actor_label(label)
exec(compile((root/'Scripts/import_lake_graded_connections.py').read_text(),'import_lake_graded_connections.py','exec'), {'__name__':'__main__'})
exec(compile((root/'Scripts/import_lake_path_seams.py').read_text(),'import_lake_path_seams.py','exec'), {'__name__':'__main__'})
unreal.PiedmontWorldTools.finish_editor_asset_loading()
raw=array.array('H');raw.frombytes(candidate.read_bytes())
if sys.byteorder!='little':raw.byteswap()
nx,ny=meta['size'];sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm']
def height(x,y):
 gx=(x-lx)/sx;gy=(-y-ly)/sy;i,j=int(gx),int(gy);u,v=gx-i,gy-j
 a,b,c,d=[(raw[(j+dy)*nx+i+dx]-32768)*sz/128 for dx,dy in [(0,0),(1,0),(0,1),(1,1)]]
 return a+(b-a)*u+(d-b)*v if u>=v else a+(d-c)*u+(c-a)*v
survey=json.loads((root/'work/lake-path-grades.json').read_text());checks=[];fail=[]
for r in survey['samples']:
 if not r['actor'].startswith('Park pavement'):continue
 x,y,z=r['xyz'];expected=height(x,y)+3;top=1800;hit=None
 for _ in range(12):
  h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,top),unreal.Vector(x,y,-1600))
  if not h:break
  if h[1].get_actor_label().startswith('Park pavement'):hit=h;break
  top=h[0].z-.5
 error=None if not hit else hit[0].z-expected
 row={'osm_id':r['osm_id'],'xyz':[x,y,expected],'height_error_cm':error};checks.append(row)
 if error is None or abs(error)>.1:fail.append(row)
assert not fail, str(fail[:5])
exec(compile((root/'Scripts/update_lake_grading_routes.py').read_text(),'update_lake_grading_routes.py','exec'), {'__name__':'__main__'})
assert unreal.EditorLoadingAndSavingUtils.save_map(world,review)
assert hashlib.sha256(main.read_bytes()).hexdigest()==before,'Main map changed unexpectedly'
(root/'Tests/Results/2026-09-13-lake-path-grading-native.json').write_text(json.dumps({'passed':not fail,'review_map':review,'main_map_unchanged_sha256':before,'checks':len(checks),'maximum_height_error_cm':max(abs(c['height_error_cm']) for c in checks),'failures':fail,'scope':'Candidate terrain and pavement collision only. Navigation is checked separately; moving bike and game visuals pending.','installed':False},indent=2)+'\n')
