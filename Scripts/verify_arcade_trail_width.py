"""Prove the two-ribbon BeltLine reached the saved map, and that the swap works.

Read-only: it never saves, so exercising the swap here cannot leave the level in
the wrong mode on disk. Reports, for the map as saved and then in each mode, how
many trail actors each ribbon has, how many are visible, and what width the ten
splines carry. Run after install_eastside_trail.py / install_beltline_connector.py.
The physical surface in each mode is BattleTrailModeAudit's job.
"""
import unreal, pathlib, sys
root = pathlib.Path(unreal.Paths.project_dir())
sys.path.insert(0, str(root / 'Scripts'))

assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def collision_enabled(actor):
    component = getattr(actor, 'static_mesh_component', None)
    if component is None:
        return None
    try:
        return str(component.get_collision_enabled())
    except Exception as error:                                   # pragma: no cover
        return 'unknown (%s)' % error


def hidden_in_game(actor):
    """AActor::bHidden. There is no Python helper for the getter, only the setter."""
    return bool(actor.get_editor_property('hidden'))


def report(where):
    widths, ribbons = {}, {}
    for actor in ea.get_all_level_actors():
        if isinstance(actor, unreal.PiedmontPathSpline) and actor.actor_has_tag('BattleTrailWidth'):
            width = round(float(actor.get_editor_property('width_cm')), 2)
            widths[width] = widths.get(width, 0) + 1
        for tag, key in (('BattleTrailArcade', 'arcade'), ('BattleTrailRealistic', 'realistic')):
            if actor.actor_has_tag(tag):
                entry = ribbons.setdefault(key, {'actors': 0, 'visible': 0, 'collision': {}})
                entry['actors'] += 1
                if not hidden_in_game(actor):
                    entry['visible'] += 1
                state = collision_enabled(actor)
                entry['collision'][state] = entry['collision'].get(state, 0) + 1
    unreal.log_error('TRAIL RIBBONS [%s] spline_widths=%s %s' % (where, widths, ribbons))
    return widths, ribbons


saved_widths, saved_ribbons = report('as saved')
if not saved_ribbons.get('arcade') or not saved_ribbons.get('realistic'):
    raise SystemExit('one of the two ribbons is missing from the saved map')
if saved_ribbons['arcade']['visible'] != saved_ribbons['arcade']['actors']:
    raise SystemExit('the wide ribbon is not the one a fresh load shows')
if saved_ribbons['realistic']['visible'] != 0:
    raise SystemExit('the narrow ribbon is visible before any mode swap')

unreal.BattleTrailMode.apply(world, True)
realistic_widths, realistic_ribbons = report('realistic')
if list(realistic_widths) != [320.0]:
    raise SystemExit('realistic splines do not carry 320 cm: %s' % realistic_widths)
if realistic_ribbons['realistic']['visible'] != realistic_ribbons['realistic']['actors']:
    raise SystemExit('the narrow ribbon did not become visible')
if realistic_ribbons['arcade']['visible'] != 0:
    raise SystemExit('the wide ribbon did not hide')

unreal.BattleTrailMode.apply(world, False)
arcade_widths, arcade_ribbons = report('arcade again')
if list(arcade_widths) != [420.0] or arcade_ribbons['arcade']['visible'] != arcade_ribbons['arcade']['actors']:
    raise SystemExit('the swap did not come back to arcade')
unreal.log_error('TRAIL RIBBONS VERIFIED: both ribbons in the map, arcade 420 cm, realistic 320 cm, swap both ways')
