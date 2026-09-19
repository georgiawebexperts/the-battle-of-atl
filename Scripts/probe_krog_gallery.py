"""What is over the Krog route either side of the bore, in the saved world?

The offline half (krog_gallery_stations.py) writes the stations and the authored
deck height; this reads them and asks the world: is the road actually where the
profile says, how high is the ground beside it, and is anything standing in the
space a gallery's roof would occupy? A gallery is only honest where the road is
already below its own roof and nothing else is in that space.

Read-only. Writes work/krog-gallery-probe.json and prints one line per station.
"""
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
stations = json.loads((root / "work/krog-gallery-stations.json").read_text())

assert unreal.EditorLoadingAndSavingUtils.load_map("/Game/PiedmontRide/Maps/PiedmontWorld")
unreal.PiedmontWorldTools.finish_editor_asset_loading()


def trace(start, end, radius=0.0):
    hit = unreal.PiedmontWorldTools.trace_world_surface(start, end, radius)
    if not hit:
        return None
    return {
        "z": round(hit[0].z, 1),
        "actor": hit[1].get_actor_label() if hit[1] else "none",
    }


rows = []
for station in stations["stations"]:
    if station["inside_bore"]:
        continue
    cx, cy = station["centreline_cm"]
    deck = station["deck_z_cm"]
    nx, ny = station["normal_xy"]
    # The stations are retained source ENU (east/north/up); the world is ESU, so
    # north is -Y (battle_geography.source_to_world). Tracing source coordinates
    # directly reads nothing but sky, which is how the first pass of this probe
    # came back all "sky" - the same trap the OBJ writers already carry.
    wx, wy, wnx, wny = cx, -cy, nx, -ny
    row = {
        "side": station["side"],
        "station_cm": station["station_cm"],
        "metres_past_portal": round(
            abs(station["station_cm"] - (stations["bore_stations_cm"][1 if station["side"] == "north" else 0])) / 100,
            1,
        ),
        "deck_z_cm": deck,
        "probes": {},
    }
    for name, lateral in [
        ("centre", 0.0),
        ("wall_west", -640.0),
        ("wall_east", 360.0),
        ("roof_west", -660.0),
        ("roof_east", 380.0),
        # Where an earth cover over the roof would have to meet the ground.
        ("berm_west", -1300.0),
        ("berm_east", 1000.0),
    ]:
        x, y = wx + wnx * lateral, wy + wny * lateral
        below = trace(unreal.Vector(x, y, deck + 900), unreal.Vector(x, y, deck - 900))
        # Under a lid the first hit from above is the gallery roof, so the road
        # has to be found from below it: start under the roof slab (deck+250)
        # and trace down. Without this the "road" column reports the lid.
        road = trace(unreal.Vector(x, y, deck + 250), unreal.Vector(x, y, deck - 500))
        above = trace(unreal.Vector(x, y, deck + 50), unreal.Vector(x, y, deck + 1200))
        row["probes"][name] = {
            "surface": below,
            "surface_over_deck_cm": round(below["z"] - deck, 1) if below else None,
            "road_over_deck_cm": round(road["z"] - deck, 1) if road else None,
            "road_actor": road["actor"] if road else None,
            "first_above": above,
            "clear_height_cm": round(above["z"] - deck, 1) if above else None,
        }
    # A rider sphere standing on the road: centred 110 cm above the road so the
    # 62 cm sphere clears it (the authored deck runs 19-50 cm below the road,
    # which is what made an earlier version of this check read BLOCKED under
    # every station where the road sits low).
    centre = row["probes"]["centre"]
    rider_z = deck + (centre["road_over_deck_cm"] or 0.0) + 110
    row["rider_fits_on_road"] = (
        trace(unreal.Vector(wx, wy, rider_z), unreal.Vector(wx, wy, rider_z), 62.0) is None
    )
    row["covered"] = centre["clear_height_cm"] is not None
    rows.append(row)

out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": stations["question"],
    "bore_stations_cm": stations["bore_stations_cm"],
    "route_line_length_cm": stations["route_line_length_cm"],
    "roof_plan": "walls at lateral -640/+360, roof slab deck+270..deck+310, roof out to -660/+380",
    "stations": rows,
}
(root / "work/krog-gallery-probe.json").write_text(json.dumps(out, indent=2) + "\n")

for row in rows:
    c = row["probes"]["centre"]
    w = row["probes"]["wall_west"]
    e = row["probes"]["wall_east"]
    print(
        f"{row['side']:>5} st={row['station_cm']:>8.0f} (+{row['metres_past_portal']:>4.1f} m past portal)  "
        f"road={c['surface_over_deck_cm']:>7}  clear_above={c['clear_height_cm']:>7} at {(c['first_above'] or {}).get('actor', 'sky')}  "
        f"ground_west={w['surface_over_deck_cm']:>7} ground_east={e['surface_over_deck_cm']:>7}  "
        f"rider={'fits' if row['rider_fits_on_road'] else 'BLOCKED'}"
    )
unreal.SystemLibrary.quit_editor()
