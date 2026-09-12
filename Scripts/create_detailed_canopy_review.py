"""Save an instanced Hornbeam review with conservative rider-height geometry clearance."""
import json,math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());manifest=json.loads((root/'SourceAssets/Manifests/epic-tree-migration.json').read_text())
meshes=[unreal.load_asset(p) for p in manifest['roots'][:2]];assert all(meshes)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for actor in ea.get_all_level_actors():
 if actor.actor_has_tag('BattleParkCanopy'):ea.destroy_actor(actor)
segments=[]
for path in json.loads((root/'SourceAssets/Terrain/park-path-network.json').read_text())['paths']:
 points=path['points_cm']
 for a,b in zip(points,points[1:]):segments.append((a[0],-a[1],b[0],-b[1],path['width_game_cm']/2))
def distance(x,y,s):
 ax,ay,bx,by,half=s;dx=bx-ax;dy=by-ay;t=max(0,min(1,((x-ax)*dx+(y-ay)*dy)/max(1e-9,dx*dx+dy*dy)))
 return math.hypot(x-ax-t*dx,y-ay-t*dy)-half
rows=json.loads((root/'SourceAssets/Terrain/park-tree-stations.json').read_text())['trees'];groups=[[],[]];kept=[];rejected=[]
for index,row in enumerate(rows):
 x,y=row['xy_cm'];choice=1 if row['placement']=='mapped_tree' or index%3==0 else 0;mesh=meshes[choice];b=mesh.get_bounds();scale=row['height_game_cm']/(2*b.box_extent.z)
 # 300 cm rider/camera envelope plus 80 cm lateral margin. All triangles
 # entering the band contribute their complete extent; higher overhang is allowed.
 radius=unreal.PiedmontWorldTools.get_tree_low_geometry_radius(mesh,300/scale)*scale
 assert radius>0
 clearance=min(distance(x,y,s) for s in segments)
 if clearance<radius+80:rejected.append({'index':index,'reason':'low_geometry_path_clearance','clearance_cm':clearance,'radius_cm':radius});continue
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000),0)
 if not hit or not isinstance(hit[1],unreal.Landscape):rejected.append({'index':index,'reason':'not_landscape'});continue
 location=unreal.Vector(x,y,hit[0].z-(b.origin.z-b.box_extent.z)*scale)
 groups[choice].append(unreal.Transform(location=location,rotation=unreal.Rotator(yaw=row['yaw']),scale=unreal.Vector(scale,scale,scale)))
 kept.append({'index':index,'mesh':mesh.get_path_name(),'path_margin_cm':clearance-radius,'low_geometry_radius_cm':radius})
assert kept
counts=[]
for mesh,transforms in zip(meshes,groups):
 if not transforms:continue
 actor=unreal.PiedmontWorldTools.create_foliage_review_instances(transforms,mesh);assert actor
 component=actor.get_component_by_class(unreal.HierarchicalInstancedStaticMeshComponent);assert component.get_instance_count()==len(transforms);counts.append(len(transforms))
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontDetailedCanopyReview')
report={'main_map_changed':False,'instances':sum(counts),'groups':counts,'accepted':kept,'rejected':rejected,'height_band_cm':300,'scope':'LOD0 triangles within rider-height band vs mapped paths and native Landscape placement. Trunk collision, under-branch riding, PCG integration, native visual/performance acceptance remain pending.'}
(root/'Tests/Results/2026-09-12-detailed-canopy-low-clearance.json').write_text(json.dumps(report,indent=2)+'\n');print('DETAILED_CANOPY_REVIEW',sum(counts),len(rejected))
