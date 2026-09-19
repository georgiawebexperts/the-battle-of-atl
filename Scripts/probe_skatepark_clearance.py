"""What is already standing where a bigger Fourth Ward skatepark would go?

Companion to probe_skatepark_expansion.py, which measures the retained ground.
The ground may be fine and the park still wrong: the deck is 650 cm of baked
concrete, so anything inside the footprint is either buried under it or pokes
through it. This walks the level's actors and reports, for the proposed
footprint, what sits in the deck's vertical window, class by class, tallest
first - buildings, trees, benches, cars, poles, the Eastside trail itself.

Read-only. Prints a summary and writes work/skatepark-expansion-clearance.json.
"""
import json
import sys
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
sys.path.insert(0, str(root / "Scripts"))
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# The proposed shape is whatever Scripts/skatepark_geometry.py says it is. It
# used to be a copy typed in here, which can disagree with the baker the moment
# anyone edits the shape - the one home is the point of that file.
from skatepark_geometry import BLOCKS, CX, CY, DECK  # noqa: E402

SKIP_CLASSES = {"Landscape", "WorldSettings", "Brush", "SkySphereBlueprint_C", "DirectionalLight"}

assert unreal.EditorLoadingAndSavingUtils.load_map("/Game/PiedmontRide/Maps/PiedmontWorld")
unreal.PiedmontWorldTools.finish_editor_asset_loading()


def block_of(x, y):
    for name, x0, x1, y0, y1 in BLOCKS:
        if x0 <= x <= x1 and y0 <= y <= y1:
            return name
    return None


PARK_TAG = unreal.Name("BattleSkatepark")
park = [a for a in ea.get_all_level_actors() if PARK_TAG in list(a.tags)]
park_names = {a.get_name() for a in park}
print(f"skatepark actors in the level: {sorted(park_names)}")

rows = []
for a in ea.get_all_level_actors():
    cls = a.get_class().get_name()
    if cls in SKIP_CLASSES or a.get_name() in park_names:
        continue
    try:
        origin, extent = a.get_actor_bounds(False)
    except Exception:
        continue
    lx, ly = origin.x - CX, origin.y - CY
    ex, ey = extent.x, extent.y
    hits = [
        name
        for name, x0, x1, y0, y1 in BLOCKS
        if lx + ex > x0 and lx - ex < x1 and ly + ey > y0 and ly - ey < y1
    ]
    if not hits:
        continue
    top, bottom = origin.z + extent.z, origin.z - extent.z
    # Only what shares the deck's vertical window: buried just under it, poking
    # through it, or standing on it.
    if top < DECK - 400 or bottom > DECK + 3000:
        continue
    rows.append(
        {
            "actor": a.get_name(),
            "class": cls,
            "label": a.get_actor_label(),
            "tags": [str(t) for t in a.tags],
            "blocks": hits,
            "local_xy_cm": [round(lx, 1), round(ly, 1)],
            "xy_extent_cm": [round(ex, 1), round(ey, 1)],
            "z_range_cm": [round(bottom, 1), round(top, 1)],
            "top_over_deck_cm": round(top - DECK, 1),
        }
    )

rows.sort(key=lambda r: -r["top_over_deck_cm"])
by_class = {}
for r in rows:
    by_class.setdefault(r["class"], []).append(r)

print(f"actors inside the proposed footprint: {len(rows)} in {len(by_class)} classes")
for cls, group in sorted(by_class.items(), key=lambda kv: -len(kv[1])):
    tops = [g["top_over_deck_cm"] for g in group]
    print(
        f"  {cls:34s} n={len(group):4d}  top_over_deck min={min(tops):8.1f} max={max(tops):8.1f}  "
        f"blocks={sorted({b for g in group for b in g['blocks']})}"
    )
print("tallest 25:")
for r in rows[:25]:
    print(
        f"  {r['top_over_deck_cm']:8.1f}  {r['class']:28s} {r['label'][:52]:52s} {r['blocks']} {r['local_xy_cm']}"
    )

ground_module = unreal.PiedmontWorldTools
grid = []
for name, x0, x1, y0, y1 in BLOCKS:
    for y in range(y0, y1 + 1, 400):
        for x in range(x0, x1 + 1, 400):
            down = ground_module.trace_world_surface(
                unreal.Vector(CX + x, CY + y, DECK + 600), unreal.Vector(CX + x, CY + y, DECK - 2400)
            )
            up = ground_module.trace_world_surface(
                unreal.Vector(CX + x, CY + y, DECK - 400), unreal.Vector(CX + x, CY + y, DECK + 2500)
            )
            grid.append(
                {
                    "block": name,
                    "local_xy_cm": [x, y],
                    "down_z": round(down[0].z, 1) if down else None,
                    "down_actor": down[1].get_actor_label() if down else None,
                    "up_z": round(up[0].z, 1) if up else None,
                    "up_actor": up[1].get_actor_label() if up else None,
                }
            )

out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": "What stands inside the proposed bigger skatepark footprint?",
    "deck_z": DECK,
    "blocks": [
        {"name": n, "local_cm": [x0, x1, y0, y1]} for n, x0, x1, y0, y1 in BLOCKS
    ],
    "actor_count": len(rows),
    "classes": {k: len(v) for k, v in sorted(by_class.items())},
    "actors": rows,
    "grid_step_cm": 400,
    "grid": grid,
}
(root / "work/skatepark-expansion-clearance.json").write_text(json.dumps(out, indent=2) + "\n")
print("wrote work/skatepark-expansion-clearance.json")
unreal.SystemLibrary.quit_editor()
