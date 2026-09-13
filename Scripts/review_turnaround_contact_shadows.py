"""Try closer vehicle contact shadows in the isolated turnaround map."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogTurnaroundReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);rows=[]
settings={'shadow_bias':.5,'shadow_slope_bias':.5,'contact_shadow_length':.03,'shadow_resolution_scale':2.0}
for a in ea.get_all_level_actors():
    if isinstance(a,unreal.DirectionalLight):
        c=a.get_component_by_class(unreal.DirectionalLightComponent);before={k:c.get_editor_property(k) for k in settings}
        for k,v in settings.items():c.set_editor_property(k,v)
        rows.append({'actor':a.get_actor_label(),'before':before,'after':{k:c.get_editor_property(k) for k in settings}})
assert len(rows)==1,rows
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-13-turnaround-shadow-candidate.json').write_text(json.dumps({'settings':rows,'main_map_changed':False,'visual_accepted':False,'scope':'Original bias and screen-space contact shadows on review-map sunlight only; appearance and performance pending.'},indent=2)+'\n')
