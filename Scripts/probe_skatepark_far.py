"""Does the park standing in the world match the park that was baked?

The second growth (the far west and the whole south strip: the Quarry pair, the
pump-track loop, the half-pipe, the two vert walls, the far west pump line) is
geometry that only exists as a mesh. BattleSkateAudit rides the core and the
west half; nothing rode the far half, so a mesh that failed to install would
have shown up as "the audit is green and the park is not there".

This compares the world against the asset instead of against a formula: it
reads the baked Concrete.obj, interpolates its surface at each named feature
point, traces the same point in the loaded world, and fails if they disagree.
That also makes it immune to the 50 cm bake grid - the mesh is the truth, and
the only question is whether the thing in the level is that mesh.

    UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file>

Read-only. Writes work/skatepark-far-probe.json.
"""
import json
import sys
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
sys.path.insert(0, str(root / "Scripts"))
from skatepark_geometry import CX, CY, DECK, feature  # noqa: E402

TOLERANCE_CM = 8.0


def load_baked():
    """The OBJ's grid: written row major over ys, one vertex per grid point."""
    xs, ys, zs = set(), set(), {}
    for line in (root / "SourceAssets/Skatepark/Concrete.obj").read_text().splitlines():
        if not line.startswith("v "):
            continue
        _, x, y, z = line.split()
        xs.add(float(x))
        ys.add(float(y))
        zs[(float(x), float(y))] = float(z)
    xs, ys = sorted(xs), sorted(ys)
    assert len(xs) * len(ys) == len(zs), f"not a grid: {len(xs)}x{len(ys)} vs {len(zs)}"
    return xs, ys, zs


XS, YS, ZS = load_baked()
STEP = XS[1] - XS[0]


def baked_z(x, y):
    """Bilinear on the baked grid. The OBJ negates y; the world does not."""
    gx, gy = (x - XS[0]) / STEP, (-y - YS[0]) / STEP
    ix, iy = int(gx), int(gy)
    dx, dy = gx - ix, gy - iy
    a, b = ZS[(XS[ix], YS[iy])], ZS[(XS[ix + 1], YS[iy])]
    c, d = ZS[(XS[ix], YS[iy + 1])], ZS[(XS[ix + 1], YS[iy + 1])]
    return a + (b - a) * dx + (d - b) * dy if dx >= dy else a + (d - c) * dx + (c - a) * dy


# Every point worth defending: the audit's own four pins, then one point on
# each second-growth feature (lip and floor where a feature has both).
POINTS = [
    ("core deck", 0, 0),
    ("Fourth Ward bowl floor", -950, -350),
    ("Fourth Ward bowl north floor", -950, 400),
    ("Fourth Ward launch bank top", 1100, -500),
    ("The Quarry floor", -4500, -3000),
    ("Little Quarry floor", -5300, -4000),
    ("The Loop berm", -1760, -3400),
    ("The Loop crest", -1808, -3155),
    ("Half-pipe north lip", -350, -2400),
    ("Half-pipe floor", -350, -3025),
    ("Half-pipe south lip", -350, -3650),
    ("West vert wall lip", -5650, 0),
    ("South vert wall lip", 1100, -4350),
    ("Far west pump crest", -4240, -1250),
    ("Far west pump trough", -4080, -1250),
]

# The park is not a placed actor: ABattleBike spawns it at (39000, 74000, 0) in
# BeginPlay, so the editor has never had one. (That is also why
# import_skatepark.py spawns its own actor to place its probes - the level keeps
# no park between sessions.) Spawn one here to ask the question, and do not save.
assert unreal.EditorLoadingAndSavingUtils.load_map("/Game/PiedmontRide/Maps/PiedmontWorld")
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
park = ea.spawn_actor_from_class(unreal.BattleSkatepark, unreal.Vector(CX, CY, 0))
assert park, "could not spawn ABattleSkatepark to probe"
concrete = park.get_editor_property("Concrete")
print(f"SkateparkFarProbe: spawned park, concrete mesh {concrete.get_editor_property('static_mesh').get_path_name()}")

rows = []
for name, x, y in POINTS:
    expect = baked_z(x, y)
    world = unreal.PiedmontWorldTools.trace_world_surface(
        unreal.Vector(CX + x, CY + y, expect + 900), unreal.Vector(CX + x, CY + y, expect - 900)
    )
    got = round(world[0].z, 1) if world else None
    rows.append(
        {
            "feature": name,
            "local_xy_cm": [x, y],
            "baked_z_cm": round(expect, 1),
            "world_z_cm": got,
            "delta_cm": None if got is None else round(got - expect, 1),
            "analytical_z_cm": round(DECK + feature(x, y), 1),
            "actor": world[1].get_actor_label() if world else None,
            "ok": got is not None and abs(got - expect) <= TOLERANCE_CM,
        }
    )

missing = [r["feature"] for r in rows if r["world_z_cm"] is None]
mismatched = [r["feature"] for r in rows if not r["ok"]]
SECOND_GROWTH = [name for name, _, _ in POINTS if name not in {p[0] for p in POINTS[:4]}]
out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": "Is the park standing in the world the park that was baked, "
    "including the second growth nothing has ridden yet?",
    "tolerance_cm": TOLERANCE_CM,
    "bake_grid_step_cm": STEP,
    "points": len(rows),
    "second_growth_points": SECOND_GROWTH,
    "missing": missing,
    "mismatched": mismatched,
    "rows": rows,
    "passed": not missing and not mismatched,
}
(root / "work/skatepark-far-probe.json").write_text(json.dumps(out, indent=2) + "\n")
print(
    "SkateparkFarProbe: "
    + json.dumps(
        {
            "passed": out["passed"],
            "points": out["points"],
            "missing": missing,
            "mismatched": mismatched,
            "worst_cm": max((abs(r["delta_cm"]) for r in rows if r["delta_cm"] is not None), default=0),
        }
    )
)
for r in rows:
    print(
        f"  {r['feature']:30s} baked {r['baked_z_cm']:8.1f}  world "
        f"{str(r['world_z_cm']):>8s}  delta {str(r['delta_cm']):>7s}  "
        f"{'ok' if r['ok'] else 'MISMATCH'}"
    )
unreal.SystemLibrary.quit_editor()
