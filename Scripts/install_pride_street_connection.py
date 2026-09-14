"""Install verified street geometry; preserve traffic actor identities and exclude fixtures."""
import unreal,pathlib,json,hashlib,shutil,math
root=pathlib.Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest();assert before=='775833c9092aba37d2755813294aea17a82f587b5638d6873c47e956a2a02dd7','Rebase against changed main map before installation'
for name in ['pride-drive-arcade-rendered-lookahead300-gate','pride-drive-real-rendered-lookahead400-gate','pride-boundary','pride-boundary-countdown-regression']:
 r=json.loads((root/f'Tests/Results/2026-09-13-{name}.json').read_text());assert r['passed'],name
backup=root/'work/map-backups'/('PiedmontWorld-before-pride-'+before[:12]+'.umap');backup.parent.mkdir(exist_ok=True)
if not backup.exists():shutil.copy2(main,backup)
assert hashlib.sha256(backup.read_bytes()).hexdigest()==before
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);folder=root/'SourceAssets/Terrain/PrideIntersection';dest='/Game/BattleForTheA/Environment/PrideIntersection';preserved=[]
assert not any(a.actor_has_tag('BattlePrideStreet') for a in ea.get_all_level_actors())
for row in json.loads((folder/'cutbacks.json').read_text())['meshes']:
 targets=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.StaticMeshActor) and a.static_mesh_component.static_mesh and a.static_mesh_component.static_mesh.get_name()==row['source_name']];assert len(targets)==1,(row['name'],len(targets));a=targets[0];identity=a.get_path_name();tags=[str(t) for t in a.tags];mesh=unreal.load_asset(dest+'/SM_'+row['name']);assert mesh;oldmat=a.static_mesh_component.get_material(0);a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_material(0,oldmat);assert a.get_path_name()==identity and [str(t) for t in a.tags]==tags;preserved.append(identity)
for row in json.loads((folder/'surfaces.json').read_text())['meshes']:
 mesh=unreal.load_asset(dest+'/SM_'+row['name']);assert mesh;a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('SM_'+row['name']);a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll');a.tags=[unreal.Name('BattlePrideStreet'),unreal.Name('RidePath')];a.set_folder_path('Midtown/PrideIntersection')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
exec(compile((root/'Scripts/add_pride_crosswalks_review.py').read_text(),'add_pride_crosswalks_review.py','exec'),{'__name__':'__main__'})
exec(compile((root/'Scripts/add_pride_street_signs_review.py').read_text(),'add_pride_street_signs_review.py','exec'),{'__name__':'__main__'})
placement=json.loads((root/'work/pride-boundary-placement.json').read_text());a=ea.spawn_actor_from_class(unreal.TargetPoint,unreal.Vector(*placement['position']),unreal.Rotator(yaw=placement['yaw']));a.set_actor_label('Piedmont south construction boundary');a.tags=[unreal.Name('PrideStreetBoundary')];a.set_folder_path('Midtown/PrideIntersection')
assert not any(a.actor_has_tag('PrideDriveFixture') for a in ea.get_all_level_actors())
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.WaterBodyLake):assert unreal.PiedmontWorldTools.refresh_water_body(a)
unreal.PiedmontWorldTools.finish_editor_asset_loading();unreal.PiedmontWorldTools.rebuild_water_zones()
for _ in range(30):unreal.PiedmontWorldTools.tick_scene_review()
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level();after=hashlib.sha256(main.read_bytes()).hexdigest();assert before!=after
(root/'Tests/Results/2026-09-13-pride-main-install.json').write_text(json.dumps({'saved_main':True,'before_sha256':before,'after_sha256':after,'backup':str(backup),'preserved_tenth_actors':preserved,'no_drive_fixtures':True,'desktop_updated':False,'remaining':'Packaged verification. Known visual limitations: dark street signs, small northwest junction artifact and older tutorial pavement joins.'},indent=2)+'\n')
