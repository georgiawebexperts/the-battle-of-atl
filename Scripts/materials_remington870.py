"""Use conventional opaque PBR materials for the shotgun candidate on Mac."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Weapons/Remington870';asset=json.loads((root/'work/remington870-import.json').read_text())[0]['path'];mesh=unreal.load_asset(asset)
slots=[{'name':str(s.material_slot_name),'material':s.material_interface.get_path_name() if s.material_interface else None} for s in mesh.static_materials];print('SHOTGUN_SLOTS '+json.dumps(slots))
lib=unreal.MaterialEditingLibrary;tools=unreal.AssetToolsHelpers.get_asset_tools()
for i,s in enumerate(mesh.static_materials):
 name=str(s.material_slot_name);plastic='plastic' in name.lower();matname='M_Remington_'+('Polymer' if plastic else 'Steel');mat=unreal.load_asset(dest+'/'+matname)
 if not mat:mat=tools.create_asset(matname,dest,unreal.Material,unreal.MaterialFactoryNew())
 lib.delete_all_material_expressions(mat)
 def scalar(value,prop):
  n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);n.r=value;lib.connect_material_property(n,'',prop)
 if plastic:
  n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);n.constant=unreal.LinearColor(.022,.026,.03);lib.connect_material_property(n,'',unreal.MaterialProperty.MP_BASE_COLOR);scalar(.6,unreal.MaterialProperty.MP_ROUGHNESS);scalar(0,unreal.MaterialProperty.MP_METALLIC)
 else:
  tex=unreal.load_asset(dest+'/Remington870/Textures/gunmetal_silver_baseColor');assert tex
  n=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample);n.texture=tex;lib.connect_material_property(n,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
  packed=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample);packed.texture=unreal.load_asset(dest+'/Remington870/Textures/gunmetal_silver_metallicRoughness');packed.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR;lib.connect_material_property(packed,'G',unreal.MaterialProperty.MP_ROUGHNESS);lib.connect_material_property(packed,'B',unreal.MaterialProperty.MP_METALLIC)
 lib.recompile_material(mat);mesh.set_material(i,mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.PiedmontWorldTools.finish_editor_asset_loading();unreal.EditorAssetLibrary.save_loaded_asset(mesh)
(root/'work/remington870-materials.json').write_text(json.dumps({'original_slots':slots,'materials':[mesh.get_material(i).get_path_name() for i in range(len(slots))]},indent=2)+'\n');unreal.SystemLibrary.quit_editor()
