import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
path='/Game/CitySampleCrowd/Character/Male/NormalWeight/Materials/M_BodySynthesized';m=unreal.load_asset(path);assert m
lib=unreal.MaterialEditingLibrary
r={'material':path,'parent':m.get_editor_property('parent').get_path_name(),'vectors':{},'scalars':{},'textures':{}}
for n in lib.get_vector_parameter_names(m):
 v=lib.get_material_instance_vector_parameter_value(m,n);r['vectors'][str(n)]=[v.r,v.g,v.b,v.a]
for n in lib.get_scalar_parameter_names(m):r['scalars'][str(n)]=lib.get_material_instance_scalar_parameter_value(m,n)
for n in lib.get_texture_parameter_names(m):
 t=lib.get_material_instance_texture_parameter_value(m,n);r['textures'][str(n)]=t.get_path_name() if t else None
(root/'Tests/Results/2026-09-13-detailed-skin-material.json').write_text(json.dumps(r,indent=2)+'\n')
print(r)
