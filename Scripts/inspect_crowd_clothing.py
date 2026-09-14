"""Read clothing material parameter names before selecting per-person colors."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());rows=[]
for path in ['/Game/CitySampleCrowd/Character/Male/NormalWeight/Materials/MI_m_nrw_crewneck','/Game/CitySampleCrowd/Character/Male/NormalWeight/Materials/MI_m_nrw_jeans','/Game/CitySampleCrowd/Character/Female/NormalWeight/Materials/MI_f_nrw_scoopneck']:
 m=unreal.load_asset(path);assert m,path
 rows.append({'path':path,'vectors':[str(n) for n in unreal.MaterialEditingLibrary.get_vector_parameter_names(m)],'switches':{str(n):unreal.MaterialEditingLibrary.get_material_instance_static_switch_parameter_value(m,n) for n in unreal.MaterialEditingLibrary.get_static_switch_parameter_names(m)}})
(root/'Tests/Results/2026-09-14-crowd-material-parameters.json').write_text(json.dumps(rows,indent=2)+'\n')
