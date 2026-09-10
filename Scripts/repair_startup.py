import unreal, json, pathlib, traceback

root = pathlib.Path(unreal.Paths.project_dir())
report = []
def step(name, fn):
    try:
        value = fn()
        report.append({'step': name, 'result': str(value)})
        return value
    except Exception:
        report.append({'step': name, 'error': traceback.format_exc()})

level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
step('load map', lambda: level.load_level('/Game/BeltLineGlide/BeltLineGlide'))
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
gm = unreal.EditorAssetLibrary.load_blueprint_class('/Game/BeltLineGlide/BP_BeltLineGameMode')
step('world game mode', lambda: world.get_world_settings().set_editor_property('default_game_mode', gm))
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
for label, cls, loc, rot in [
    ('Glide_Sun', unreal.DirectionalLight, unreal.Vector(0,0,20000), unreal.Rotator(-45,-35,0)),
    ('Glide_SkyLight', unreal.SkyLight, unreal.Vector(0,0,10000), unreal.Rotator()),
    ('Glide_Atmosphere', unreal.SkyAtmosphere, unreal.Vector(), unreal.Rotator())]:
    a = existing.get(label) or actors.spawn_actor_from_class(cls, loc, rot)
    a.set_actor_label(label)
    if cls == unreal.DirectionalLight:
        step('sun intensity', lambda: a.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(5.0))
    if cls == unreal.SkyLight:
        step('sky intensity', lambda: a.get_component_by_class(unreal.SkyLightComponent).set_intensity(1.0))
        step('sky capture', lambda: a.get_component_by_class(unreal.SkyLightComponent).recapture_sky())
bike = existing.get('BeltLineBike_Start_LakeLoop')
if bike:
    step('possess bike', lambda: bike.set_editor_property('auto_possess_player', unreal.AutoReceiveInput.PLAYER0))
    loc = bike.get_actor_location()
    start = existing.get('PlayerStart_LakeLoop') or actors.spawn_actor_from_class(unreal.PlayerStart, loc + unreal.Vector(0,0,150), bike.get_actor_rotation())
    start.set_actor_label('PlayerStart_LakeLoop')
    report.append({'bike_location': str(loc), 'components': [str(c.get_class().get_name()) for c in bike.get_components_by_class(unreal.ActorComponent)]})
step('save level', level.save_current_level)
step('save assets', lambda: unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True))
(root/'Scripts'/'startup-repair-result.json').write_text(json.dumps(report, indent=2))
print('GLIDE_REPAIR_DONE ' + json.dumps(report))
