import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogApproachReview');unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
a=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('KrogApproachReview'));a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
verts=[];faces=[]
for line in (root/'SourceAssets/Terrain/KrogApproach/KrogApproach.obj').read_text().splitlines():
 t=line.split()
 if t and t[0]=='v':verts.append(unreal.Vector(float(t[1]),-float(t[2]),float(t[3])))
 if t and t[0]=='f':faces.append([int(v.split('/')[0])-1 for v in t[1:]])
rows=[]
for face in faces:
 a,b,c=[verts[i] for i in face]
 for u,v in ((.25,.25),(.5,.25),(.25,.5),(1/3,1/3)):
  p=a*(1-u-v)+b*u+c*v;h=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,200),p-unreal.Vector(0,0,200));rows.append({'gap_cm':p.z-h[0].z if h else None,'actor':h[1].get_actor_label() if h else None,'xyz':[p.x,p.y,p.z]})
(root/'Tests/Results/2026-09-13-approach-underlay-gaps.json').write_text(json.dumps({'samples':rows,'minimum_gap_cm':min(r['gap_cm'] for r in rows if r['gap_cm'] is not None),'scope':'Interior triangle samples versus native underlying surfaces with only review strip collision disabled transiently. No map save.'},indent=2)+'\n')
