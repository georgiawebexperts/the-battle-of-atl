"""What is above the Krog bore past each portal?

Elliott: "are you also working to extend the tunnel it is way too short". The
source rasters cannot answer this - the rail yard over the bore is structure,
not terrain, so a bare-earth sample reads below the roof even today. The only
authoritative answer is the world itself.

This walks outward from each end of the installed tunnel road, along the bore
axis, and at each station records what a trace upward hits (label and height),
what a trace downward hits, and whether a 62 cm rider sphere fits. The first
station that is open sky is the furthest a longer bore could reach before it
would have to cover live street.
"""
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
STEP_CM = 250.0
REACH_CM = 5000.0

assert unreal.EditorLoadingAndSavingUtils.load_map("/Game/PiedmontRide/Maps/PiedmontWorld")
unreal.PiedmontWorldTools.finish_editor_asset_loading()


def labelled(kind):
    found = [
        a
        for a in ea.get_all_level_actors()
        if a.get_actor_label() == "Krog route SM_KrogTunnel_" + kind
    ]
    assert len(found) == 1, f"expected one {kind} actor, found {len(found)}"
    return found[0]


# Use the game's own definition of the bore, the two constants the hazards,
# the crash scene and the finish check all key off (BattleHomeData.h:168).
entry = unreal.Vector(29782.14816, 115813.62941, 956.82463)
exit_ = unreal.Vector(31590.10655, 120052.33872, 963.51887)
dx, dy, dz = exit_.x - entry.x, exit_.y - entry.y, exit_.z - entry.z
bore = (dx * dx + dy * dy + dz * dz) ** 0.5
ux, uy, uz = dx / bore, dy / bore, dz / bore
print(f"bore from the game constants: {bore:.0f} cm")

stations = []
for label, end, sign in (("entry/Dekalb", entry, -1.0), ("exit/trail", exit_, 1.0)):
    for step in range(0, int(REACH_CM) + 1, int(STEP_CM)):
        travel = step * sign
        floor = end.z + uz * travel
        p = unreal.Vector(end.x + ux * travel, end.y + uy * travel, floor)
        up = unreal.PiedmontWorldTools.trace_world_surface(
            unreal.Vector(p.x, p.y, floor + 60), unreal.Vector(p.x, p.y, floor + 2400)
        )
        down = unreal.PiedmontWorldTools.trace_world_surface(
            unreal.Vector(p.x, p.y, floor + 400), unreal.Vector(p.x, p.y, floor - 400)
        )
        rider = unreal.PiedmontWorldTools.trace_world_surface(
            unreal.Vector(p.x, p.y, floor + 95),
            unreal.Vector(p.x, p.y, floor + 95),
            62,
        )
        stations.append(
            {
                "end": label,
                "beyond_portal_cm": step,
                "xyz": [round(p.x, 1), round(p.y, 1), round(p.z, 1)],
                "above": None
                if not up
                else {
                    "actor": up[1].get_actor_label(),
                    "z": round(up[0].z, 1),
                    "height_above_floor_cm": round(up[0].z - floor, 1),
                },
                "below": None
                if not down
                else {"actor": down[1].get_actor_label(), "z": round(down[0].z, 1)},
                "rider_blocked_by": None if not rider else rider[1].get_actor_label(),
            }
        )

out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "question": "How far past each portal does the bore stay covered?",
    "scope": "Traces only, on PiedmontWorld as saved. step 250 cm, reach 5000 cm.",
    "stations": stations,
}
(root / "Tests/Results/2026-09-19-krog-tunnel-extension-world-probe.json").write_text(
    json.dumps(out, indent=2) + "\n"
)
for row in stations:
    above = row["above"]
    print(
        f"{row['end']:>5} +{row['beyond_portal_cm']:>4.0f}cm  "
        f"above={'sky' if above is None else above['actor'] + '@' + str(above['height_above_floor_cm'])}  "
        f"rider={'clear' if row['rider_blocked_by'] is None else row['rider_blocked_by']}"
    )
