"""Restore original sun bias for an isolated two-cascade rendering comparison."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogTurnaroundReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);rows=[]
for a in ea.get_all_level_actors():
    if isinstance(a,unreal.DirectionalLight):
        c=a.get_component_by_class(unreal.DirectionalLightComponent)
        for k,v in {'shadow_bias':.5,'shadow_slope_bias':.5,'contact_shadow_length':0.0,'shadow_resolution_scale':1.0}.items():c.set_editor_property(k,v)
        rows.append({k:c.get_editor_property(k) for k in ['shadow_bias','shadow_slope_bias','contact_shadow_length','shadow_resolution_scale','dynamic_shadow_cascades','cascade_distribution_exponent']})
assert len(rows)==1 and rows[0]['dynamic_shadow_cascades']>=2,rows
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-13-two-cascade-review-settings.json').write_text(json.dumps({'settings':rows,'main_map_changed':False,'scope':'Original sun settings restored in review. Two-cascade command-line override and visual/performance verification still required.'},indent=2)+'\n')
