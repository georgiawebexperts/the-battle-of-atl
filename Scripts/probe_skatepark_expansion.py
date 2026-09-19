"""Does the bigger Fourth Ward skatepark still fit on the ground it stands on?

Elliott: "the skate park should be way bigger and more fun its way too small".
The park is baked geometry, so before anything is cut this answers three
questions about the shape in Scripts/skatepark_geometry.py, offline, against the
same retained DEM the baker reads:

1. Does the deck stay at 650? The rule is deck = max(ground - feature) + 18,
   so a bowl dug where the ground is high pushes the whole deck up - and 650 is
   what the bonus pickups, the east ramp to the trail, the four surface probes
   in BattleSkateAudit and the placement probes in import_skatepark.py assume.
2. How much headroom does each feature have - how far is its lowest point above
   the ground under it? A negative number is a feature buried in the terrain.
3. Are the audit's lanes still clear? The audit drives an entry lane and an
   exit lane along local y = 168, and pins four surface heights; new geometry
   in either lane is a broken audit, not a surprise.

Read-only. Writes work/skatepark-expansion-terrain.json.
"""
import json
import math
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from skatepark_geometry import (  # noqa: E402
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
    VOLCANOES,
    feature,
    footprint,
    inside,
)

root = Path(__file__).resolve().parents[1]
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


STEP = 50
x0, x1, y0, y1 = footprint()
xs = np.arange(x0, x1 + STEP, STEP)
ys = np.arange(y0, y1 + STEP, STEP)
deck = (
    math.ceil(max(ground(x, y) - feature(x, y) + 18 for x in xs for y in ys if inside(x, y)) / 10)
    * 10
)


def box_stats(name, bx0, bx1, by0, by1, note=""):
    """Ground and feature stats over a box, and the headroom of the feature."""
    pts = [(x, y) for y in np.arange(by0, by1 + 1, STEP) for x in np.arange(bx0, bx1 + 1, STEP)]
    g = [ground(x, y) for x, y in pts]
    f = [feature(x, y) for x, y in pts]
    low = min(f)
    moved = [a - b for a, b in zip(g, f)]
    return {
        "feature": name,
        "note": note,
        "local_box_cm": [bx0, bx1, by0, by1],
        "ground_min": round(min(g), 1),
        "ground_max": round(max(g), 1),
        "feature_low_cm": round(low, 1),
        "feature_high_cm": round(max(f), 1),
        "headroom_cm": round(DECK + low - max(g), 1),
        "deck_would_need": round(math.ceil((max(moved) + 18) / 10) * 10, 1),
    }


rows = [
    box_stats(b[0], b[1] - b[3], b[1] + b[3], b[2] - b[4], b[2] + b[4], "bowl, dug down")
    for b in BOWLS
]
rows += [
    box_stats(k[0], k[2], k[5], k[1] - k[7], k[1] + k[7], "kicker, built up") for k in KICKERS
]
rows += [
    box_stats(q[0], q[1], q[2], min(q[3], q[4]), max(q[3], q[4]), "quarter pipe, built up")
    for q in QUARTERS
]
rows += [box_stats(r[0], r[2], r[3], r[1] - r[5], r[1] + r[5], "rollers, built up") for r in ROLLERS]
rows += [
    box_stats(p[0], p[1] - p[3], p[1] + p[3], p[2] - p[4], p[2] + p[4], "pyramid, built up")
    for p in PYRAMIDS
]
rows += [
    box_stats(v[0], v[1] - v[3], v[1] + v[3], v[2] - v[3], v[2] + v[3], "volcano, built up")
    for v in VOLCANOES
]
rows += [box_stats(p[0], p[1], p[2], p[3], p[4], "pad, built up") for p in PADS]

# The audit's lanes and its pinned heights, computed from the geometry itself.
pins = {
    "flat_deck_at_park_centre_cm": [0, 0, DECK + feature(0, 0)],
    "launch_bank_top_cm": [1100, -500, DECK + feature(1100, -500)],
    "bowl_south_floor_cm": [-950, -350, DECK + feature(-950, -350)],
    "bowl_north_floor_cm": [-950, 400, DECK + feature(-950, 400)],
    "cauldron_floor_cm": [-2600, -1500, DECK + feature(-2600, -1500)],
    "pocket_bowl_floor_cm": [-1900, -2050, DECK + feature(-1900, -2050)],
    "pyramid_cap_cm": [500, 1750, DECK + feature(500, 1750)],
}
lanes = {}
for name, (lx0, lx1, ly) in {
    "entry_and_exit_lane": (600, 2900, 168),
    "west_return_lane": (-3300, -1800, 1100),
}.items():
    bumps = [(x, round(feature(x, ly), 1)) for x in range(lx0, lx1 + 1, 50) if abs(feature(x, ly)) > 0.5]
    lanes[name] = {"local_cm": [lx0, lx1, ly], "bumps": bumps[:12], "bump_count": len(bumps), "clear": not bumps}

fill = [DECK - ground(x, y) for y in ys for x in xs if inside(x, y)]
out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": "Does the bigger skatepark still fit the retained ground, and does it "
    "still keep the audit's lanes and pinned heights?",
    "deck_z_needed": float(deck),
    "deck_z_expected": DECK,
    "footprint_cm": [x0, x1, y0, y1],
    "footprint_m": [round((x1 - x0) / 100, 1), round((y1 - y0) / 100, 1)],
    "footprint_area_m2": round(sum((b[2] - b[1]) * (b[4] - b[3]) for b in BLOCKS) / 10000, 0),
    "installed_area_m2": 864.0,
    "fill_cm": {"min": round(min(fill), 1), "mean": round(sum(fill) / len(fill), 1), "max": round(max(fill), 1)},
    "features": rows,
    "pinned_heights": {k: {"local_cm": v[:2], "surface_z_cm": round(v[2], 1)} for k, v in pins.items()},
    "audit_lanes": lanes,
    "buried_features": [r["feature"] for r in rows if r["headroom_cm"] < 0],
    "features_that_would_raise_the_deck": [r["feature"] for r in rows if r["deck_would_need"] > DECK],
}
(root / "work/skatepark-expansion-terrain.json").write_text(json.dumps(out, indent=2) + "\n")

print(f"deck: needs {out['deck_z_needed']}, audit and pickups expect {DECK}")
print(
    f"footprint: {out['footprint_m'][0]} x {out['footprint_m'][1]} m = "
    f"{out['footprint_area_m2']} m2 (installed {out['installed_area_m2']} m2)"
)
print(f"fill over the ground: min {out['fill_cm']['min']} mean {out['fill_cm']['mean']} max {out['fill_cm']['max']} cm")
print(f"{'feature':30s} {'ground':>15s} {'low':>8s} {'high':>7s} {'headroom':>9s} {'deck':>7s}")
for r in rows:
    print(
        f"{r['feature']:30s} {r['ground_min']:7.0f}..{r['ground_max']:<7.0f} "
        f"{r['feature_low_cm']:8.0f} {r['feature_high_cm']:7.0f} {r['headroom_cm']:9.0f} {r['deck_would_need']:7.0f}"
    )
print("pinned:", json.dumps({k: round(v["surface_z_cm"], 1) for k, v in out["pinned_heights"].items()}))
for name, lane in lanes.items():
    if lane["clear"]:
        print(f"{name}: clear")
    else:
        print(f"{name}: {lane['bump_count']} bumps, first {lane['bumps'][:6]}")
print("buried features:", out["buried_features"] or "none")
print("would raise the deck:", out["features_that_would_raise_the_deck"] or "none")
