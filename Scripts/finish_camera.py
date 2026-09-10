import unreal, pathlib, json, traceback
root=pathlib.Path(unreal.Paths.project_dir())
actor_sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors={a.get_actor_label():a for a in actor_sub.get_all_level_actors()}
bike=actors['BeltLineBike_Start_LakeLoop']
report=[]
def attempt(name,fn):
    try: report.append({name:str(fn())})
    except Exception: report.append({name:traceback.format_exc()})
cam=actors.get('Glide_ChaseCamera') or actor_sub.spawn_actor_from_class(unreal.CameraActor,bike.get_actor_location()+unreal.Vector(-550,0,280),unreal.Rotator(-15,0,0))
cam.set_actor_label('Glide_ChaseCamera')
attempt('camera activation',lambda:cam.set_editor_property('auto_activate_for_player',unreal.AutoReceiveInput.PLAYER0))
attempt('camera attach',lambda:cam.attach_to_actor(bike,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False))
for name in ['BP_BeltLineBike','BP_Pedestrian','BP_ScooterRider','BP_BeltLineHUD','BP_BeltLineGameMode']:
    bp=unreal.EditorAssetLibrary.load_asset('/Game/BeltLineGlide/'+name)
    attempt('compile '+name,lambda bp=bp:unreal.BlueprintEditorLibrary.compile_blueprint(bp))
    attempt('save '+name,lambda bp=bp:unreal.EditorAssetLibrary.save_loaded_asset(bp))
attempt('save level',lambda:unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level())
(root/'Scripts'/'camera-result.json').write_text(json.dumps(report,indent=2))
print('GLIDE_CAMERA_DONE '+json.dumps(report))
