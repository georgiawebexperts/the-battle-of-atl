import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
task=unreal.AssetImportTask();task.filename=str(root/'SourceAssets/Story/98-estoria-celebration.png');task.destination_path='/Game/BattleForTheA/Story';task.destination_name='T_EstoriaCelebration';task.automated=True;task.replace_existing=True;task.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
tex=unreal.load_asset('/Game/BattleForTheA/Story/T_EstoriaCelebration');assert tex
tex.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI);tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON);tex.set_editor_property('never_stream',True)
assert unreal.EditorAssetLibrary.save_loaded_asset(tex)
(root/'work/story-art-import.json').write_text(json.dumps({'asset':tex.get_path_name(),'imported':True})+'\n')
unreal.SystemLibrary.quit_editor()
