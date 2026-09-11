import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir())
path='/Game/BattleForTheA/Data/DT_Difficulty'
struct=unreal.load_object(None,'/Script/AuraPlayground.BattleDifficultyRow')
assert struct, 'Missing compiled difficulty row struct'
table=unreal.load_asset(path)
if not table:
    factory=unreal.DataTableFactory()
    factory.set_editor_property('struct',struct)
    table=unreal.AssetToolsHelpers.get_asset_tools().create_asset('DT_Difficulty','/Game/BattleForTheA/Data',unreal.DataTable,factory)
assert table
assert unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(table,str(root/'SourceAssets/Data/Difficulty.csv'),struct)
assert unreal.EditorAssetLibrary.save_loaded_asset(table,False)
names=[str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(table)]
assert sorted(names)==['Easy','Hard','Medium'],names
(root/'Scripts/difficulty-installed.json').write_text(json.dumps({'rows':names,'saved':True},indent=2))
