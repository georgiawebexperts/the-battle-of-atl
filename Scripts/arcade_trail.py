"""Single source of truth for the authored arcade trail width.

The Eastside Trail and the park-to-BeltLine connector are both baked from this
number. It is a playability dimension, not a surveyed one: the world is 1:3 and
the player is unscaled, so real BeltLine widths would be unrideable.

2026-09-18 [codex-maclaptop]: raised from 320 to 420 game cm. Elliott rode the
trail and said "lets widen the beltlien path its a little too narrow". 320 baked
into the pavement chunks, and the same 320 is what APiedmontPathSpline::WidthCm
carries, so the grass rule starts where the pavement ends. Change it here, then
re-run prepare_eastside_trail.py, prepare_beltline_connector.py, the two
bake_park_pavement.py passes, bake_eastside_rails.py, and the two installers.
"""

TRAIL_WIDTH_GAME_CM = 420

# Guardrails sit just inside the pavement edge on the three authored bridges.
RAIL_INSET_CM = 6

# Offset of a guardrail from the centerline.
def rail_offset_cm():
    return TRAIL_WIDTH_GAME_CM / 2 - RAIL_INSET_CM


def width_policy(what):
    return (f'Authored {TRAIL_WIDTH_GAME_CM} game cm for arcade play; '
            f'raised from 320 on 2026-09-18 because the trail rode too narrow. '
            f'Not a surveyed width. {what}')
