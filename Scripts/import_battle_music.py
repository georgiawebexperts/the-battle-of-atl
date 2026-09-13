"""Import Elliott's supplied game theme as a looping music asset."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Audio/S_BattleATLTheme.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name='S_BattleATLTheme';t.automated=True;t.replace_existing=True;t.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
s=unreal.load_asset('/Game/BattleForTheA/Audio/S_BattleATLTheme');assert s
s.set_editor_property('looping',True);s.set_editor_property('sound_group',unreal.SoundGroup.SOUNDGROUP_MUSIC)
unreal.EditorAssetLibrary.save_loaded_asset(s)
duration=s.get_editor_property('duration');assert 180<duration<195
(root/'work/battle-music-import.json').write_text(json.dumps({'asset':s.get_path_name(),'duration':duration,'looping':True,'passed':True},indent=2))
