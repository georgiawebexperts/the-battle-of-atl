"""Walk the Krog route outside the bore and write down what is above it.

Elliott, twice: "are you also working to extend the tunnel it is way too short"
and "finish krog street tunnel". The bore is 4221 cm of route between
`bridge_start_cm` and `bridge_end_cm` in krog-height-profiles.json, and the only
cheap way to make the ride longer is to cover more of the route that already
exists either side of it - a cut-and-cover gallery, not a re-graded approach.

That is only possible where the road is already below what would become the
gallery's roof. This is the offline half: it samples the route either side of
each portal at the authored profile height and writes the stations for
probe_krog_gallery.py, which is the half that needs the world (terrain height,
obstructions, trench walls).
"""
import json
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / "Scripts"))
from route_height_profiles import RouteHeightProfiles  # noqa: E402

profiles = RouteHeightProfiles(root / "SourceAssets/Terrain/krog-height-profiles.json")
line = profiles.line
profile = profiles.profiles[0]
lo, hi = profile["bridge_start_cm"], profile["bridge_end_cm"]

REACH = 4500.0
STEP = 100.0
stations = []
for station, side in [
    *[(lo - d, "south") for d in range(0, int(REACH) + 1, int(STEP))],
    *[(hi + d, "north") for d in range(0, int(REACH) + 1, int(STEP))],
]:
    if station < 0 or station > line.length:
        continue
    point = line.interpolate(station)
    ahead = line.interpolate(min(station + 1.0, line.length)).coords[0]
    behind = line.interpolate(max(station - 1.0, 0.0)).coords[0]
    dx, dy = ahead[0] - behind[0], ahead[1] - behind[1]
    length = (dx * dx + dy * dy) ** 0.5
    normal = (-dy / length, dx / length)
    fraction = (station - profile["blend_start_cm"]) / (
        profile["blend_end_cm"] - profile["blend_start_cm"]
    )
    deck = profile["start_z_cm"] + (profile["end_z_cm"] - profile["start_z_cm"]) * fraction
    stations.append(
        {
            "side": side,
            "station_cm": round(station, 1),
            "inside_bore": lo <= station <= hi,
            "centreline_cm": [round(point.coords[0][0], 2), round(point.coords[0][1], 2)],
            "deck_z_cm": round(deck, 2),
            "normal_xy": [round(normal[0], 6), round(normal[1], 6)],
            # The shell's own cross-section: walls at -640 and +360, roof out to
            # -660 and +380, roof slab between deck+270 and deck+310.
            "wall_laterals_cm": [-640.0, 360.0],
            "roof_laterals_cm": [-660.0, 380.0],
            "roof_z_cm": round(deck + 310.0, 2),
            "wall_top_z_cm": round(deck + 270.0, 2),
        }
    )

out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": "How far past each portal could a cut-and-cover gallery run?",
    "route_line_length_cm": round(line.length, 1),
    "bore_stations_cm": [lo, hi],
    "bore_length_cm": round(hi - lo, 1),
    "step_cm": STEP,
    "reach_cm": REACH,
    "stations": stations,
}
(root / "work/krog-gallery-stations.json").write_text(json.dumps(out, indent=2) + "\n")
print(
    json.dumps(
        {
            "line_length_cm": out["route_line_length_cm"],
            "bore_cm": out["bore_length_cm"],
            "stations": len(stations),
            "south_reaches_station_cm": stations[0]["station_cm"],
            "north_reaches_station_cm": stations[-1]["station_cm"],
            "line_end_station_cm": round(line.length, 1),
        },
        indent=2,
    )
)
