"""Create original solid PBR finishes for the fictional starting bungalow."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();dest='/Game/BattleForTheA/Environment/TutorialHouse'
lib=unreal.MaterialEditingLibrary;created=[]
for name,color,rough,metal in [('Siding',(0.18,0.29,0.32),.8,0),('Trim',(.73,.69,.55),.7,0),('Door',(.035,.085,.105),.55,0),('Glass',(.035,.095,.135),.17,.35)]:
 path=dest+'/M_'+name
 mat=unreal.load_asset(path)
 if mat:
  mat.set_editor_property("used_with_instanced_static_meshes",True);lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat);created.append(path);continue
 if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+name,dest,unreal.Material,unreal.MaterialFactoryNew())
 c=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-300,0);c.set_editor_property('constant',unreal.LinearColor(*color,1));lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 for prop,value,y in [(unreal.MaterialProperty.MP_ROUGHNESS,rough,150),(unreal.MaterialProperty.MP_METALLIC,metal,250)]:
  n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant,-300,y);n.set_editor_property('r',value);lib.connect_material_property(n,'',prop)
 mat.set_editor_property("used_with_instanced_static_meshes",True);lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat);created.append(path)
(root/'Tests/Results/2026-09-12-tutorial-house-materials.json').write_text(json.dumps({'created':created,'source':'Original authored PBR colors; no external assets or purchases.','visual_review':'pending'},indent=2)+'\n')
