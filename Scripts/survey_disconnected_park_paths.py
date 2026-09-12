"""Read-only native survey from isolated path components toward reachable paths."""
import json
import math
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
report = json.loads((root / 'Tests/Results/2026-09-12-krog-world-navigation.json').read_text())
bad_labels = {s['label'] for s in report['after']['samples'] if not s['reachable']}
good_labels = {s['label'] for s in report['after']['samples'] if s['reachable']}
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
paths = {}
for actor in actors:
    if isinstance(actor, unreal.PiedmontPathSpline):
        spline = actor.centerline
        paths[actor.get_actor_label()] = [spline.get_location_at_spline_point(i, unreal.SplineCoordinateSpace.WORLD) for i in range(spline.get_number_of_spline_points())]
def xyz(v):
    return [v.x, v.y, v.z]
good_points = [(label, p) for label, points in paths.items() if label in good_labels for p in points]
rows = []
for label in sorted(bad_labels):
    points = paths[label]
    distance, _, nearest_label, start, end = min(
        (math.hypot(p.x-q.x, p.y-q.y), j, other, p, q)
        for p in points for j, (other, q) in enumerate(good_points))
    samples = []
    count = max(1, math.ceil(distance / 25))
    for i in range(count + 1):
        t = i / count
        x, y, z = [a+(b-a)*t for a, b in zip(xyz(start), xyz(end))]
        hit = unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+200), unreal.Vector(x,y,z-400))
        nav = unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(x,y,z))
        samples.append({'xyz': [x,y,z], 'surface': xyz(hit[0]) if hit else None,
                        'actor': hit[1].get_actor_label() if hit else None,
                        'nav': xyz(nav) if nav else None})
    rows.append({'label': label, 'nearest_reachable_label': nearest_label,
                 'centerline_xy_gap_cm': distance, 'start': xyz(start), 'end': xyz(end), 'samples': samples})
(root / 'Tests/Results/2026-09-12-disconnected-park-native-survey.json').write_text(json.dumps({
    'scope': 'Nearest sampled centerlines and native surface traces, not proposed connecting routes. No map changes.',
    'paths': rows}, indent=2)+'\n')
