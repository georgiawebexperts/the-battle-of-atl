import unreal
unreal.EditorAssetLibrary.save_directory('/Game/PiedmontRide',only_if_is_dirty=False,recursive=True)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
