"""Read-only current map water/hazard placement and render component inventory."""
import unreal,json
from pathlib import Path
r=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
def v(p):return [p.x,p.y,p.z]
rows=[]
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 if not any(x in a.get_class().get_name() for x in ['WaterBody','WaterZone','WaterHazard']):continue
 d={'label':a.get_actor_label(),'class':a.get_class().get_name(),'location':v(a.get_actor_location()),'scale':v(a.get_actor_scale3d()),'bounds':[v(x) for x in a.get_actor_bounds(False)],'components':[]}
 for c in a.get_components_by_class(unreal.SceneComponent):
  e={'name':c.get_name(),'class':c.get_class().get_name(),'visible':c.is_visible()}
  if isinstance(c,unreal.MeshComponent):e['materials']=[str(c.get_material(i)) for i in range(c.get_num_materials())]
  d['components'].append(e)
 if isinstance(a,unreal.PiedmontWaterHazard):d['polygon']=[v(a.get_actor_transform().transform_location(p)) for p in a.get_editor_property('polygon')]
 if isinstance(a,unreal.WaterBodyLake):
  sp=a.get_component_by_class(unreal.WaterSplineComponent);d['spline']=[v(sp.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD)) for i in range(sp.get_number_of_spline_points())]
 rows.append(d)
(r/'Tests/Results/2026-09-13-swim-lake-inventory.json').write_text(json.dumps(rows,indent=2)+'\n')
