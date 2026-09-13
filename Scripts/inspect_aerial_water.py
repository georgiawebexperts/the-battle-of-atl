"""Read-only map geometry/culling inspection for the aerial intro."""
import unreal,json,pathlib,traceback
root=pathlib.Path(unreal.Paths.project_dir());out={'actors':[]}
try:
 unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level('/Game/PiedmontRide/Maps/PiedmontWorld')
 def vec(v):return [v.x,v.y,v.z]
 for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
  name=a.get_actor_label()
  if not any(s in name.lower() for s in ['lake clara','water zone','lake crossing','underwater basin']):continue
  row={'actor_hidden':a.get_editor_property('hidden'),'label':name,'class':a.get_class().get_name(),'location':vec(a.get_actor_location()),'scale':vec(a.get_actor_scale3d()),'components':[]}
  for c in a.get_components_by_class(unreal.PrimitiveComponent):
   d={'name':c.get_name(),'class':c.get_class().get_name()}
   for prop in ['visible','hidden_in_game','ld_max_draw_distance','cached_max_draw_distance','bounds_scale']:
    try:d[prop]=c.get_editor_property(prop)
    except Exception:pass
   if isinstance(c,unreal.MeshComponent):d['materials']=[str(c.get_material(i)) for i in range(c.get_num_materials())]
   row['components'].append(d)
  splines=a.get_components_by_class(unreal.SplineComponent)
  if splines:
   s=splines[0];row['spline_world']=[vec(s.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD)) for i in range(s.get_number_of_spline_points())]
  if 'water recovery' in name:
   row['hazard_world']=[vec(a.get_actor_transform().transform_location(v)) for v in a.get_editor_property('polygon')]
  out['actors'].append(row)
 out['surface_samples']=[]
 for x,y in [(-6000,1000),(-3500,0),(-8000,2500),(-10000,0),(-4000,-2000)]:
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,2000),unreal.Vector(x,y,-2000))
  out['surface_samples'].append({'xy':[x,y],'hit':str(hit)})
 out['passed']=True
except Exception:out.update(passed=False,error=traceback.format_exc())
(root/'work/aerial-water-inspection.json').write_text(json.dumps(out,indent=2)+'\n')
