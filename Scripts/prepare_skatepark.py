"""Bake bike-scale bowls, ramps and street geometry beside the mapped skatepark.

Original geometry; OSM determines position, official aerial guides arrangement.
Retain underlying DEM unchanged; grade a surrounding berm down to existing ground.

The shape itself lives in Scripts/skatepark_geometry.py, because it grew: what
used to be a 36 x 24 m pad - two bowls, one launch bank and a manual pad - is
now roughly 51 x 46 m (2.6x the area) of blocks, five bowls, two launch banks,
a quarter pipe, a pyramid, a volcano, pump rollers, banks and ledges. This
script only turns that definition into the two OBJ files and the manifest the
importer reads.

The deck height is an assertion, not a calculation to be trusted: 650 cm is
what the three bonus pickups, the east ramp to the Eastside trail, the four
surface probes in BattleSkateAudit and import_skatepark.py's placement probes
all assume. A feature deep enough to raise the deck fails the bake here rather
than quietly moving the floor of the world out from under them.
"""
import json
import math
import sys
from pathlib import Path

import numpy as np
from shapely.geometry import MultiLineString, Point

sys.path.insert(0, str(Path(__file__).resolve().parent))
from skatepark_geometry import (  # noqa: E402
    BLEND,
    BLOCKS,
    BOWLS,
    CX,
    CY,
    DECK,
    KICKERS,
    PADS,
    PYRAMIDS,
    QUARTERS,
    ROLLERS,
    TRAIL,
    VOLCANOES,
    feature,
    footprint,
    inside,
    outside_gap,
)

root = Path(__file__).resolve().parents[1]
out = root / "SourceAssets/Skatepark"
out.mkdir(exist_ok=True)
meta = json.loads((root / "SourceAssets/Terrain/terrain-georeference.json").read_text())
raw = np.fromfile(root / "SourceAssets/Terrain/atlanta-height-krog.r16", dtype="<u2").reshape(
    meta["size"][1], meta["size"][0]
)
loc, sc = meta["unreal_location_cm"], meta["unreal_scale"]


def ground(x, y):
    gx = (x + CX - loc[0]) / sc[0]
    gy = (-y - CY - loc[1]) / sc[1]
    ix, iy = int(gx), int(gy)
    dx, dy = gx - ix, gy - iy
    a, b, c, d = [
        (float(raw[j, i]) - 32768) * sc[2] / 128
        for i, j in [(ix, iy), (ix + 1, iy), (ix, iy + 1), (ix + 1, iy + 1)]
    ]
    return a + (b - a) * dx + (d - b) * dy if dx >= dy else a + (d - c) * dx + (c - a) * dy


step = 50
x0, x1, y0, y1 = footprint()
xs = np.arange(x0 - BLEND, x1 + BLEND + step, step)
ys = np.arange(y0 - BLEND, y1 + BLEND + step, step)

# The deck rule, exactly as it was first written: highest ground anywhere under
# the footprint, less the feature already standing on it, plus 18 cm of cover.
deck = max(ground(x, y) - feature(x, y) + 18 for x in xs for y in ys if inside(x, y))
deck = math.ceil(deck / 10) * 10
assert deck == DECK, (
    f"baked deck would be {deck}, but {DECK} is what the bonus pickups, the trail "
    f"ramp, import_skatepark.py's probes and BattleSkateAudit assume - a feature "
    f"is dug too deep for the ground under it"
)

# East connection joins the existing Eastside trail at verified source-network elevation.
network = json.loads((root / "SourceAssets/Terrain/eastside-trail-network.json").read_text())
trail_lines = MultiLineString(
    [[(p[0], -p[1]) for p in path["points_cm"]] for path in network["paths"]]
)
trail = TRAIL


def surface(x, y):
    if inside(x, y):
        return DECK + feature(x, y)
    blend = max(0.0, 1.0 - outside_gap(x, y))
    z = ground(x, y) + 2 + (DECK + feature(x, y) * blend - ground(x, y) - 2) * blend
    if x >= 1800 and abs(y - (trail[1] - CY)) < 250:
        t = min(1.0, (x - 1800) / (2500 - 1800))
        z = DECK + (trail[2] - DECK) * t
    return z


verts = [(float(x), float(y), surface(x, y)) for y in ys for x in xs]
groups = {"Concrete": [], "Berm": []}
nx = len(xs)
for j in range(len(ys) - 1):
    for i in range(nx - 1):
        x = (xs[i] + xs[i + 1]) / 2
        y = (ys[j] + ys[j + 1]) / 2
        concrete = inside(x, y) or (x >= 1800 and x <= 2550 and abs(y - (trail[1] - CY)) < 250)
        if not concrete and trail_lines.distance(Point(CX + x, CY + y)) < 230:
            continue
        a = j * nx + i + 1
        b = a + 1
        c = a + nx
        d = c + 1
        groups["Concrete" if concrete else "Berm"].extend([(a, b, d), (a, d, c)])

# Encode OBJ handedness explicitly: importer reverses Y; reverse winding too.
for name, faces in groups.items():
    lines = ["# Original Battle for the ATL skatepark " + name]
    lines += ["v %.6f %.6f %.6f" % (v[0], -v[1], v[2]) for v in verts]
    lines += ["vt %.6f %.6f" % (v[0] / 200, v[1] / 200) for v in verts]
    lines += ["f " + " ".join(f"{i}/{i}" for i in reversed(f)) for f in faces]
    (out / (name + ".obj")).write_text("\n".join(lines) + "\n")

manifest = {
    "origin_world": [CX, CY, 0],
    "deck_z": DECK,
    "trail_connection": list(trail),
    "grid_spacing_cm": step,
    "vertex_count": len(verts),
    "triangles": {k: len(v) for k, v in groups.items()},
    "bounds": [[min(v[k] for v in verts) for k in range(3)], [max(v[k] for v in verts) for k in range(3)]],
    "blocks": [{"name": n, "local_cm": [a, b, c, d]} for n, a, b, c, d in BLOCKS],
    "bowls": [{"name": b[0], "center_local_cm": list(b[1:3]), "radii_cm": list(b[3:5]), "depth_cm": b[5]} for b in BOWLS],
    "kickers": [{"name": k[0], "axis_local_y_cm": k[1], "run_cm": [k[2], k[5]], "height_cm": k[6]} for k in KICKERS],
    "quarters": [{"name": q[0], "span_local_x_cm": [q[1], q[2]], "edge_local_y_cm": q[4], "height_cm": q[5]} for q in QUARTERS],
    "rollers": [{"name": r[0], "span_local_x_cm": [r[2], r[3]], "height_cm": r[4]} for r in ROLLERS],
    "pyramids": [{"name": p[0], "center_local_cm": [p[1], p[2]], "height_cm": p[6]} for p in PYRAMIDS],
    "volcanoes": [{"name": v[0], "center_local_cm": [v[1], v[2]], "radius_cm": v[3], "height_cm": v[5]} for v in VOLCANOES],
    "pads": [{"name": p[0], "height_cm": p[6]} for p in PADS],
    "bonuses": [
        [CX - 950, CY - 350, 510],
        [CX - 950, CY + 400, 550],
        [CX + 1100, CY - 500, 990],
    ],
    "ramp_start": [CX + 100, CY - 500, DECK + 98],
    "bowl_start": [CX - 950, CY - 350, DECK - 180 + 98],
    "area_m2": round(sum((b[2] - b[1]) * (b[4] - b[3]) for b in BLOCKS) / 10000, 0),
    "policy": (
        "Original playable adaptation of mapped Fourth Ward skatepark. Five bowls, two launch "
        "banks, a south quarter pipe, north pyramid and volcano, west pump rollers, ledges and "
        "bank-to-banks over 2.6x the first footprint. Berm/entrance meet retained terrain. "
        "Landscape untouched."
    ),
}
(out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
print(json.dumps({k: manifest[k] for k in ("deck_z", "area_m2", "vertex_count", "triangles", "bounds")}))
