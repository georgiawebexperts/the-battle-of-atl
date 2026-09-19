"""Install the cut-and-cover gallery south of the Krog bore, with its lighting.

Imports the three baked chunks, places them on the same source-ENU transform the
rest of the Krog route uses, and furnishes the new stretch the way the bore is
furnished: a lens fixture and a downlight every ~720 cm (copied from one of the
bore's own downlights rather than re-typed), and dark-zone volumes along the
line. The riding surface is not touched - the road under the gallery is the
approach road that was already there.

This script saves the map.
"""
import json
import math
import pathlib
import sys

import unreal

root = pathlib.Path(unreal.Paths.project_dir())
sys.path.insert(0, str(root / "Scripts"))
from battle_geography import place_source_geometry, require_converted_world  # noqa: E402

assert unreal.EditorLoadingAndSavingUtils.load_map("/Game/PiedmontRide/Maps/PiedmontWorld")
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
require_converted_world(world)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

base = root / "SourceAssets/Terrain/KrogGallery"
manifest = json.loads((base / "manifest.json").read_text())
stations_file = json.loads((root / "work/krog-gallery-stations.json").read_text())

# Station rows carry the line: centreline in source ENU, the normal, the deck.
rows_by_station = {round(row["station_cm"], 1): row for row in stations_file["stations"]}
for row in manifest["stations_cm"]:
    key = round(row, 1)
    if key in rows_by_station:
        continue
    nearest = min(rows_by_station, key=lambda k: abs(k - key))
    rows_by_station[key] = rows_by_station[nearest]
stations = [round(s, 1) for s in manifest["stations_cm"]]


def on_line(station, lateral, z_offset):
    """A world point on the gallery line, in ESU cm."""
    row = rows_by_station[min(rows_by_station, key=lambda k: abs(k - station))]
    cx, cy = row["centreline_cm"]
    nx, ny = row["normal_xy"]
    return unreal.Vector(
        cx + nx * lateral, -(cy + ny * lateral), row["deck_z_cm"] + z_offset
    ), row


dest = "/Game/BattleForTheA/Environment/BeltLine/Paths"
existing = {a.get_actor_label(): a for a in ea.get_all_level_actors()}
chunks = []
for chunk in manifest["chunks"]:
    name = "SM_" + pathlib.Path(chunk["file"]).stem
    options = unreal.FbxImportUI()
    options.import_as_skeletal = False
    options.import_materials = False
    options.import_textures = False
    options.automated_import_should_detect_type = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.auto_generate_collision = False
    options.static_mesh_import_data.remove_degenerates = False
    task = unreal.AssetImportTask()
    task.filename = str(base / chunk["file"])
    task.destination_path = dest
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(dest + "/" + name)
    assert mesh, name
    box = mesh.get_bounding_box()
    bounds = [[box.min.x, box.min.y, box.min.z], [box.max.x, box.max.y, box.max.z]]
    assert max(abs(bounds[i][j] - chunk["bounds_cm"][i][j]) for i in range(2) for j in range(3)) < 0.1, (
        name,
        bounds,
    )
    mesh.get_editor_property("body_setup").set_editor_property(
        "collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE
    )
    nanite = mesh.get_editor_property("nanite_settings")
    nanite.enabled = True
    nanite.position_precision = 8
    nanite.generate_fallback = unreal.NaniteGenerateFallback.ENABLED
    nanite.fallback_target = unreal.NaniteFallbackTarget.PERCENT_TRIANGLES
    nanite.fallback_relative_error = 0
    nanite.fallback_percent_triangles = 1
    mesh.set_editor_property("nanite_settings", nanite)
    mesh.set_material(0, unreal.load_asset("/Game/PiedmontRide/Materials/M_" + chunk["material"]))
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)

    label = "Krog route " + name
    actor = existing.get(label) or ea.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
    actor.set_actor_label(label)
    actor.set_folder_path("BattleForTheA/BeltLine")
    actor.static_mesh_component.set_static_mesh(mesh)
    place_source_geometry(actor)
    actor.static_mesh_component.set_collision_profile_name("BlockAll")
    actor.tags = [unreal.Name("KrogGallery")] + (
        [unreal.Name("RideGrass")] if chunk["structure"] == "Cover" else [unreal.Name("RideBarrier")]
    )
    chunks.append({"mesh": name, "actors": 1, "triangles": chunk["triangles"]})
unreal.PiedmontWorldTools.finish_editor_asset_loading()

# --- Furniture, copied from the bore so the two stretches match.
source_light = None
source_lens = None
for actor in ea.get_all_level_actors():
    if isinstance(actor, unreal.SpotLight) and actor.actor_has_tag(unreal.Name("KrogDownlight")):
        source_light = source_light or actor
    if actor.get_actor_label().startswith("Krog utility lens "):
        source_lens = source_lens or actor
assert source_light, "no bore downlight found to copy"
assert source_lens, "no bore lens found to copy"
light_properties = source_light.get_component_by_class(unreal.SpotLightComponent)
lens_mesh = source_lens.static_mesh_component.static_mesh
lens_material = source_lens.static_mesh_component.get_material(0)
lens_scale = source_lens.get_actor_scale3d()

for actor in list(ea.get_all_level_actors()):
    if actor.actor_has_tag(unreal.Name("KrogGalleryLight")) or actor.actor_has_tag(
        unreal.Name("KrogGalleryDarkness")
    ):
        ea.destroy_actor(actor)

lights = []
for index in range(1, len(stations) - 1, 6):  # one fixture every six stations
    station = stations[index]
    location, _ = on_line(station, 0.0, 250)
    spot = ea.spawn_actor_from_class(unreal.SpotLight, location)
    spot.set_actor_label(f"Krog gallery light {index}")
    spot.set_folder_path("BattleForTheA/BeltLine")
    spot.tags = [unreal.Name("KrogDownlight"), unreal.Name("KrogGalleryLight")]
    component = spot.get_component_by_class(unreal.SpotLightComponent)
    for prop in (
        "mobility",
        "intensity_units",
        "intensity",
        "attenuation_radius",
        "use_temperature",
        "temperature",
        "cast_shadows",
        "inner_cone_angle",
        "outer_cone_angle",
    ):
        component.set_editor_property(prop, light_properties.get_editor_property(prop))
    spot.set_actor_rotation(unreal.Rotator(pitch=-90, yaw=0, roll=0), False)
    lens_location, _ = on_line(station, 0.0, 268)
    lens = ea.spawn_actor_from_class(unreal.StaticMeshActor, lens_location)
    lens.set_actor_label(f"Krog gallery lens {index}")
    lens.set_folder_path("BattleForTheA/BeltLine")
    lens.tags = [unreal.Name("KrogGalleryLight")]
    lens.set_actor_scale3d(lens_scale)
    lens.static_mesh_component.set_static_mesh(lens_mesh)
    lens.static_mesh_component.set_material(0, lens_material)
    lens.static_mesh_component.set_collision_profile_name("NoCollision")
    lights.append(station)

zones = []
bore_zone = None
for actor in ea.get_all_level_actors():
    if isinstance(actor, unreal.PiedmontDarkZone) and actor.get_actor_label().startswith("Krog darkness "):
        bore_zone = actor
        break
if bore_zone:
    extent = bore_zone.get_component_by_class(unreal.BoxComponent).get_unscaled_box_extent()
    for index in range(0, len(stations) - 2, 2):
        station = stations[index]
        location, _ = on_line(station, 0.0, 130)
        ahead, _ = on_line(stations[index + 2], 0.0, 130)
        zone = ea.spawn_actor_from_class(unreal.PiedmontDarkZone, location)
        zone.set_actor_label(f"Krog gallery darkness {index}")
        zone.set_folder_path("BattleForTheA/BeltLine")
        zone.tags = [unreal.Name("KrogGalleryDarkness")]
        zone.set_actor_rotation(
            unreal.Rotator(
                pitch=0,
                yaw=math.degrees(math.atan2(ahead.y - location.y, ahead.x - location.x)),
                roll=0,
            ),
            False,
        )
        zone.get_component_by_class(unreal.BoxComponent).set_box_extent(extent)
        zones.append(zone.get_actor_label())
    unreal.PiedmontWorldTools.finish_editor_asset_loading()

# The audit follows the gallery by its own spine rather than by guessing the
# road's centre inside a diagonal bounding box, so the line the bake was built
# from is placed in the world with it.
spine_label = "Krog gallery spine"
spine = existing.get(spine_label)
if not spine:
    spine = ea.spawn_actor_from_class(unreal.PiedmontPathSpline, unreal.Vector())
spine.set_actor_label(spine_label)
spine.set_folder_path("BattleForTheA/BeltLine")
spine.tags = [unreal.Name("KrogGallerySpine"), unreal.Name("KrogGallery")]
spine_points = []
for station in stations:
    row = rows_by_station[min(rows_by_station, key=lambda k: abs(k - station))]
    cx, cy = row["centreline_cm"]
    spine_points.append(unreal.Vector(cx, -cy, row["deck_z_cm"]))
spine.set_centerline(spine_points)
unreal.PiedmontWorldTools.finish_editor_asset_loading()

assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level(), "map save failed"
out = {
    "author": "2026-09-19 [codex-maclaptop]",
    "extension_cm": manifest["extension_cm"],
    "stations_cm": stations,
    "chunks": chunks,
    "lights": lights,
    "dark_zones": len(zones),
    "spine_actor": spine_label,
    "spine_points": len(spine_points),
    "map_saved": True,
    "light_source": source_light.get_actor_label(),
    "lens_source": source_lens.get_actor_label(),
}
(root / "work/krog-gallery-install.json").write_text(json.dumps(out, indent=2) + "\n")
print(json.dumps({"chunks": chunks, "lights": len(lights), "dark_zones": len(zones)}))
