"""Read-only probe: does the saved map already route Monroe to Irwin?

The installers assert this after they rebuild navigation. When that assert fires
it is worth knowing whether the route was unreachable before the rebuild too, so
this measures the query, the rebuild and the query again without saving anything.
"""
import unreal, pathlib, sys, json
root = pathlib.Path(unreal.Paths.project_dir())
sys.path.insert(0, str(root / 'Scripts'))
from battle_geography import source_vector

assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
network = json.loads((root / 'SourceAssets/Terrain/eastside-trail-network.json').read_text())
start = source_vector(network['paths'][0]['points_cm'][0])
end = source_vector(network['paths'][-1]['points_cm'][-1])
before = unreal.PiedmontWorldTools.park_route_length(start, end)
unreal.log_error('NAVPROBE saved_map_route_cm=%s' % before)

all_points = [a.centerline.get_location_at_spline_point(i, unreal.SplineCoordinateSpace.WORLD)
              for a in ea.get_all_level_actors() if isinstance(a, unreal.PiedmontPathSpline)
              for i in range(a.centerline.get_number_of_spline_points())]
lo = [min(getattr(p, k) for p in all_points) for k in ['x', 'y', 'z']]
hi = [max(getattr(p, k) for p in all_points) for k in ['x', 'y', 'z']]
built = unreal.PiedmontWorldTools.build_park_navigation(
    unreal.Vector(*[(a + b) / 2 for a, b in zip(lo, hi)]),
    unreal.Vector(*[(b - a) / 2 + 500 for a, b in zip(lo, hi)]))
finished = unreal.PiedmontWorldTools.finish_park_navigation_build()
after = unreal.PiedmontWorldTools.park_route_length(start, end)
unreal.log_error('NAVPROBE rebuild_started=%s finished=%s rebuilt_route_cm=%s nav_splines=%d' % (
    built, finished, after, len(all_points)))
# The editor-style invocation does not exit when the script ends the way the
# python commandlet does.
if '-BattleQuitAfterScript' in unreal.SystemLibrary.get_command_line():
    unreal.SystemLibrary.quit_editor()
