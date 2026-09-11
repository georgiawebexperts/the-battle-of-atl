"""Preserve ESU geometry while removing negative-scale landscape sweep failures."""
import unreal,pathlib,json,random,sys
r=pathlib.Path(unreal.Paths.project_dir());sys.path.insert(0,str(r/'Scripts'))
from battle_geography import import_source_landscape,require_converted_world
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();require_converted_world(w);unreal.PiedmontWorldTools.finish_editor_asset_loading()
old=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.Landscape));assert old.get_actor_scale3d().y<0,'Already normalized or unexpected terrain frame'
meta_path=r/'SourceAssets/Terrain/terrain-georeference.json';meta=json.loads(meta_path.read_text())
newmeta=dict(meta);newmeta.update(landscape_import_layout='transpose_xy',landscape_import_size=[meta['size'][1],meta['size'][0]],landscape_rotation_yaw=-90,world_scale=[abs(v) for v in meta['world_scale']])
# Only terrain participates in these comparisons. Restore every other actor before saving.
others=[(a,a.get_actor_enable_collision()) for a in ea.get_all_level_actors() if a!=old]
for a,enabled in others:a.set_actor_enable_collision(False)
pts=[(4641.959,4967.324),(-6179.605,7295.868),(4866.098,4915.732),(5616.476,4743.011)]
rng=random.Random(26);pts.extend((rng.uniform(-35000,62000),rng.uniform(-66000,132000)) for _ in range(160))
def sample():
 rows=[]
 for x,y in pts:
  top=unreal.Vector(x,y,7000);bottom=unreal.Vector(x,y,-7000)
  line=unreal.PiedmontWorldTools.trace_world_surface(top,bottom)
  cap=unreal.SystemLibrary.capsule_trace_single(w,top,bottom,32,96,unreal.TraceTypeQuery.ECC_VISIBILITY,False,[],unreal.DrawDebugTrace.NONE)
  assert line
  rows.append({'xy':[x,y],'height':line[0].z,'capsule':cap is not None})
 return rows
before=sample();old.set_actor_enable_collision(False)
new=import_source_landscape(r/'SourceAssets/Terrain/atlanta-height-krog.r16',newmeta);assert new and len(new.get_components_by_class(unreal.LandscapeComponent))==288
new.set_editor_property('landscape_material',old.get_editor_property('landscape_material'));new.tags=list(old.tags)
unreal.PiedmontWorldTools.finish_editor_asset_loading();after=sample()
err=max(abs(a['height']-b['height']) for a,b in zip(before,after));assert err<.001 and all(a['capsule'] for a in after)
label=old.get_actor_label();assert ea.destroy_actor(old);new.set_actor_label(label)
for a,enabled in others:a.set_actor_enable_collision(enabled)
# Rebuild the path-only navigation mesh after replacing the terrain actor.
points=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) for i in range(a.centerline.get_number_of_spline_points())]
lo=[min(getattr(p,k) for p in points) for k in ['x','y','z']];hi=[max(getattr(p,k) for p in points) for k in ['x','y','z']]
assert unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]),unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)]))
assert unreal.PiedmontWorldTools.finish_park_navigation_build()
assert unreal.EditorLoadingAndSavingUtils.save_map(w,'/Game/PiedmontRide/Maps/PiedmontWorld')
meta_path.write_text(json.dumps(newmeta,indent=2)+'\n')
report={'passed':True,'before':before,'after':after,'max_height_change_cm':err,'map_saved':True,'landscape_components':288,'new_rotation_yaw':-90,'source_heightmap':'atlanta-height-krog.r16','source_samples_unchanged':True,'native_character_movement_pending':True}
(r/'Tests/Results/2026-09-11-build026-terrain-sweeps.json').write_text(json.dumps(report,indent=2)+'\n');print('TERRAIN_FIXED',sum(a['capsule'] for a in before),len(after),err)
