"""Report the width every Eastside/connector spline actually carries in the saved map.

Read-only. Run after install_eastside_trail.py / install_beltline_connector.py to
prove the widened width reached the level rather than just the source JSON.
"""
import unreal, pathlib, sys
root = pathlib.Path(unreal.Paths.project_dir())
sys.path.insert(0, str(root / 'Scripts'))

assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

widths = {}
for a in ea.get_all_level_actors():
    if not isinstance(a, unreal.PiedmontPathSpline):
        continue
    label = a.get_actor_label()
    if not (label.startswith('Eastside trail spline') or label.startswith('Eastside connector spline')):
        continue
    widths.setdefault(round(float(a.get_editor_property('width_cm')), 2), []).append(label)

unreal.log_error('ARCADE TRAIL WIDTHS: %s' % ({w: len(v) for w, v in widths.items()},))
for w in sorted(widths):
    unreal.log_error('  width_cm=%s -> %d splines, e.g. %s' % (w, len(widths[w]), widths[w][0]))
