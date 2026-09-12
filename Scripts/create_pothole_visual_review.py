"""Place one visible shallow pothole in an isolated review with fixed cameras."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/IrwinTraffic';site=json.loads((folder/'pothole-site.json').read_text());x,y,z=site['xyz']
assert json.loads((root/'Tests/Results/2026-09-12-irwin-pothole-road-support.json').read_text())['passed']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinPotholeReview');ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.BattleRoadTrafficDirector):ea.destroy_actor(a)
dest='/Game/BattleForTheA/Environment/IrwinTraffic';opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
task=unreal.AssetImportTask();task.filename=str(folder/'Pothole_Surface.obj');task.destination_path=dest;task.destination_name='SM_Pothole_Surface';task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/SM_Pothole_Surface');assert mesh
mat=unreal.load_asset(dest+'/M_PotholeInterior')
if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_PotholeInterior',dest,unreal.Material,unreal.MaterialFactoryNew())
lib=unreal.MaterialEditingLibrary;lib.delete_all_material_expressions(mat)
def n(cls):return lib.create_material_expression(mat,cls)
def wire(a,b,pin):assert lib.connect_material_expressions(a,'',b,pin)
def scalar(v):
 a=n(unreal.MaterialExpressionConstant);a.set_editor_property('r',v);return a
def color(v):
 a=n(unreal.MaterialExpressionConstant3Vector);a.set_editor_property('constant',unreal.LinearColor(*v));return a
# Neutral exposed asphalt aggregate instead of a flat brown fill.
noise=n(unreal.MaterialExpressionNoise);noise.set_editor_property('scale',1.1);noise.set_editor_property('quality',2);noise.set_editor_property('levels',2);noise.set_editor_property('output_min',0);noise.set_editor_property('output_max',1)
aggregate=n(unreal.MaterialExpressionLinearInterpolate);wire(color((.001,.0012,.0015)),aggregate,'A');wire(color((.009,.01,.012)),aggregate,'B');wire(noise,aggregate,'Alpha')
# UV centre is (0.5,0.5), invariant under importer V flip. Broken rim follows
# an irregular radial mask and coarse noise; physical depression remains road mesh.
uv=n(unreal.MaterialExpressionTextureCoordinate)
centre=n(unreal.MaterialExpressionConstant2Vector);centre.set_editor_property('r',.5);centre.set_editor_property('g',.5)
local=n(unreal.MaterialExpressionSubtract);wire(uv,local,'A');wire(centre,local,'B')
radius=n(unreal.MaterialExpressionLength);wire(local,radius,'')
edge=n(unreal.MaterialExpressionSmoothStep);wire(scalar(.295),edge,'Min');wire(scalar(.42),edge,'Max');wire(radius,edge,'Value')
coarse=n(unreal.MaterialExpressionNoise);coarse.set_editor_property('scale',.2);coarse.set_editor_property('levels',1);coarse.set_editor_property('output_min',0);coarse.set_editor_property('output_max',1)
rim=n(unreal.MaterialExpressionMultiply);wire(edge,rim,'A');wire(coarse,rim,'B')
base=n(unreal.MaterialExpressionLinearInterpolate);wire(aggregate,base,'A');wire(color((.06,.063,.065)),base,'B');wire(rim,base,'Alpha');lib.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
lib.connect_material_property(scalar(1),'',unreal.MaterialProperty.MP_ROUGHNESS)
lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
mesh.set_material(0,mat);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
hole=ea.spawn_actor_from_class(unreal.BattlePothole,unreal.Vector(x,y,z));hole.set_actor_label('Shallow pothole candidate');hole.tags=[unreal.Name('PotholeReview')];hole.get_editor_property('Visual').set_static_mesh(mesh);hole.set_editor_property('ContactRadius',site['contact_radius_cm']);hole.set_editor_property('bDeep',False)
gate=ea.spawn_actor_from_class(unreal.BattleRoadCrossing,unreal.Vector(x,y,z));gate.tags=[unreal.Name('MonroeCrossingReview')]
for sign in [-1,1]:
 car=ea.spawn_actor_from_class(unreal.BattleRoadCar,unreal.Vector(x+sign*1000,y,z+73.3));car.tags=list(car.tags)+[unreal.Name('MonroeCarLaneReview')];car.set_editor_property('route',[unreal.Vector(x+sign*1000,y,z),unreal.Vector(x-sign*1000,y,z)])
 binding=unreal.BattleCarCrossing();binding.set_editor_property('crossing',gate);binding.set_editor_property('stop_distance',1000);car.set_editor_property('crossings',[binding])
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontPotholeVisualReview')
(root/'Tests/Results/2026-09-12-pothole-visual-placement.json').write_text(json.dumps({'main_map_changed':False,'potholes':1,'depth_cm':8,'scope':'Shallow visual/physical candidate and contact actor; fictitious gate/car fixtures only provide existing camera review positions.'},indent=2)+'\n')
