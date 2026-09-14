"""Review separate ground-level market footplates without changing the main map."""
import unreal,json,pathlib,hashlib
root=pathlib.Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest()
s=(root/'Scripts/review_park_ground_diagnostic.py').read_text();prefix=s[:s.index('rows=[]')]
exec(compile(prefix,'park_ground_review_setup','exec'))
out=root/'work/market-grounded-feet';out.mkdir(exist_ok=True)
def capture(label):
 for _ in range(32):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),label+'.png')
capture('before')
dest='/Game/BattleForTheA/Environment/TwelfthMarket/GroundedFeet'
opt=unreal.FbxImportUI();opt.import_as_skeletal=False;opt.import_materials=False;opt.import_textures=False;opt.automated_import_should_detect_type=False;opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False
job=unreal.AssetImportTask();job.filename=str(root/'SourceAssets/Terrain/TwelfthMarket/Stall/GroundedFeet/SM_MarketStall_Metal.obj');job.destination_path=dest;job.destination_name='SM_MarketMetalWithoutPads';job.automated=True;job.save=True;job.replace_existing=True;job.options=opt
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([job]);mesh=unreal.load_asset(dest+'/SM_MarketMetalWithoutPads');assert mesh
mat=unreal.load_asset('/Game/BattleForTheA/Environment/TwelfthMarket/M_Metal');mesh.set_material(0,mat);mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
market=[a for a in ea.get_all_level_actors() if unreal.Name('TwelfthStreetMarket') in a.tags];bylabel={a.get_actor_label():a for a in market};metals=[a for a in market if a.static_mesh_component.static_mesh.get_name()=='SM_MarketStall_Metal'];assert len(metals)==10
for a in metals:a.static_mesh_component.set_static_mesh(mesh)
rows=json.loads((root/'Tests/Results/2026-09-14-market-feet.json').read_text())['feet'];pads=[]
for r in rows:
 x,y,z=r['xyz'];support=bylabel[r['support']] if r['support'] else None
 if support:
  origin,extent=support.get_actor_bounds(False);z=origin.z-extent.z
 pad=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,z+1.9),bylabel[r['stall']].get_actor_rotation());pad.set_actor_label(r['stall']+' ground plate '+str(r['foot']));pad.tags=[unreal.Name('MarketGroundPlateReview')]
 pad.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'));pad.static_mesh_component.set_material(0,mat);pad.static_mesh_component.set_collision_profile_name('NoCollision');w=.16 if r['foot']<4 else .07;pad.set_actor_scale3d(unreal.Vector(w,w,.04));pads.append(pad)
unreal.PiedmontWorldTools.finish_editor_asset_loading();capture('after')
assert hashlib.sha256(main.read_bytes()).hexdigest()==before
(root/'Tests/Results/2026-09-14-market-grounded-feet-review.json').write_text(json.dumps({'map_saved':False,'main_map_sha256':before,'changed_metal_instances':len(metals),'new_ground_plates':len(pads),'images':[str(out/'before.png'),str(out/'after.png')],'scope':'Transient visual candidate. New metal mesh asset saved, main map unchanged. Native contact and collision acceptance of candidate pending.'},indent=2)+'\n')
