"""Install the sourced Irwin-to-Krog trail into the converted park world."""
import unreal,json,pathlib,sys,math
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
base=root/'SourceAssets/Terrain/KrogRoute'
manifest=json.loads((base/'manifest.json').read_text())
network=json.loads((root/'SourceAssets/Terrain/krog-route-network.json').read_text())
existing={a.get_actor_label():a for a in ea.get_all_level_actors()}
# Preserve measured collision outside this local excavation before replacing terrain.
baseline=[]
for filename in ['park-path-network.json','eastside-trail-network.json']:
    f=root/'SourceAssets/Terrain'/filename
    if not f.exists():continue
    for path in json.loads(f.read_text())['paths']:
        points=path.get('points_cm',[])
        for p in points[::max(1,len(points)//8)]:
            v=source_vector(p)
            hit=unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,150),v-unreal.Vector(0,0,150))
            if hit:baseline.append((v,hit[0].z))
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
old=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.Landscape)]
assert len(old)==1
land=unreal.PiedmontWorldTools.import_measured_landscape(str(root/'SourceAssets/Terrain/atlanta-height-krog.r16'),meta['size'][0],meta['size'][1],unreal.Vector(*meta['world_location_cm']),unreal.Vector(*meta['world_scale']))
assert land and len(land.get_components_by_class(unreal.LandscapeComponent))==288
land.set_editor_property('landscape_material',old[0].get_editor_property('landscape_material'))
label=old[0].get_actor_label();land.tags=list(old[0].tags)
assert ea.destroy_actor(old[0]);land.set_actor_label(label)
assert unreal.PiedmontWorldTools.refresh_landscape_collision(land)
unreal.PiedmontWorldTools.finish_editor_asset_loading()

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
    label='Krog route '+name
    a=existing.get(label) or ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
    a.set_actor_label(label);a.set_folder_path('BattleForTheA/BeltLine');a.static_mesh_component.set_static_mesh(mesh)
    place_source_geometry(a)
    a.static_mesh_component.set_collision_profile_name('BlockAll');a.tags=([unreal.Name('RideBarrier')] if chunk.get('structure') in ['Shell','Columns'] else [unreal.Name('RidePath'),unreal.Name('BattleKrogRoute')])
    rows.append({'mesh':name,'bounds_match':True})
for i,path in enumerate(network['paths']):
    label='Krog route spline '+str(i);a=existing.get(label) or ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector())
    a.set_actor_label(label);a.set_folder_path('BattleForTheA/BeltLine')
    a.set_editor_property('osm_way_id',str(path['osm_id']));a.set_editor_property('width_cm',path['width_game_cm']);a.set_editor_property('artifact_eligible',False);a.set_editor_property('bridge',path['tags'].get('bridge')=='yes')
    a.set_centerline([source_vector(v) for v in path['points_cm']])
    a.tags=[unreal.Name('PiedmontPathSource'),unreal.Name('BattleKrog_'+str(i))]
for i,zone in enumerate(manifest['dark_zones']):
    a=source_vector(zone['a']);b=source_vector(zone['b']);d=b-a
    actor=existing.get('Krog darkness '+str(i)) or ea.spawn_actor_from_class(unreal.PiedmontDarkZone,unreal.Vector())
    actor.set_actor_location_and_rotation((a+b)*.5,unreal.Rotator(pitch=0,yaw=math.degrees(math.atan2(d.y,d.x)),roll=0),False,True)
    actor.set_actor_label('Krog darkness '+str(i));actor.set_folder_path('BattleForTheA/BeltLine')
    actor.get_component_by_class(unreal.BoxComponent).set_box_extent(unreal.Vector(d.length()*.5+2,zone['half_width'],zone['half_height']))
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert len(baseline)>1900,'Incomplete baseline coverage'
regression_max=0
for v,z in baseline:
    hit=unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,150),v-unreal.Vector(0,0,150))
    assert hit and abs(hit[0].z-z)<.5,(v,z,str(hit))
    regression_max=max(regression_max,abs(hit[0].z-z))
roof_errors=[]
for p in manifest['roof_samples']:
    v=source_vector(p)
    hit=unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,200),v+unreal.Vector(0,0,400))
    assert hit and hit[1].actor_has_tag('RideBarrier'),('Missing overhead tunnel collision',p,str(hit),str(unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,400),v+unreal.Vector(0,0,200))))
    error=abs(hit[0].z-v.z-270);assert error<.5,(p,error)
    roof_errors.append(error)
samples=[]
for path in network['paths']:
    for source_p in path['points_cm']:
        world_p=source_vector(source_p);p=[world_p.x,world_p.y,world_p.z]
        hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(p[0],p[1],p[2]+100),unreal.Vector(p[0],p[1],p[2]-100))
        assert hit,'Missing Krog collision'
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
assert length>0,'Krog is not reachable in navigation'
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontWorld')
(root/'Tests/Results/2026-09-11-krog-installed.json').write_text(json.dumps({'passed':True,'collision_samples':samples,'navigation_length_cm':length,'meshes':rows,'preserved_surface_samples':len(baseline),'preserved_surface_max_error_cm':regression_max,'roof_error_cm':roof_errors,'scope':'Installed collision and navigation; rendered appearance and physical riding require separate checks'},indent=2))
