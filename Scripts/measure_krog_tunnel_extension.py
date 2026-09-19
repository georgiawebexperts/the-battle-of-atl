"""How far can the Krog bore be extended before it stops being underground?

Elliott: "are you also working to extend the tunnel it is way too short". The
bore is 4608 game cm standing in for a 127 m real tunnel, because the whole
world is built at 1:3 - so a rider crosses it in about five seconds.

Extending it is only clean where the bare earth is still above the tunnel roof:
there the longer bore is invisible from the surface and nothing has to move.
Where the terrain has already dropped to street level, an extension turns the
approach into a covered box with the street, sidewalks and buildings inside it,
which is a different and much larger job.

This measures both ends against the raw terrain raster and against the mapped
road network, and writes the numbers to Tests/Results.
"""
import json
from pathlib import Path

import numpy as np
from shapely.geometry import LineString
from shapely.ops import unary_union

root = Path(__file__).resolve().parents[1]
profile = json.loads((root / "SourceAssets/Terrain/krog-height-profiles.json").read_text())
p = profile["profiles"][0]
line = LineString(profile["centerline_xy_cm"])
meta = json.loads((root / "SourceAssets/Terrain/terrain-georeference.json").read_text())
raw = np.fromfile(root / "SourceAssets/Terrain/atlanta-height.r16", dtype="<u2")
raw = raw.reshape(meta["size"][1], meta["size"][0])
loc, sc = meta["unreal_location_cm"], meta["unreal_scale"]

ROOF_TOP_CM = 310.0  # shell ribbon top, from bake_krog_structure.py
SAMPLE_CM = 25.0


def floor_z(s):
    """Authored tunnel floor grade, the same line bake_krog_structure.py uses."""
    return p["start_z_cm"] + (p["end_z_cm"] - p["start_z_cm"]) * (
        (s - p["blend_start_cm"]) / (p["blend_end_cm"] - p["blend_start_cm"])
    )


def bare_earth_z(s):
    x, y = line.interpolate(s).coords[0]
    gx, gy = (x - loc[0]) / sc[0], (y - loc[1]) / sc[1]
    ix, iy = int(gx), int(gy)
    dx, dy = gx - ix, gy - iy
    z = [
        (float(raw[yy, xx]) - 32768) * sc[2] / 128
        for xx, yy in [(ix, iy), (ix + 1, iy), (ix, iy + 1), (ix + 1, iy + 1)]
    ]
    ground = (
        z[0] + (z[1] - z[0]) * dx + (z[3] - z[2]) * dy
        if dx >= dy
        else z[0] + (z[3] - z[1]) * dx + (z[2] - z[0]) * dy
    )
    return ground + 3


def cross_section(s):
    q = line.interpolate(s)
    a, b = line.interpolate(s - 1), line.interpolate(s + 1)
    dx, dy = b.x - a.x, b.y - a.y
    length = (dx * dx + dy * dy) ** 0.5
    if length == 0:  # interpolate() clamps past the ends of the centreline
        return None
    nx, ny = -dy / length, dx / length
    return LineString([(q.x + nx * v, q.y + ny * v) for v in [-660, 380]])


network = json.loads((root / "SourceAssets/Terrain/KrogTraffic/network.json").read_text())
roads = unary_union(
    [
        LineString([(q[0], -q[1]) for q in row["points_cm"]]).buffer(450, join_style=2)
        for row in network["roads"]
    ]
)

lo, hi = p["bridge_start_cm"], p["bridge_end_cm"]
rows = []
s = max(1.0, lo - 12000.0)
last = min(line.length - 1.0, hi + 4000.0)
while s <= last:
    roof = floor_z(s) + ROOF_TOP_CM
    ground = bare_earth_z(s)
    section = cross_section(s)
    rows.append(
        {
            "station_cm": round(s, 1),
            "floor_z_cm": round(floor_z(s), 1),
            "roof_top_cm": round(roof, 1),
            "bare_earth_cm": round(ground, 1),
            "cover_cm": round(ground - roof, 1),
            "clear_of_roads_cm": None if section is None else round(section.distance(roads), 1),
        }
    )
    s += SAMPLE_CM


def reach(direction):
    """Largest extension whose whole run stays buried and clear of roads."""
    best = 0.0
    step = lo if direction < 0 else hi
    while abs(step - (lo if direction < 0 else hi)) < 12000.0:
        step += direction * SAMPLE_CM
        row = next((r for r in rows if abs(r["station_cm"] - step) < SAMPLE_CM / 2), None)
        if row is None:
            break
        if row["cover_cm"] < 0:
            break
        if row["clear_of_roads_cm"] is None or row["clear_of_roads_cm"] < 80:
            break
        best = abs(step - (lo if direction < 0 else hi))
    return best


out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": "How far can the Krog bore extend and stay underground?",
    "current_bore_game_cm": round(hi - lo, 1),
    "roof_top_above_floor_cm": ROOF_TOP_CM,
    "extension_available_before_portal_cm": reach(-1),
    "extension_available_after_portal_cm": reach(+1),
    "samples": rows,
}
(root / "Tests/Results/2026-09-19-krog-tunnel-extension-reach.json").write_text(
    json.dumps(out, indent=2) + "\n"
)
print(
    json.dumps(
        {
            "current_bore_game_cm": out["current_bore_game_cm"],
            "extension_available_before_portal_cm": out["extension_available_before_portal_cm"],
            "extension_available_after_portal_cm": out["extension_available_after_portal_cm"],
        }
    )
)
print("station  cover_cm  clear_of_roads_cm")
for row in rows[::8]:
    print(f"{row['station_cm']:>9.0f} {row['cover_cm']:>8.0f} {row['clear_of_roads_cm']:>16.0f}")
