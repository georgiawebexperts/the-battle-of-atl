"""List candidate prop actors in PiedmontWorld so a sighting can be named.

Run with:
  UnrealEditor-Cmd <uproject> -run=pythonscript -script=Scripts/inspect_park_props.py \
    -AllowCommandletRendering -NoTextureStreaming -RCWebControlDisable -unattended -nosound -stdout

Reports every actor whose label, class or static-mesh name contains one of the
KEYWORDS, and everything within RADIUS of each anchor whose label matches one of
the ANCHORS. Written for the repeated "logs" sighting near the gate and the lake.
"""
import json
from pathlib import Path
import unreal

KEYWORDS = ["log", "stump", "cylinder", "trash", "bollard", "barrel", "amber",
            "marker", "debris", "wreck", "can", "post", "pole"]
ANCHORS = ["lake", "gate", "piedmont", "boathouse", "pavilion"]
RADIUS = 6000.0

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "work" / "park-props.json"


def mesh_names(actor):
    names = []
    for component in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = component.get_editor_property("static_mesh")
        if mesh:
            names.append(mesh.get_name())
    return names


def describe(actor):
    try:
        location = actor.get_actor_location()
    except Exception:
        location = unreal.Vector(0, 0, 0)
    return {
        "label": actor.get_actor_label(),
        "class": actor.get_class().get_name(),
        "meshes": mesh_names(actor),
        "location": [round(location.x, 1), round(location.y, 1), round(location.z, 1)],
        "hidden": bool(actor.get_editor_property("bHidden")) if hasattr(actor, "get_editor_property") else None,
    }


def main():
    # A commandlet starts on an empty untitled world; the map has to be loaded.
    unreal.EditorLevelLibrary.load_level("/Game/PiedmontRide/Maps/PiedmontWorld")
    world = unreal.EditorLevelLibrary.get_editor_world()
    actors = list(unreal.EditorLevelLibrary.get_all_level_actors())
    anchors = [a for a in actors if any(k in a.get_actor_label().lower() for k in ANCHORS)]
    keyword_hits, near_anchor = [], []
    seen = set()
    for actor in actors:
        if actor in seen:
            continue
        seen.add(actor)
        text = " ".join([actor.get_actor_label(), actor.get_class().get_name()] + mesh_names(actor)).lower()
        if any(k in text for k in KEYWORDS):
            keyword_hits.append(describe(actor))
        for anchor in anchors:
            if (actor.get_actor_location() - anchor.get_actor_location()).length() <= RADIUS:
                entry = describe(actor)
                entry["near"] = anchor.get_actor_label()
                near_anchor.append(entry)
                break
    report = {
        "world": world.get_name(),
        "actor_count": len(actors),
        "anchors": [describe(a) for a in anchors],
        "keyword_hits": keyword_hits,
        "near_anchors": near_anchor,
        "all_actors": [describe(a) for a in actors],
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=1))
    unreal.log("ParkPropInspect: wrote {}".format(OUT))
    unreal.log("ParkPropInspect: {} actors, {} keyword hits, {} near anchors".format(
        len(actors), len(keyword_hits), len(near_anchor)))


main()
