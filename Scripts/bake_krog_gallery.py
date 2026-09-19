"""Bake a cut-and-cover gallery over the Krog approach, south of the bore.

Elliott, twice: "are you also working to extend the tunnel it is way too short"
and "finish krog street tunnel". The bore is 4221 cm of route. North of the
trail portal the route *ends* 411 cm later, so the only stretch with room is the
south approach toward DeKalb, and the world already answers what is over it
(probe_krog_gallery.py): the road sits in an excavated trench whose sides rise
1.4-3.5 m west and 1.2-7.9 m east ten metres out, with open sky directly above
the road. That is a trench waiting for a lid - the definition of cut-and-cover -
so this bakes one:

  Shell     the bore's own cross-section (roof slab deck+270..+310 out to
            lateral -660/+380, side walls at -640 and +360 down to deck-32)
            continued south from the existing portal, capped only at the new
            mouth so it joins the old portal without a seam.
  Columns   the same internal columns, every 280 cm, on the same line.
  Cover     an earth/grass slab from the roof edge out to where the ground
            already stands, so the lid is buried and reads as ground rather
            than as a box standing in the open.

The shell is built off the *measured* road surface from the world probe, not the
authored profile: outside the bore the authored profile runs 19-50 cm below the
road that is actually there, and a roof placed off the plan would sag into the
riding line.

No terrain is edited and the riding surface is untouched - everything here is
above and beside the road. Offline; writes SourceAssets/Terrain/KrogGallery/.
"""
import argparse
import json
import math
import sys
from pathlib import Path

import numpy as np

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / "Scripts"))
from route_height_profiles import RouteHeightProfiles  # noqa: E402

parser = argparse.ArgumentParser()
parser.add_argument("--extension-cm", type=float, default=4000.0)
parser.add_argument("--overlap-cm", type=float, default=240.0)
parser.add_argument("--station-step-cm", type=float, default=120.0)
args = parser.parse_args()

profiles = RouteHeightProfiles(root / "SourceAssets/Terrain/krog-height-profiles.json")
line = profiles.line
profile = profiles.profiles[0]
portal = profile["bridge_start_cm"]
probe = json.loads((root / "work/krog-gallery-probe.json").read_text())

# Measured surfaces, per probe station, keyed by the lateral's name.
measured = {}
for row in probe["stations"]:
    deck = row["deck_z_cm"]
    surfaces = {}
    for name, data in row["probes"].items():
        # `road_over_deck_cm` is the trace made from under the roof slab, which
        # is the one that still finds the *road* once a lid is installed.
        # `surface_over_deck_cm` finds the lid itself there, and reading it as
        # the road is how a re-bake after installation put the whole gallery
        # 310 cm into the air.
        delta = data.get("road_over_deck_cm", data["surface_over_deck_cm"])
        if delta is None:
            delta = data["surface_over_deck_cm"]
        surfaces[name] = None if delta is None else deck + delta
    measured[row["station_cm"]] = surfaces
known = sorted(measured)


def surface_at(station, name, fallback_delta):
    """Measured surface z at a station, interpolated between probe stations."""
    if station <= known[0]:
        value = measured[known[0]][name]
    elif station >= known[-1]:
        value = measured[known[-1]][name]
    else:
        value = None
        for a, b in zip(known, known[1:]):
            if a <= station <= b:
                va, vb = measured[a][name], measured[b][name]
                if va is None and vb is None:
                    value = None
                elif va is None or vb is None:
                    value = va if vb is None else vb
                else:
                    value = va + (vb - va) * (station - a) / (b - a)
                break
    if value is None:
        return deck_at(station) + fallback_delta
    return value


def deck_at(station):
    fraction = (station - profile["blend_start_cm"]) / (
        profile["blend_end_cm"] - profile["blend_start_cm"]
    )
    return profile["start_z_cm"] + (profile["end_z_cm"] - profile["start_z_cm"]) * fraction


def point(station, lateral, z):
    """Route station + lateral offset + absolute z, in retained source ENU."""
    position = line.interpolate(station)
    ahead = line.interpolate(min(station + 1.0, line.length)).coords[0]
    behind = line.interpolate(max(station - 1.0, 0.0)).coords[0]
    dx, dy = ahead[0] - behind[0], ahead[1] - behind[1]
    length = math.hypot(dx, dy)
    nx, ny = -dy / length, dx / length
    return np.array(
        [position.coords[0][0] + nx * lateral, position.coords[0][1] + ny * lateral, z]
    )


def quad(mesh, vertices):
    n = len(mesh["v"])
    mesh["v"].extend([list(v) for v in vertices])
    mesh["f"].extend([[n, n + 1, n + 2], [n, n + 2, n + 3]])


def beam(mesh, a, b, width, depth):
    a, b = np.array(a), np.array(b)
    d = b - a
    side = np.array([-d[1], d[0], 0.0])
    side = side / np.linalg.norm(side) * width / 2
    top = [a - side, a + side, b + side, b - side]
    low = [v - np.array([0, 0, depth]) for v in top]
    quad(mesh, list(reversed(top)))
    quad(mesh, low)
    for i in range(4):
        j = (i + 1) % 4
        quad(mesh, [top[i], low[i], low[j], top[j]])


stations = list(
    np.arange(portal - args.extension_cm, portal + args.overlap_cm + 0.1, args.station_step_cm)
)
if stations[-1] < portal + args.overlap_cm:
    stations.append(portal + args.overlap_cm)
fine = list(np.arange(portal - args.extension_cm, portal + args.overlap_cm + 0.1, 60.0))

meshes = {name: {"v": [], "f": []} for name in ("Shell", "Columns", "Cover")}

# --- Shell: roof slab + side walls, one segment per station pair.
for a, b in zip(fine, fine[1:]):
    road_a, road_b = surface_at(a, "centre", 0.0), surface_at(b, "centre", 0.0)
    roof_top = [
        point(a, -660, road_a + 310),
        point(a, 380, road_a + 310),
        point(b, 380, road_b + 310),
        point(b, -660, road_b + 310),
    ]
    roof_low = [v - np.array([0, 0, 40]) for v in roof_top]
    quad(meshes["Shell"], list(reversed(roof_top)))
    quad(meshes["Shell"], roof_low)
    for i in range(4):
        j = (i + 1) % 4
        quad(meshes["Shell"], [roof_top[i], roof_low[i], roof_low[j], roof_top[j]])
    for side in (-640.0, 360.0):
        wo_a, wo_b = point(a, side - 20, road_a - 32), point(b, side - 20, road_b - 32)
        wi_a, wi_b = point(a, side + 20, road_a - 32), point(b, side + 20, road_b - 32)
        wt_a, wt_b = point(a, side - 20, road_a + 270), point(b, side - 20, road_b + 270)
        wti_a, wti_b = point(a, side + 20, road_a + 270), point(b, side + 20, road_b + 270)
        quad(meshes["Shell"], [wo_a, wo_b, wti_b, wti_a])
        quad(meshes["Shell"], [wi_a, wi_b, wt_b, wt_a])
        quad(meshes["Shell"], [wo_a, wi_a, wi_b, wo_b])
        quad(meshes["Shell"], [wt_a, wt_b, wti_b, wti_a])

# --- Columns on the bore's own line, every 280 cm.
for station in np.arange(stations[0] + 80, stations[-1] - 80, 280):
    road = surface_at(station, "centre", 0.0)
    beam(meshes["Columns"], point(station, -318, road + 282), point(station, -282, road + 282), 36, 282)

# --- Earth cover: roof edge out to the ground that already stands there.
cover_rows = []
for station in stations:
    road = surface_at(station, "centre", 0.0)
    west = max(surface_at(station, "berm_west", -20.0), road - 500)
    east = min(surface_at(station, "berm_east", -80.0), road + 320)
    cover_rows.append(
        [
            point(station, -1300, west),
            point(station, -700, road + 320),
            point(station, 380, road + 320),
            point(station, 1000, east),
        ]
    )
for row_a, row_b in zip(cover_rows, cover_rows[1:]):
    for i in range(3):
        quad(meshes["Cover"], [row_a[i], row_a[i + 1], row_b[i + 1], row_b[i]])
# Skirts down the outer edges and across both ends. 60 cm, not 200: a 200 cm
# fascia across the ends hung down to road+120 and caught a rider sphere at the
# new mouth (the probe read BLOCKED there), and the cover's underside is above
# the shell's roof slab anyway, so a shallow skirt is all that is needed to stop
# the slab reading as paper-thin from outside.
SKIRT = np.array([0, 0, 60])
for row, flip in ((cover_rows[0], False), (cover_rows[-1], True)):
    for i in range(3):
        a, b = row[i], row[i + 1]
        quad(
            meshes["Cover"],
            [a, b, b - SKIRT, a - SKIRT][:: -1 if flip else 1],
        )
for index in (0, 3):
    for row_a, row_b in zip(cover_rows, cover_rows[1:]):
        a, b = row_a[index], row_b[index]
        quad(meshes["Cover"], [a, b, b - SKIRT, a - SKIRT])

out_dir = root / "SourceAssets/Terrain/KrogGallery"
out_dir.mkdir(parents=True, exist_ok=True)
manifest = {
    "author": "2026-09-19 [codex-maclaptop]",
    "purpose": "Cut-and-cover gallery extending the Krog bore south toward DeKalb",
    "extension_cm": args.extension_cm,
    "portal_station_cm": portal,
    "station_step_cm": args.station_step_cm,
    "stations_cm": [round(float(s), 1) for s in stations],
    "cross_section": {
        "roof_slab": "road+270..road+310, lateral -660..+380",
        "walls": "lateral -640/-620 and +340/+360, road-32..road+270",
        "columns": "lateral -318..-282, every 280 cm, road..road+282",
        "cover": "earth from the roof edge to the measured ground at -1300/+1000",
    },
    "deck_source": "measured road surface from work/krog-gallery-probe.json",
    "chunks": [],
}
for name, mesh in meshes.items():
    vertices = np.array(mesh["v"])
    lines = [f"# Authored cut-and-cover gallery {name}; ODbL route alignment", f"o KrogGallery_{name}"]
    lines += ["v %.6f %.6f %.6f" % (x, -y, z) for x, y, z in vertices]
    lines += ["vt %.6f %.6f" % (x / 200, z / 200) for x, y, z in vertices]
    lines += ["f " + " ".join(f"{i + 1}/{i + 1}" for i in reversed(face)) for face in mesh["f"]]
    (out_dir / f"KrogGallery_{name}.obj").write_text("\n".join(lines) + "\n")
    manifest["chunks"].append(
        {
            "file": f"KrogGallery_{name}.obj",
            "material": "Grass" if name == "Cover" else "Concrete",
            "structure": name,
            "triangles": len(mesh["f"]),
            "vertices": len(mesh["v"]),
            "bounds_cm": [vertices.min(axis=0).tolist(), vertices.max(axis=0).tolist()],
        }
    )
(out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
print(
    json.dumps(
        {
            "extension_m": args.extension_cm / 100,
            "stations": len(stations),
            "chunks": {c["structure"]: {"tris": c["triangles"], "verts": c["vertices"]} for c in manifest["chunks"]},
            "shell_bounds": [[round(v, 1) for v in row] for row in manifest["chunks"][0]["bounds_cm"]],
        },
        indent=2,
    )
)
