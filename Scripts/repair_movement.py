import unreal,pathlib,json,traceback
bp=unreal.EditorAssetLibrary.load_asset('/Game/BeltLineGlide/BP_BeltLineBike')
cls=unreal.EditorAssetLibrary.load_blueprint_class('/Game/BeltLineGlide/BP_BeltLineBike')
cdo=unreal.get_default_object(cls)
r=[]
bp.modify();cdo.modify()
for key,val in [('CharMoveComp',cdo.get_editor_property('character_movement')),('StartLocation',unreal.Vector(-2484.837927,6543.335979,90)),('StartRotation',unreal.Rotator(pitch=0,yaw=0,roll=0))]:
    try:cdo.set_editor_property(key,val);r.append({key:str(cdo.get_editor_property(key))})
    except Exception:r.append({key:traceback.format_exc()})
unreal.EditorAssetLibrary.save_loaded_asset(bp)
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if a.get_actor_label()=='Glide_SkyLight':a.get_component_by_class(unreal.SkyLightComponent).set_mobility(unreal.ComponentMobility.MOVABLE)
    if a.get_actor_label()=='BeltLineBike_Start_LakeLoop':
        r.append({'placed_movement_ref':str(a.get_editor_property('CharMoveComp'))})
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
pathlib.Path(unreal.Paths.project_dir(),'Scripts','movement-result.json').write_text(json.dumps(r,indent=2))
