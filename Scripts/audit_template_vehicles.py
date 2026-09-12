"""Inspect imported Epic template vehicle candidates without changing gameplay."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());rows=[]
for path in ['/Game/Vehicles/SportsCar/SM_SportsCar','/Game/Vehicles/SportsCar/SM_SportsCar_Wheel','/Game/Vehicles/SportsCar/SM_SportsCar_Glass','/Game/Vehicles/SportsCar/SKM_SportsCar','/Game/Vehicles/OffroadCar/SM_Offroad_Body','/Game/Vehicles/OffroadCar/SKM_Offroad']:
 a=unreal.load_asset(path);assert a,path
 r={'path':path,'class':a.get_class().get_name()}
 if isinstance(a,unreal.StaticMesh):
  b=a.get_bounds();r['bounds_origin']=[b.origin.x,b.origin.y,b.origin.z];r['size_cm']=[b.box_extent.x*2,b.box_extent.y*2,b.box_extent.z*2];r['materials']=[m.material_interface.get_path_name() if m.material_interface else None for m in a.static_materials]
 else:
  c=unreal.SkeletalMeshComponent();c.set_skeletal_mesh_asset(a);r['bones']=[str(c.get_bone_name(i)) for i in range(c.get_num_bones())]
 rows.append(r)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
(root/'Tests/Results/2026-09-12-template-vehicle-assets.json').write_text(json.dumps({'assets':rows,'gameplay_installed':False,'visual_acceptance':False},indent=2)+'\n')
