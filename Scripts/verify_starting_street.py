"""Trace the native tutorial frontage surfaces without saving the main map."""
import unreal,json,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);actor=ea.spawn_actor_from_class(unreal.BattleTutorial,unreal.Vector());assert actor
unreal.PiedmontWorldTools.finish_editor_asset_loading();r=json.loads((root/'SourceAssets/Terrain/StartingStreet/profile.json').read_text());samples=[]
for i,row in enumerate(r['rows']):
 for side in (-300,-210,0,210,300):
  z=row['z']+(12.4 if abs(side)==300 else -abs(side)*.02)*row['fade'];p=row['point'];n=row['normal'];q=unreal.Vector(p[0]+side*n[0],p[1]+side*n[1],z)
  hit=unreal.PiedmontWorldTools.trace_world_surface(q+unreal.Vector(0,0,100),q-unreal.Vector(0,0,100))
  samples.append({'row':i,'side':side,'height_error_cm':abs(hit[0].z-z) if hit else None,'actor':hit[1].get_actor_label() if hit else None,'plateau':row['fade']>.999})
errors=[s for s in samples if s['plateau'] and (s['height_error_cm'] is None or s['height_error_cm']>.3)]
assert hashlib.sha256(main.read_bytes()).hexdigest()==before
report={'passed':not errors,'samples':len(samples),'plateau_samples':sum(s['plateau'] for s in samples),'errors':errors,'max_plateau_error_cm':max(s['height_error_cm'] or 0 for s in samples if s['plateau']),'main_sha256_preserved':before,'scope':'Native tutorial actor and five cross-section ground traces at101stations; central full-profile road and sidewalk match exported surface within.3cm. Taper endpoints, rendering and riding acceptance separate.'}
(root/'Tests/Results/2026-09-13-starting-street-traces.json').write_text(json.dumps(report,indent=2)+'\n');assert report['passed'],report
