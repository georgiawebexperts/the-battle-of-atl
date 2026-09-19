"""Import the staged bear candidate and render it, for Elliott to look at.

Editor-only and transient: this imports the mesh and a preview material as saved
assets, but the staged actors are never saved into the map and
ABattleSpirit::bPresentationReady is not touched. The renders are the point -
the design note for the spirit forbids calling a low-poly substitute finished
without a visual review, so this produces the review material and nothing else.

Run headless:
  UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file> \
    -unattended -nosplash -stdout -NoTextureStreaming
"""
import datetime
import hashlib
import json
import struct
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
source = root / "SourceAssets/Spirit/BearCandidate/Bear.fbx"
dest = "/Game/BattleForTheA/Spirit/BearCandidate"
out = root / "work/bear-candidate-review"
out.mkdir(parents=True, exist_ok=True)
assert source.exists(), "staged FBX missing"

report = {"created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
          "source": str(source), "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
          "map_saved": False, "bPresentationReady_touched": False}

# ---------------------------------------------------------------- import the mesh
task = unreal.AssetImportTask()
task.filename = str(source)
task.destination_path = dest
task.destination_name = "BearCandidate"
task.automated = True
task.replace_existing = True
task.save = True
options = unreal.FbxImportUI()
options.set_editor_property("import_mesh", True)
options.set_editor_property("import_materials", True)
options.set_editor_property("import_textures", True)
options.set_editor_property("import_animations", True)
options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
task.options = options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

created = list(unreal.EditorAssetLibrary.list_assets(dest, recursive=False, include_folder=False))
report["imported"] = [str(a).split(".")[-1] for a in created]
sk = unreal.load_asset(dest + "/BearCandidate_Skeleton") or None
skeletal = None
static = None
for a in created:
    asset = unreal.load_asset(a)
    if isinstance(asset, unreal.SkeletalMesh):
        skeletal = asset
    elif isinstance(asset, unreal.StaticMesh):
        static = asset
report["kinds"] = {"skeletal": skeletal.get_path_name() if skeletal else None,
                   "static": static.get_path_name() if static else None}
assert skeletal or static, "the FBX imported neither a skeletal nor a static mesh"

mesh_asset = skeletal or static
origin, extent = mesh_asset.get_bounds().origin, mesh_asset.get_bounds().box_extent
report["bounds_cm"] = {"origin": [origin.x, origin.y, origin.z], "extent": [extent.x, extent.y, extent.z],
                       "longest_axis_cm": 2 * max(extent.x, extent.y, extent.z)}
report["materials"] = [str(m.get_name()) for m in mesh_asset.get_editor_property("materials" if skeletal else "static_materials") or []]
if skeletal:
    report["skeleton"] = str(skeletal.get_editor_property("skeleton").get_name()) if skeletal.get_editor_property("skeleton") else None
    report["bone_count"] = len(skeletal.get_editor_property("skeleton").get_editor_property("bone_tree") or []) if skeletal.get_editor_property("skeleton") else 0
anims = [a for a in created if isinstance(unreal.load_asset(a), unreal.AnimSequence)]
report["animations"] = [str(a).split(".")[-1] for a in anims]

# ------------------------------------------------------------------- the scene
assert unreal.EditorLoadingAndSavingUtils.load_map("/Game/PiedmontRide/Maps/PiedmontWorld")
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
unreal.PiedmontWorldTools.finish_editor_asset_loading()

# Somewhere flat and open: the Fourth Ward pad, west of the skatepark.
spot = unreal.Vector(37000.0, 75000.0, 8000.0)
hit = unreal.PiedmontWorldTools.trace_world_surface(spot, spot - unreal.Vector(0, 0, 6000))
assert hit[0], "no ground under the bear review spot"
ground = hit[1]
report["ground"] = [ground.x, ground.y, ground.z]

if skeletal:
    actor = ea.spawn_actor_from_class(unreal.SkeletalMeshActor, ground)
    comp = actor.skeletal_mesh_component
    comp.set_skeletal_mesh_asset(skeletal)
    comp.set_editor_property("disable_post_process_blueprint", True)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
else:
    actor = ea.spawn_actor_from_class(unreal.StaticMeshActor, ground)
    comp = actor.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_static_mesh(static)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
actor.set_actor_label("Bear candidate review")
actor.set_actor_rotation(unreal.Rotator(yaw=0), False)

# A bear is about 180 cm nose to tail; report what the file actually is rather
# than silently rescaling it.
length = 2 * extent.y
scale = 180.0 / length if length > 1.0 else 1.0
report["authored_length_cm"] = length
report["review_scale"] = scale
actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
height = 2 * extent.z * scale

# ------------------------------------------------------- a spectral preview material
lib = unreal.MaterialEditingLibrary
mat = unreal.load_asset(dest + "/M_BearSpectralPreview")
if not mat:
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_BearSpectralPreview", dest, unreal.Material, unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(mat)
mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property("two_sided", True)
fresnel = lib.create_material_expression(mat, unreal.MaterialExpressionFresnel, -600, 0)
fresnel.set_editor_property("exponent", 3.0)
fresnel.set_editor_property("base_reflect_fraction", 0.15)
rim = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, -160)
rim.set_editor_property("constant", unreal.LinearColor(0.42, 0.62, 0.95, 1.0))
body = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, 120)
body.set_editor_property("constant", unreal.LinearColor(0.02, 0.02, 0.03, 1.0))
mul = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -200, -160)
opacity = lib.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -200, 200)
opacity.set_editor_property("const_a", 0.22)
opacity.set_editor_property("const_b", 0.92)
lib.connect_material_expressions(mul, "", opacity, "alpha")
lib.connect_material_expressions(fresnel, "", mul, "a")
lib.connect_material_expressions(rim, "", mul, "b")
lib.connect_material_expressions(body, "", opacity, "a")
lib.connect_material_expressions(rim, "", opacity, "b")
lib.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.connect_material_property(opacity, "", unreal.MaterialProperty.MP_BASE_COLOR)
lib.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY)
lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat, False)
report["spectral_material"] = mat.get_path_name()

# ------------------------------------------------------------------- capture
for light in ea.get_all_level_actors():
    if isinstance(light, unreal.DirectionalLight):
        light.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(40)

cam = ea.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector())
capture = cam.get_component_by_class(unreal.SceneCaptureComponent2D)
target = unreal.RenderingLibrary.create_render_target2d(world, 1280, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8)
capture.set_editor_property("texture_target", target)
capture.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
capture.set_editor_property("always_persist_rendering_state", True)
capture.set_editor_property("capture_every_frame", False)
capture.set_editor_property("capture_on_movement", False)
capture.set_editor_property("fov_angle", 45)

centre = ground + unreal.Vector(0, 0, height * 0.5)
distance = max(420.0, height * 3.0)


def render(name, offset, look_from):
    location = look_from + offset
    cam.set_actor_location(location, False, False)
    cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(location, look_from), False)
    for _ in range(14):
        unreal.PiedmontWorldTools.tick_scene_review()
        capture.capture_scene()
    path = out / (name + ".png")
    unreal.RenderingLibrary.export_render_target(world, target, str(out), name + ".png")
    data = path.read_bytes()
    assert data[:8] == b"\x89PNG\r\n\x1a\n", name
    size = struct.unpack(">II", data[16:24])
    assert size == (1280, 720), (name, size)
    return {"file": name + ".png", "sha256": hashlib.sha256(data).hexdigest(),
            "camera": [location.x, location.y, location.z]}

views = []
views.append(render("authored-three-quarter", unreal.Vector(-distance, -distance, distance * 0.45), centre))
views.append(render("authored-side", unreal.Vector(distance * 1.2, 0, distance * 0.25), centre))

for i in range(comp.get_num_materials() if not skeletal else len(skeletal.get_editor_property("materials"))):
    comp.set_material(i, mat)
views.append(render("spectral-three-quarter", unreal.Vector(-distance, -distance, distance * 0.45), centre))
views.append(render("spectral-front", unreal.Vector(0, -distance * 1.3, height * 0.7), centre))
report["images"] = views

report["scope"] = ("Shape and spectral-read review only. The staged actors are transient, the map is not "
                   "saved, no gameplay, encounter rule or presentation flag was changed, and the CC-BY 4.0 "
                   "attribution is not yet in any credits file.")
(out / "manifest.json").write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps({k: report[k] for k in ("kinds", "bounds_cm", "authored_length_cm", "animations", "bone_count")}))
