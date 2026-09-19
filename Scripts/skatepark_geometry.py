"""The Fourth Ward skatepark's shape, in one place.

Elliott: "the skate park should be way bigger and more fun its way too small".
The park is baked geometry, not a scaled actor, so the shape has to live in
exactly one definition that both the baker (prepare_skatepark.py) and the size
probe (probe_skatepark_expansion.py) read. This module is it.

Local frame: centimetres from the park actor at world (39000, 74000, 0). The
deck is 650 cm above world zero. That height is not free to move: three bonus
pickups, the east ramp down to the Eastside trail, the four surface probes in
BattleSkateAudit and the placement probes in import_skatepark.py all assume it.
The baker's rule (deck = max(ground - feature) + 18, rounded up to 10) therefore
has to keep landing on 650, which caps how deep anything may be dug where the
ground is high. probe_skatepark_expansion.py checks that and prints the
headroom per feature, so a bowl that would raise the deck is caught before it
is baked.

Everything over the park is one of five shapes:

  blocks     the footprint. Not one rectangle: the north and east stay as they
             are so the trail ramp and the audit's entry/exit lanes are
             untouched, and the new area goes west and south where the ground
             is lowest (205-330 cm, so bowls may be dug without raising the
             deck). A block bigger than the deck's fill limit would show as a
             raise in the probe.
  bowls      round or oval depressions, flat-bottomed, blended to the deck.
  kickers    a run-up, a flat top and a drop - the shape that throws a bike.
  quarters   a wall rising to a lip at the footprint's edge.
  pyramids   a raised hip with a flat cap, plus volcano cones and beveled pads
             and ledges, all positive.

The two bowls and the kicker that were already installed keep their centres,
radii, depths and heights exactly: they are what the audit and the three bonus
pickups are pinned to.
"""
import math

CX, CY = 39000.0, 74000.0
DECK = 650.0
TRAIL = (41703.21309731, 74167.72982629, 475.11337638)
BLEND = 600.0  # how far the berm takes to meet retained ground, cm

# x0, x1, y0, y1
BLOCKS = [
    ("core", -1800, 1800, -1200, 1200),        # as installed
    ("west", -3300, -1800, -1600, 1200),       # new: low ground, the bowls
    ("southwest", -3300, -1200, -2400, -1600), # new: lowest ground, deep bowl
    ("southeast", -1200, 1800, -2400, -1200),  # new: quarter pipe and street
    ("north", -1800, 1800, 1200, 2200),        # new: high ground, hips only
]

# cx, cy, rx, ry, depth
BOWLS = [
    ("Fourth Ward bowl", -950, -350, 650, 600, 180),   # as installed
    ("Fourth Ward bowl north", -950, 400, 600, 550, 140),  # as installed
    # The new bowls go west and south, where the retained ground is 200-250 cm
    # rather than the 440-610 cm of the north-west and north: the deck rule
    # leaves 380 cm of digging there before the deck would have to rise.
    ("The Cauldron", -2600, -1500, 640, 470, 300),     # new, deepest
    ("Little Cauldron", -3000, -2100, 280, 250, 180),  # new, deepest corner
    ("Pocket bowl", -1900, -2050, 460, 330, 130),      # new, transfer bowl
]

# y axis, x0, x_apex0, x_apex1, x1, height, half width, side fade
KICKERS = [
    ("Fourth Ward launch bank", -500, 250, 1000, 1200, 1350, 300, 380, 100),  # as installed
    # North half of the west block - clear of the Cauldron's rim, which reaches
    # y = -1030, and of bowl B, which ends at x = -350. Its axis is y = 800 so
    # the landing, roughly 700-1100 cm east of the lip, is flat deck or the
    # very shallow outer rim of bowl B rather than the inside of its wall.
    ("West launch bank", 800, -3300, -2320, -2200, -2050, 240, 300, 100),     # new
]

# x0, x1, y_inner, y_edge, height, side fade
QUARTERS = [
    # The lip sits exactly on the southeast block's south edge and on a bake
    # grid row (y = -2400), so the wall reaches its full height in the mesh
    # instead of being sampled off at 1032 cm between rows.
    ("South quarter pipe", -300, 1500, -1850, -2400, 430, 250),
]

# y axis, x0, x1, height, wavelength
ROLLERS = [
    ("West pump rollers", -700, -3300, -2150, 95, 340),
]

# cx, cy, half_x, half_y, cap, height  (cap is the flat top as a fraction)
PYRAMIDS = [
    ("North pyramid", 500, 1750, 800, 420, 0.15, 200),
]

# cx, cy, radius, cap, height  (cap is the flat top as a fraction)
VOLCANOES = [
    ("Northwest volcano", -1150, 1700, 720, 0.3, 240),
]

# x0, x1, y0, y1, bevel, height  (flat raised pad; the manual pad as installed)
PADS = [
    ("Manual pad", 300, 1300, 270, 830, 120, 70),      # as installed
    ("South plaza", -200, 900, -1750, -1450, 120, 55), # new flat ledge plaza
]

# x0, x1, y0, y1, bevel, height  (positive bank, one edge dead square)
BANKS = [
    ("Bank to bank, west", -3300, -2880, -640, 40, 220, 200),
    ("Bank to bank, east", -2600, -2180, -640, 40, 220, 200),
    ("Southeast landing bank", -1150, -650, -1500, -2050, 220, 170),
]


def _clamp01(v):
    return 0.0 if v < 0 else 1.0 if v > 1 else v


def inside(x, y):
    """Is this point on the baked deck?"""
    return any(x0 <= x <= x1 and y0 <= y <= y1 for _, x0, x1, y0, y1 in BLOCKS)


def outside_gap(x, y, blend=BLEND):
    """How far outside the footprint, in blend lengths (0 on the edge)."""
    best = None
    for _, x0, x1, y0, y1 in BLOCKS:
        gap = max(x0 - x, x - x1, y0 - y, y - y1, 0.0) / blend
        best = gap if best is None else min(best, gap)
    return 0.0 if best is None else best


def _bowl(x, y, cx, cy, rx, ry, depth):
    r = math.hypot((x - cx) / rx, (y - cy) / ry)
    if r >= 1:
        return 0.0
    return -depth * (0.5 + 0.5 * math.cos(math.pi * _clamp01((r - 0.25) / 0.75)))


def _kicker(x, y, y_axis, x0, x_apex0, x_apex1, x1, height, half, fade):
    if not (x0 < x < x1) or abs(y - y_axis) >= half:
        return 0.0
    side = _clamp01((half - abs(y - y_axis)) / fade)
    if x < x_apex0:
        h = height * ((x - x0) / (x_apex0 - x0)) ** 2
    elif x <= x_apex1:
        h = height
    else:
        h = height * (x1 - x) / (x1 - x_apex1)
    return h * side


def _quarter(x, y, x0, x1, y_inner, y_edge, height, fade):
    lo, hi = min(y_inner, y_edge), max(y_inner, y_edge)
    if not (x0 - fade <= x <= x1 + fade) or not (lo <= y <= hi):
        return 0.0
    t = _clamp01((y - y_inner) / (y_edge - y_inner))
    return height * t * t * min(1.0, (x - x0 + fade) / fade, (x1 + fade - x) / fade)


def _roller(x, y, y_axis, x0, x1, height, wavelength):
    if not (x0 <= x <= x1) or abs(y - y_axis) > wavelength * 0.5:
        return 0.0
    side = _clamp01((wavelength * 0.5 - abs(y - y_axis)) / (wavelength * 0.25))
    return height * side * 0.5 * (1 - math.cos(2 * math.pi * (x - x0) / wavelength))


def _pyramid(x, y, cx, cy, hx, hy, cap, height):
    dx, dy = abs(x - cx) / hx, abs(y - cy) / hy
    if dx >= 1 or dy >= 1:
        return 0.0
    m = max(dx, dy)
    t = _clamp01((m - cap) / (1 - cap))
    return height * (1 - t * t)


def _volcano(x, y, cx, cy, radius, cap, height):
    r = math.hypot(x - cx, y - cy) / radius
    if r >= 1:
        return 0.0
    t = _clamp01((r - cap) / (1 - cap))
    return height * (1 - t * t)


def _pad(x, y, x0, x1, y0, y1, bevel, height):
    if not (x0 <= x <= x1 and y0 <= y <= y1):
        return 0.0
    return height * min(1.0, (x - x0) / bevel, (x1 - x) / bevel, (y - y0) / bevel, (y1 - y) / bevel)


def _bank(x, y, x0, x1, y0, y1, bevel, height):
    if not (x0 <= x <= x1 and y0 <= y <= y1):
        return 0.0
    return height * min(1.0, (x - x0) / bevel, (x1 - x) / bevel, (y - y0) / bevel, (y1 - y) / bevel)


def feature(x, y):
    """Height of the park surface relative to the deck, in cm."""
    down = min([0.0] + [_bowl(x, y, *b[1:]) for b in BOWLS])
    up = [
        0.0,
        max(_kicker(x, y, *k[1:]) for k in KICKERS),
        max(_quarter(x, y, *q[1:]) for q in QUARTERS),
        max(_roller(x, y, *r[1:]) for r in ROLLERS),
        max(_pyramid(x, y, *p[1:]) for p in PYRAMIDS),
        max(_volcano(x, y, *v[1:]) for v in VOLCANOES),
        max(_pad(x, y, *p[1:]) for p in PADS),
        max(_bank(x, y, *b[1:]) for b in BANKS),
    ]
    return down + max(up)


def footprint():
    """x0, x1, y0, y1 of the whole thing, footprint only."""
    return (
        min(b[1] for b in BLOCKS),
        max(b[2] for b in BLOCKS),
        min(b[3] for b in BLOCKS),
        max(b[4] for b in BLOCKS),
    )
