import unreal,json,pathlib,traceback
p=pathlib.Path(unreal.Paths.project_dir());meta=json.loads((p/'SourceAssets/Terrain/terrain-georeference.json').read_text())
try:
    editor=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor.new_level('/Game/PiedmontRide/Maps/PiedmontWorld')
    import sys;sys.path.insert(0,str(p/'Scripts'));from battle_geography import import_source_landscape
    land=import_source_landscape(p/'SourceAssets/Terrain/atlanta-height.r16',meta)
    if not land:raise RuntimeError('Landscape import returned no actor')
    land.set_editor_property('landscape_material',unreal.load_asset('/Game/PiedmontRide/Materials/M_Grass'))
    ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    sun=ea.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,8000),unreal.Rotator(pitch=-28,yaw=40,roll=0));sun.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);sun.light_component.set_editor_property('intensity',4)
    ea.spawn_actor_from_class(unreal.SkyAtmosphere,unreal.Vector())
    sky=ea.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,10000));sky.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);sky.light_component.set_editor_property('real_time_capture',True)
    world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();world.get_world_settings().set_editor_property('default_game_mode',unreal.load_class(None,'/Script/AuraPlayground.PiedmontRideMode'))
    world.get_world_settings().tags=list(world.get_world_settings().tags)+[unreal.Name('BattleGeography_ESU_v1')]
    editor.save_current_level()
    (p/'Scripts/measured-landscape-built.json').write_text(json.dumps({'map':world.get_name(),'landscape':str(land),'component_count':len(land.get_components_by_class(unreal.LandscapeComponent)),'georeference':meta},indent=2))
except Exception:
    (p/'Scripts/measured-landscape-error.txt').write_text(traceback.format_exc());raise
