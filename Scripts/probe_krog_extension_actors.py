"""Which actors own the Krog mouth: portal, columns, lights and the shell?

Before extending the tunnel south, find out what would end up *inside* the new
gallery - the portal structures, the downlights and the shell itself are all
separate actors in the saved map, and moving the mouth means moving them.

Read-only. Writes work/krog-extension-actors.json.
"""
import json
import math
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map("/Game/PiedmontRide/Maps/PiedmontWorld")
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

KROG = unreal.Vector(29782.0, 115813.0, 957.0)  # TunnelEntry, world ESU

rows = []
for actor in ea.get_all_level_actors():
    label = actor.get_actor_label()
    tags = [str(t) for t in actor.tags]
    if not (
        "Krog" in label
        or "krog" in label.lower()
        or any("Krog" in t for t in tags)
        or isinstance(actor, unreal.SpotLight)
        or isinstance(actor, unreal.PointLight)
    ):
        continue
    try:
        origin, extent = actor.get_actor_bounds(False)
    except Exception:
        continue
    rows.append(
        {
            "label": label,
            "class": actor.get_class().get_name(),
            "tags": tags,
            "location": [round(v, 1) for v in (origin.x, origin.y, origin.z)],
            "extent": [round(v, 1) for v in (extent.x, extent.y, extent.z)],
            "distance_to_entry_cm": round(
                math.dist((origin.x, origin.y, origin.z), (KROG.x, KROG.y, KROG.z)), 1
            ),
            "folder": str(actor.get_folder_path()),
        }
    )

rows.sort(key=lambda r: r["distance_to_entry_cm"])
lights = [r for r in rows if r["class"] in ("SpotLight", "PointLight")]
out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": "Which actors form the Krog tunnel mouth, and where are its lights?",
    "entry_world_cm": [KROG.x, KROG.y, KROG.z],
    "actor_count": len(rows),
    "lights": lights,
    "actors": rows,
}
(root / "work/krog-extension-actors.json").write_text(json.dumps(out, indent=2) + "\n")
print(f"actors near Krog: {len(rows)}, lights: {len(lights)}")
for r in rows[:40]:
    print(
        f"  {r['distance_to_entry_cm']:9.0f}  {r['class']:22s} {r['label'][:44]:44s} "
        f"{r['location']} extent={r['extent']} {r['tags'][:3]}"
    )
unreal.SystemLibrary.quit_editor()
