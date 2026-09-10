"""One-time migration from Aura's blockout pawn to the compiled prototype controller."""
import unreal,json,pathlib,time,traceback
_glide_root=pathlib.Path(unreal.Paths.project_dir())
_glide_done=_glide_root/'Scripts'/'native-setup-done.json'
_glide_started=time.monotonic()
def _glide_setup(delta):
    global _glide_setup_handle
    if time.monotonic()-_glide_started<3:return
    try:
        world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        if not world or 'BeltLineGlide' not in world.get_name():return
        gm=unreal.load_class(None,'/Script/AuraPlayground.GlideGameMode')
        if not gm:return
        world.get_world_settings().set_editor_property('default_game_mode',gm)
        hidden=[]
        for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
            label=a.get_actor_label()
            if label=='BeltLineBike_Start_LakeLoop':
                a.set_editor_property('auto_possess_player',unreal.AutoReceiveInput.DISABLED)
                a.set_actor_hidden_in_game(True);a.set_actor_enable_collision(False);a.set_actor_tick_enabled(False);hidden.append(label)
            elif label.startswith('Glide_Bike'):
                a.set_actor_hidden_in_game(True);hidden.append(label)
            elif label=='Glide_ChaseCamera':a.set_editor_property('auto_activate_for_player',unreal.AutoReceiveInput.DISABLED)
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
        _glide_done.write_text(json.dumps({'game_mode':str(gm),'world':world.get_name(),'old_pawn_hidden':hidden},indent=2))
        unreal.unregister_slate_post_tick_callback(_glide_setup_handle)
    except Exception:
        (_glide_root/'Scripts'/'native-setup-error.txt').write_text(traceback.format_exc())
        unreal.unregister_slate_post_tick_callback(_glide_setup_handle)
if not _glide_done.exists():_glide_setup_handle=unreal.register_slate_post_tick_callback(_glide_setup)

# Explicit one-shot development request. No automatic rebuilding on normal launches.
_v2_request=_glide_root/'Scripts'/'request-v2-lab'
if _v2_request.exists():
    def _v2_build(delta):
        global _v2_handle
        if time.monotonic()-_glide_started<7:return
        unreal.unregister_slate_post_tick_callback(_v2_handle)
        _v2_request.unlink()
        exec((_glide_root/'Scripts'/'build_v2_lab.py').read_text(),{'__name__':'__main__'})
    _v2_handle=unreal.register_slate_post_tick_callback(_v2_build)
_v2_test_request=_glide_root/'Scripts'/'request-v2-validation'
if _v2_test_request.exists():
    def _v2_validate(delta):
        global _v2_test_handle
        if time.monotonic()-_glide_started<10:return
        unreal.unregister_slate_post_tick_callback(_v2_test_handle)
        _v2_test_request.unlink()
        exec((_glide_root/'Scripts'/'upgrade_v2_lab.py').read_text(),{'__name__':'__main__'})
        exec((_glide_root/'Scripts'/'validate_v2_bike.py').read_text(),{'__name__':'__main__'})
    _v2_test_handle=unreal.register_slate_post_tick_callback(_v2_validate)
_terrain_request=_glide_root/'Scripts'/'request-measured-landscape'
if _terrain_request.exists():
    def _terrain_build(delta):
        global _terrain_handle
        if time.monotonic()-_glide_started<10:return
        unreal.unregister_slate_post_tick_callback(_terrain_handle)
        _terrain_request.unlink()
        exec((_glide_root/'Scripts'/'build_measured_landscape.py').read_text(),{'__name__':'__main__'})
    _terrain_handle=unreal.register_slate_post_tick_callback(_terrain_build)
