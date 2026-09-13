"""Import the original AI-generated APD command for in-world playback."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
task=unreal.AssetImportTask()
task.filename=str(root/'SourceAssets/Audio/S_APDStop.wav')
task.destination_path='/Game/BattleForTheA/Audio'
task.destination_name='S_APDStop'
task.automated=True
task.replace_existing=True
task.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
sound=unreal.load_asset('/Game/BattleForTheA/Audio/S_APDStop')
assert sound and 1 < sound.get_editor_property('duration') < 2
(root/'work/police-voice-import.json').write_text(json.dumps({'asset':sound.get_path_name(),'duration':sound.get_editor_property('duration'),'passed':True},indent=2))
