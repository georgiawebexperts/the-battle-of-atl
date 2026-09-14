"""Read saved main-map utility light types and settings."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors();rows=[]
points=[a for a in actors if isinstance(a,unreal.PointLight) and a.actor_has_tag('KrogUtilityLightReview')]
for a in actors:
 if isinstance(a,unreal.SpotLight) and a.actor_has_tag('KrogDownlight'):
  c=a.get_component_by_class(unreal.SpotLightComponent);row={k:c.get_editor_property(k) for k in ['intensity','attenuation_radius','inner_cone_angle','outer_cone_angle']};row['down']=a.get_actor_forward_vector().z;rows.append(row)
passed=len(rows)==6 and not points and all(r['down']<-.99 and abs(r['intensity']-2)<.001 and r['attenuation_radius']==680 and r['inner_cone_angle']==50 and r['outer_cone_angle']==75 for r in rows)
(root/'Tests/Results/2026-09-14-tunnel-downlight-saved.json').write_text(json.dumps({'passed':passed,'lights':rows,'old_points':len(points),'map_saved':False},indent=2)+'\n');assert passed
