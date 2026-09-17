import unreal, pathlib, json
root=pathlib.Path(unreal.Paths.project_dir())
task=unreal.AssetImportTask()
task.filename=str(root/'SourceAssets/Spirit/SpectralBlackBear.png')
task.destination_path='/Game/BattleForTheA/Spirit'
task.destination_name='T_SpectralBlackBear'
task.automated=True
task.replace_existing=True
task.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
tex=unreal.load_asset('/Game/BattleForTheA/Spirit/T_SpectralBlackBear')
assert tex, 'Spirit texture import failed'
tex.set_editor_property('srgb',True)
tex.set_editor_property('virtual_texture_streaming',False)
tex.set_editor_property('never_stream',True)
unreal.EditorAssetLibrary.save_loaded_asset(tex,False)
(root/'Scripts/spirit-bear-import.json').write_text(json.dumps({'asset':'/Game/BattleForTheA/Spirit/T_SpectralBlackBear','source':str(root/'SourceAssets/Spirit/SpectralBlackBear.png'),'saved':True},indent=2))
