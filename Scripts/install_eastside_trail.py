"""Install the sourced Monroe-to-Irwin trail into the converted park world."""
import unreal,json,pathlib,sys
root=pathlib.Path(unreal.Paths.project_dir())
sys.path.insert(0,str(root/'Scripts'))
from battle_geography import source_vector,place_source_geometry,require_converted_world
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_name()=='PiedmontWorld'
require_converted_world(world)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
base=root/'SourceAssets/Terrain/EastsideTrail'
manifest=json.loads((base/'manifest.json').read_text())
network=json.loads((root/'SourceAssets/Terrain/eastside-trail-network.json').read_text())
existing={a.get_actor_label():a for a in ea.get_all_level_actors()}
rows=[]
for chunk in manifest['chunks']:
    name='SM_'+pathlib.Path(chunk['file']).stem;dest='/Game/BattleForTheA/Environment/BeltLine/Paths'
    options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False
    options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;options.static_mesh_import_data.remove_degenerates=False
    task=unreal.AssetImportTask();task.filename=str(base/chunk['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name);assert mesh
    box=mesh.get_bounding_box();bounds=[[box.min.x,box.min.y,box.min.z],[box.max.x,box.max.y,box.max.z]]
    assert max(abs(bounds[i][j]-chunk['bounds_cm'][i][j]) for i in range(2) for j in range(3))<.1
    mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    nanite=mesh.get_editor_property('nanite_settings')
    nanite.enabled=True;nanite.position_precision=8;nanite.generate_fallback=unreal.NaniteGenerateFallback.ENABLED
    nanite.fallback_target=unreal.NaniteFallbackTarget.PERCENT_TRIANGLES;nanite.fallback_relative_error=0;nanite.fallback_percent_triangles=1
    mesh.set_editor_property('nanite_settings',nanite);mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_'+chunk['material']));unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    label='Eastside trail '+name
    a=existing.get(label) or ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
    a.set_actor_label(label);a.set_folder_path('BattleForTheA/BeltLine');a.static_mesh_component.set_static_mesh(mesh)
    place_source_geometry(a)
    a.static_mesh_component.set_collision_profile_name('BlockAll');a.tags=([unreal.Name('RideBarrier')] if chunk['material']=='BridgeRails' else [unreal.Name('RidePath'),unreal.Name('BattleEastsideRoute')])
    rows.append({'mesh':name,'bounds_match':True})
for i,path in enumerate(network['paths']):
    label='Eastside trail spline '+str(i);a=existing.get(label) or ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector())
    a.set_actor_label(label);a.set_folder_path('BattleForTheA/BeltLine')
    a.set_editor_property('osm_way_id',str(path['osm_id']));a.set_editor_property('width_cm',path['width_game_cm']);a.set_editor_property('artifact_eligible',False);a.set_editor_property('bridge',path['tags'].get('bridge')=='yes')
    a.set_centerline([source_vector(v) for v in path['points_cm']])
    a.tags=[unreal.Name('PiedmontPathSource'),unreal.Name('BattleEastside_'+str(i))]
samples=[]
for path in network['paths']:
    for source_p in path['points_cm']:
        world_p=source_vector(source_p);p=[world_p.x,world_p.y,world_p.z]
        hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(p[0],p[1],p[2]+100),unreal.Vector(p[0],p[1],p[2]-100))
        assert hit,'Missing Eastside collision'
        impact,actor=hit
        error=abs(impact.z-p[2])
        assert actor.actor_has_tag('RidePath') and error<.5,(p,str(actor),error)
        samples.append({'point':p,'error_cm':error})
all_points=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) for i in range(a.centerline.get_number_of_spline_points())]
lo=[min(getattr(p,k) for p in all_points) for k in ['x','y','z']];hi=[max(getattr(p,k) for p in all_points) for k in ['x','y','z']]
assert unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]),unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)]))
assert unreal.PiedmontWorldTools.finish_park_navigation_build()
start=source_vector(network['paths'][0]['points_cm'][0]);end=source_vector(network['paths'][-1]['points_cm'][-1])
length=unreal.PiedmontWorldTools.park_route_length(start,end)
assert length>0,'Eastside is not reachable in navigation'
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontWorld')
(root/'Tests/Results/2026-09-11-eastside-installed.json').write_text(json.dumps({'passed':True,'collision_samples':samples,'navigation_length_cm':length,'meshes':rows,'scope':'Installed collision and navigation; rendered appearance and physical riding require separate checks'},indent=2))
