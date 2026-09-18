"""Single source of truth for the authored trail widths.

The Eastside Trail and the park-to-BeltLine connector are both baked from these
numbers. They are playability dimensions, not surveyed ones: the world is 1:3 and
the player is unscaled, so real BeltLine widths would be unrideable.

2026-09-18 [codex-maclaptop]: raised the trail from 320 to 420 game cm. Elliott
rode it and said "lets widen the beltlien path its a little too narrow". That
widening was global, and he then asked whether it could follow the handling mode:
"i wonder if you are doing the arcade mode if the beltlien is wide and if you
are doing the pro mode or realistic mode it was like it was before?"

So there are now two authored widths, and the pavement is baked twice - the wide
arcade ribbon and the original realistic one. Exactly one of the two exists at a
time in the world: UBattleTrailMode::Apply hides and un-collides the other one
and rewrites APiedmontPathSpline::WidthCm, which is the number the BeltLine grass
rule reads to decide where the pavement ends.

Change a width here, then run Scripts/rebuild_trail_pavement.sh, then
install_eastside_trail.py and install_beltline_connector.py.
"""

TRAIL_WIDTH_ARCADE_CM = 420
TRAIL_WIDTH_REALISTIC_CM = 320

# The arcade ribbon is the one a freshly loaded level shows, so anything that
# wants a bare default gets the forgiving number.
TRAIL_WIDTH_GAME_CM = TRAIL_WIDTH_ARCADE_CM

# Guardrails sit just inside the pavement edge on the three authored bridges.
RAIL_INSET_CM = 6


# Offset of a guardrail from the centerline at an authored width.
def rail_offset_cm(width_cm=TRAIL_WIDTH_GAME_CM):
    return width_cm / 2 - RAIL_INSET_CM


def width_policy(what):
    return (f'Authored {TRAIL_WIDTH_ARCADE_CM} game cm for arcade play and '
            f'{TRAIL_WIDTH_REALISTIC_CM} game cm for realistic play, chosen at '
            f'runtime by handling mode. The 420 came from 2026-09-18 because the '
            f'trail rode too narrow; 320 is the original width realistic keeps. '
            f'Not a surveyed width. {what}')


def realistic_variant(source, out):
    """Copy a prepared network, re-authoring every path at the realistic width.

    The two ribbons are baked from two networks that are identical apart from
    width, so the narrow one is by construction the same alignment as the wide
    one and not a second, drifting survey of the route.
    """
    import json
    from pathlib import Path
    data = json.loads(Path(source).read_text())
    for path in data['paths']:
        path['width_game_cm'] = TRAIL_WIDTH_REALISTIC_CM
    data['width_game_cm'] = TRAIL_WIDTH_REALISTIC_CM
    data['widths_game_cm'] = {'arcade': TRAIL_WIDTH_ARCADE_CM,
                              'realistic': TRAIL_WIDTH_REALISTIC_CM}
    data['mode_variant'] = 'realistic'
    data['width_policy'] = width_policy(
        f'Narrow-mode variant of {Path(source).name}; baked and installed '
        f'alongside the wide ribbon.')
    out = Path(out)
    out.write_text(json.dumps(data, indent=2) + '\n')
    return out
