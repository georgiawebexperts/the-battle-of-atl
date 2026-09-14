"""Save reviewed market footplates in an isolated map, or promote after validation."""
import unreal,json,pathlib,hashlib,shutil
root=pathlib.Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest();install='-InstallGroundedMarketFeet' in unreal.SystemLibrary.get_command_line()
reference=json.loads((root/'Tests/Results/2026-09-14-market-grounded-feet-review.json').read_text());assert before==reference['main_map_sha256']
if install:
 check=json.loads((root/'Tests/Results/2026-09-14-market-grounded-feet-saved.json').read_text());assert check['passed'] and not check['main_map']
 backup=root/'work/map-backups'/('PiedmontWorld-before-grounded-market-'+before[:12]+'.umap');backup.parent.mkdir(exist_ok=True);shutil.copy2(main,backup);assert hashlib.sha256(backup.read_bytes()).hexdigest()==before
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
mesh=unreal.load_asset('/Game/BattleForTheA/Environment/TwelfthMarket/GroundedFeet/SM_MarketMetalWithoutPads');assert mesh
mat=unreal.load_asset('/Game/BattleForTheA/Environment/TwelfthMarket/M_Metal')
market=[a for a in ea.get_all_level_actors() if unreal.Name('TwelfthStreetMarket') in a.tags];bylabel={a.get_actor_label():a for a in market};metals=[a for a in market if a.static_mesh_component.static_mesh.get_name()=='SM_MarketStall_Metal'];assert len(metals)==10
assert not any(unreal.Name('MarketGroundPlate') in a.tags for a in ea.get_all_level_actors())
for a in metals:a.static_mesh_component.set_static_mesh(mesh)
rows=json.loads((root/'Tests/Results/2026-09-14-market-feet.json').read_text())['feet'];assert len(rows)==80
for r in rows:
 x,y,z=r['xyz'];nearby=[a for a in market if 'adjustable foot' in a.get_actor_label() and (a.get_actor_location().x-x)**2+(a.get_actor_location().y-y)**2<.25**2]
 assert len(nearby)==(1 if r['support'] else 0)
 support=nearby[0] if nearby else None
 if support:
  origin,extent=support.get_actor_bounds(False);z=origin.z-extent.z
 pad=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,z+1.9),bylabel[r['stall']].get_actor_rotation());pad.set_actor_label(r['stall']+' ground plate '+str(r['foot']));pad.tags=[unreal.Name('TwelfthStreetMarket'),unreal.Name('MarketGroundPlate')];pad.set_folder_path('Piedmont/12th Street Market')
 pad.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'));pad.static_mesh_component.set_material(0,mat);pad.static_mesh_component.set_collision_profile_name('NoCollision');w=.16 if r['foot']<4 else .07;pad.set_actor_scale3d(unreal.Vector(w,w,.04))
unreal.PiedmontWorldTools.finish_editor_asset_loading()
path='/Game/PiedmontRide/Maps/'+('PiedmontWorld' if install else 'PiedmontMarketFeetReview');assert unreal.EditorLoadingAndSavingUtils.save_map(world,path)
after=hashlib.sha256(main.read_bytes()).hexdigest();assert install or after==before
(root/'Tests/Results'/('2026-09-14-market-feet-install.json' if install else '2026-09-14-market-feet-save-review.json')).write_text(json.dumps({'map':path,'main_map':install,'before_sha256':before,'after_sha256':after,'metal_instances':10,'plates':80},indent=2)+'\n')
