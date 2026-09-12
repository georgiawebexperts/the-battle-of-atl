"""Reduce overexposed utility-light pools in the combined candidate only."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
rows=[]
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 if isinstance(actor,unreal.PointLight) and actor.actor_has_tag('KrogUtilityLightReview'):
  light=actor.get_component_by_class(unreal.PointLightComponent)
  rows.append({'label':actor.get_actor_label(),'before_candela':light.get_editor_property('intensity'),'after_candela':2.0})
  light.set_intensity(2.0)
assert len(rows)==6
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-krog-softened-utility-lights.json').write_text(json.dumps({'lights':rows,'main_map_changed':False},indent=2)+'\n')
