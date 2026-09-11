"""Create editable PCG canopy with an explicitly interim Epic template mesh."""
import unreal,pathlib,json,sys,time
root=pathlib.Path(unreal.Paths.project_dir());sys.path.insert(0,str(root/'Scripts'))
from battle_geography import require_converted_world
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();require_converted_world(w)
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any('BattleParkCanopy' in [str(t) for t in a.tags] for a in ea.get_all_level_actors()),'Existing canopy requires review before replacement'
mesh=unreal.load_asset('/Game/SampleScene/Tree/HillTree_02');assert mesh
lib=unreal.MaterialEditingLibrary;assets=unreal.AssetToolsHelpers.get_asset_tools();folder='/Game/BattleForTheA/Environment/Park'
for i,(name,tex,normal,masked) in enumerate([
 ('M_OakBark','T_Craghead_Oak_Tile_01_D','T_Craghead_Oak_Tile_01_N',False),
 ('M_OakBranches','T_Craghead_Oak_LimbTile_02_D','T_Craghead_Oak_LimbTile_02_N',False),
 ('M_OakFronds','T_HillTree_01_Atlas','T_HillTree_01_Atlas_N',True),
 ('M_OakLeaves','T_HillTree_01_Atlas','T_HillTree_01_Atlas_N',True)]):
 mat=unreal.load_asset(folder+'/'+name) or assets.create_asset(name,folder,unreal.Material,unreal.MaterialFactoryNew())
 lib.delete_all_material_expressions(mat)
 color=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-500,0);color.texture=unreal.load_asset('/Game/SampleScene/Tree/Textures/'+tex);assert color.texture
 lib.connect_material_property(color,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
 n=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-500,300);n.texture=unreal.load_asset('/Game/SampleScene/Tree/Textures/'+normal);n.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
 lib.connect_material_property(n,'RGB',unreal.MaterialProperty.MP_NORMAL)
 rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant,-200,400);rough.r=.82;lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 if masked:
  mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED);mat.set_editor_property('two_sided',True)
  lib.connect_material_property(color,'A',unreal.MaterialProperty.MP_OPACITY_MASK)
 mat.set_editor_property('used_with_instanced_static_meshes',True)
 lib.recompile_material(mat);mesh.set_material(i,mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
stations=json.loads((root/'SourceAssets/Terrain/park-tree-stations.json').read_text());bound=mesh.get_bounds();height=bound.box_extent.z*2;bottom=bound.origin.z-bound.box_extent.z
transforms=[];rejected=[]
for index,row in enumerate(stations['trees']):
 x,y=row['xy_cm'];result=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000),0)
 if not result:rejected.append(index);continue
 point,actor=result
 if not isinstance(actor,unreal.Landscape):rejected.append(index);continue
 scale=row['height_game_cm']/height
 transforms.append(unreal.Transform(location=unreal.Vector(x,y,point.z-bottom*scale),rotation=unreal.Rotator(yaw=row['yaw']),scale=unreal.Vector(scale,scale,scale)))
assert len(transforms)>300
actor=unreal.PiedmontWorldTools.create_park_foliage(transforms,mesh);assert actor,'PCG creation failed'
component=actor.get_component_by_class(unreal.PCGComponent)
count=0
for i in range(900):
 unreal.PiedmontWorldTools.tick_scene_review()
 count=sum(c.get_instance_count() for a in ea.get_all_level_actors() if 'BattleParkCanopy' in [str(t) for t in a.tags] for c in a.get_components_by_class(unreal.InstancedStaticMeshComponent))
 if count==len(transforms):break
assert count==len(transforms),(count,len(transforms))
unreal.EditorAssetLibrary.save_asset(folder+'/DA_TreeStations');unreal.EditorAssetLibrary.save_asset(folder+'/PCG_ParkCanopy')
assert unreal.EditorLoadingAndSavingUtils.save_map(w,'/Game/PiedmontRide/Maps/PiedmontWorld')
(root/'work/park-canopy-install.json').write_text(json.dumps({'instances':count,'rejected_station_indices':rejected,'pcg_graph':folder+'/PCG_ParkCanopy','map_saved':True,'asset':'Epic ArchVis HillTree_02 with rebuilt materials','megascans_species_requirement_met':False,'collision':'visual canopy only; trunk collision pending','render_review_pending':True},indent=2)+'\n')
