import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir());tools=unreal.AssetToolsHelpers.get_asset_tools()
for name in ['S_Shotgun','S_SMG']:
 t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Audio'/f'{name}.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name=name;t.automated=True;t.save=True;t.replace_existing=True;tools.import_asset_tasks([t])
