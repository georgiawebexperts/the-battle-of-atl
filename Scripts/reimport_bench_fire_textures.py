"""Reimport exported Epic source pixels as standalone project textures."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Effects/BenchFire';rows=[]
for kind in ['Fire','Smoke']:
 name='T_'+kind+'_SubUV';old=unreal.load_asset(dest+'/'+name)
 before={'srgb':old.get_editor_property('srgb'),'compression':str(old.get_editor_property('compression_settings')),'address_x':str(old.get_editor_property('address_x')),'address_y':str(old.get_editor_property('address_y'))}
 task=unreal.AssetImportTask();task.filename=str(root/'SourceAssets/Effects/BenchFire'/f'{name}.tga');task.destination_path=dest;task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);tex=unreal.load_asset(dest+'/'+name);assert tex
 tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_DEFAULT);tex.set_editor_property('srgb',True);tex.set_editor_property('never_stream',True)
 unreal.EditorAssetLibrary.save_loaded_asset(tex)
 rows.append({'name':name,'before':before,'standalone_reimport':True})
(root/'work/bench-fire-texture-reimport.json').write_text(json.dumps(rows,indent=2))
