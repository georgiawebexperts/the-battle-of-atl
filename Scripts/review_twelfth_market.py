"""Import original stall parts, survey and render transient market. Never save map."""
import unreal,json,math,pathlib,uuid
root=pathlib.Path(unreal.Paths.project_dir()).resolve();folder=root/'SourceAssets/Terrain/TwelfthMarket';out=root/'work/market-review'/uuid.uuid4().hex;out.mkdir(parents=True)
dest='/Game/BattleForTheA/Environment/TwelfthMarket';colors={'Canvas':(.73,.70,.61),'Metal':(.20,.22,.23),'Wood':(.27,.15,.065),'Leaf':(.15,.30,.035),'Tomato':(.55,.035,.018),'Cloth':(.12,.24,.15)};meshes={}
for row in json.loads((folder/'Stall/manifest.json').read_text())['surfaces']:
 name=row['material'];mat=unreal.load_asset(dest+'/M_'+name)
 if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+name,dest,unreal.Material,unreal.MaterialFactoryNew())
 unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
 c=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*colors[name]));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 if name in ['Canvas','Cloth','Wood']:
  texture_task=unreal.AssetImportTask();texture_task.filename=str(folder/'Stall'/('T_Market_'+name+'.png'));texture_task.destination_path=dest;texture_task.destination_name='T_Market_'+name;texture_task.automated=True;texture_task.save=True;texture_task.replace_existing=True;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([texture_task])
  sample=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionTextureSample);sample.set_editor_property('texture',unreal.load_asset(dest+'/T_Market_'+name));unreal.MaterialEditingLibrary.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
 v=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant);v.set_editor_property('r',.35 if name=='Metal' else .83);unreal.MaterialEditingLibrary.connect_material_property(v,'',unreal.MaterialProperty.MP_ROUGHNESS)
 mat.set_editor_property('two_sided',name in ['Canvas','Cloth']);unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
 opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/'Stall'/row['file']);task.destination_path=dest;task.destination_name='SM_MarketStall_'+name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+task.destination_name);assert mesh;mesh.set_material(0,mat)
 mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh);meshes[name]=mesh
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
sign_materials=[]
for i in range(5):
 task=unreal.AssetImportTask();task.filename=str(folder/'Stall'/f'T_MarketSign_{i}.png');task.destination_path=dest;task.destination_name=f'T_MarketSign_{i}';task.automated=True;task.save=True;task.replace_existing=True;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 mat=unreal.load_asset(dest+f'/M_MarketSign_{i}')
 if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset(f'M_MarketSign_{i}',dest,unreal.Material,unreal.MaterialFactoryNew())
 unreal.MaterialEditingLibrary.delete_all_material_expressions(mat);mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT);mat.set_editor_property('two_sided',True)
 sample=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionTextureSample);sample.set_editor_property('texture',unreal.load_asset(dest+f'/T_MarketSign_{i}'));unreal.MaterialEditingLibrary.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR);unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat);sign_materials.append(mat)
task=unreal.AssetImportTask();task.filename=str(folder/'Stall/SM_MarketSign.obj');task.destination_path=dest;task.destination_name='SM_MarketSign';task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);sign_mesh=unreal.load_asset(dest+'/SM_MarketSign');assert sign_mesh;sign_mesh.set_material(0,sign_materials[0]);unreal.EditorAssetLibrary.save_loaded_asset(sign_mesh)
lighting=[]
for sky_actor in ea.get_all_level_actors():
 if isinstance(sky_actor,unreal.SkyLight):
  sky=sky_actor.get_component_by_class(unreal.SkyLightComponent)
  row={'actor':sky_actor.get_actor_label(),'intensity':sky.get_editor_property('intensity'),'mobility':str(sky.get_editor_property('mobility')),'real_time_capture':sky.get_editor_property('real_time_capture'),'source_type':str(sky.get_editor_property('source_type'))}
  if '-MarketRecaptureSky' in unreal.SystemLibrary.get_command_line():sky.set_editor_property('real_time_capture',False);sky.recapture_sky();row['review_recapture']=True;row['review_real_time_capture']=False
  lighting.append(row)
# Thin ground-following pavement should not project detached shadows onto terrain.
if '-MarketPathShadowReview' in unreal.SystemLibrary.get_command_line():
 for actor in ea.get_all_level_actors():
  if actor.get_actor_label() in ['Park pavement SM_Park_Concrete_2_16','Park pavement SM_Park_Asphalt_2_16']:
   actor.static_mesh_component.set_cast_shadow(False)
layout=json.loads((folder/'layout.json').read_text());survey=[]
bounds=meshes['Wood'].get_bounding_box();table_sign=1 if bounds.min.y+bounds.max.y>0 else -1
for index,row in enumerate(layout['stalls']):
 samples=[]
 yaw=row['yaw_degrees']+(180 if row['side']==table_sign else 0);angle=math.radians(yaw);c,s=math.cos(angle),math.sin(angle);cx,cy=row['center_xy']
 feet=row['footprint_xy']+[[cx+c*x-s*y,cy+s*x+c*y] for x in [-90,90] for y in [table_sign*80,table_sign*40]]
 for x,y in [row['center_xy']]+feet:
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1500),unreal.Vector(x,y,-1500));assert hit
  samples.append({'xy':[x,y],'z':hit[0].z,'actor':hit[1].get_actor_label()})
 base=max(s['z'] for s in samples);survey.append({'stall':index+1,'samples':samples,'base_z':base,'ground_spread_cm':base-min(s['z'] for s in samples)})
 # Grounded levelling blocks keep all eight legs supported on sloped pavement.
 supports=[]
 for sample in samples[1:]:
  height=base-sample['z']
  if height<.1:continue
  x,y=sample['xy'];pad=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,sample['z']+height/2),unreal.Rotator(yaw=yaw));pad.set_actor_label(f'Market review {index+1} levelling block');pad.tags=[unreal.Name('MarketReview')];pad.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'));pad.static_mesh_component.set_material(0,unreal.load_asset(dest+'/M_Wood'));pad.set_actor_scale3d(unreal.Vector(.18,.18,height/100));pad.static_mesh_component.set_collision_profile_name('BlockAll');supports.append({'xy':[x,y],'bottom_z':sample['z'],'top_z':base})
 survey[-1]['levelling_blocks']=supports

 for name,mesh in meshes.items():
  actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*row['center_xy'],base),unreal.Rotator(yaw=yaw));actor.set_actor_label(f'Market review {index+1} {name}');actor.tags=[unreal.Name('MarketReview')];actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll' if name in ['Wood','Metal'] else 'NoCollision')
 # Baked sign panel on the aisle side; static mesh avoids text-component rendering.
 bx,by=cx-s*table_sign*151,cy+c*table_sign*151
 board=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(bx,by,base+202),unreal.Rotator(yaw=yaw));board.set_actor_scale3d(unreal.Vector(2.7,.02,.36));board.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'));board.static_mesh_component.set_material(0,unreal.load_asset(dest+'/M_Cloth'));board.static_mesh_component.set_collision_profile_name('NoCollision');board.tags=[unreal.Name('MarketReview')]
 sign=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(cx,cy,base),unreal.Rotator(yaw=yaw));sign.tags=[unreal.Name('MarketReview')];sign.static_mesh_component.set_static_mesh(sign_mesh);sign.static_mesh_component.set_material(0,sign_materials[index//2]);sign.static_mesh_component.set_collision_profile_name('NoCollision')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
if '-MarketClosurePreview' in unreal.SystemLibrary.get_command_line():
 gx,gy=layout['gate_xy'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(gx,gy,1000),unreal.Vector(gx,gy,-1000));assert hit
 closure=ea.spawn_actor_from_class(unreal.BattleMarketClosure,hit[0]);closure.tags=list(closure.tags)+[unreal.Name('MarketReview')];assert closure.build_grounded_return()
 return_points=list(closure.get_editor_property('return_ground'));return_checks=[]
 for a,b in zip(return_points,return_points[1:]):
  dx,dy=b.x-a.x,b.y-a.y;length=math.hypot(dx,dy);nx,ny=-dy/length,dx/length;cx,cy,cz=(a.x+b.x)/2,(a.y+b.y)/2,(a.z+b.z)/2
  for z in [90,180,300]:
   hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(cx-nx*100,cy-ny*100,cz+z),unreal.Vector(cx+nx*100,cy+ny*100,cz+z),30)
   return_checks.append({'xy':[cx,cy],'height':z,'blocked_by_return':bool(hit and hit[1]==closure)})
 return_report={'points':[[v.x,v.y,v.z] for v in return_points],'checks':return_checks,'passed':all(v['blocked_by_return'] for v in return_checks),'max_neighbor_step_cm':max(abs(a.z-b.z) for a,b in zip(return_points,return_points[1:]))}
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',70)]:cap.set_editor_property(prop,value)
gx,gy=layout['gate_xy'];base=survey[0]['base_z'];x,y=layout['stalls'][0]['center_xy']
views=[('entrance',[gx-1000,gy,base+165],[gx+1500,gy,base+120]),('stall',[x+100,y+500,base+150],[x,y-30,base+130]),('wide',[gx+600,gy-1600,base+1200],[gx+1000,gy,base])];images=[]
if '-MarketClosurePreview' in unreal.SystemLibrary.get_command_line():
 views.append(('return',[-19500,-2500,1800],[-18100,-1400,100]))
 for name,v in [('return_market_end',return_points[0]),('return_gate_end',return_points[-1])]:
  views.append((name,[v.x-900,v.y+500,v.z+650],[v.x,v.y,v.z+120]))
for name,p,target in views:
 loc=unreal.Vector(*p);cam.set_actor_location(loc,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(loc,unreal.Vector(*target)),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png');images.append(str(out/(name+'.png')))
result={'path_shadow_diagnostic':'-MarketPathShadowReview' in unreal.SystemLibrary.get_command_line(),'return_report':return_report if '-MarketClosurePreview' in unreal.SystemLibrary.get_command_line() else None,'closure_preview':'-MarketClosurePreview' in unreal.SystemLibrary.get_command_line(),'lighting':lighting,'sign_implementation':'Textured static panels; five baked fictional labels','imported_table_y_sign':table_sign,'stalls':survey,'images':images,'main_map_saved':False,'visual_accepted':False,'scope':'Transient static market study. Measured levelling blocks included. Collision/closure, tutorial routes, detailed materials and gameplay acceptance pending.'}
(root/'Tests/Results/2026-09-12-market-review.json').write_text(json.dumps(result,indent=2)+'\n')
print('MARKET_REVIEW '+str(out))
