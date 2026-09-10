import unreal,json,pathlib,traceback
r={}
bp=unreal.EditorAssetLibrary.load_asset('/Game/BeltLineGlide/BP_BeltLineBike')
try:
    unreal.BlueprintEditorLibrary.set_blueprint_variable_instance_editable(bp,'CharMoveComp',True)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        if a.get_actor_label()=='BeltLineBike_Start_LakeLoop':
            a.set_editor_property('CharMoveComp',a.get_editor_property('character_movement'))
            r['reference']=str(a.get_editor_property('CharMoveComp'))
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
except Exception:r['error']=traceback.format_exc();r['methods']=[n for n in dir(unreal.BlueprintEditorLibrary) if 'variable' in n]
pathlib.Path(unreal.Paths.project_dir(),'Scripts','expose-result.json').write_text(json.dumps(r,indent=2))
