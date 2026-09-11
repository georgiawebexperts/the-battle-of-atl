"""Install OSM Lake Clara Meer, its island exclusion and solid shoreline."""
import unreal,json,pathlib,traceback
p=pathlib.Path(unreal.Paths.project_dir());data=p/'SourceAssets/Terrain';lake_data=json.loads((data/'lake-clara-meer.json').read_text());meta=json.loads((data/'terrain-georeference.json').read_text());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if w.get_name()!='PiedmontWorld':raise RuntimeError('Lake installation requires PiedmontWorld')
import sys;sys.path.insert(0,str(p/'Scripts'))
from battle_geography import source_to_world,place_source_geometry,require_converted_world
require_converted_world(w)
try:
 actors=ea.get_all_level_actors();by_label={a.get_actor_label():a for a in actors}
 label='USGS Atlanta terrain + authored underwater basin'
 land=by_label.get(label)
 if not land:
  old=[a for a in actors if isinstance(a,unreal.Landscape)]
  land=unreal.PiedmontWorldTools.import_measured_landscape(str(data/'atlanta-height-lakebed.r16'),*meta['size'],unreal.Vector(*meta['world_location_cm']),unreal.Vector(*meta['world_scale']))
  if not land or len(land.get_components_by_class(unreal.LandscapeComponent))!=288:raise RuntimeError('Replacement terrain failed validation; original retained')
  land.set_editor_property('landscape_material',unreal.load_asset('/Game/PiedmontRide/Materials/M_Grass'));land.set_actor_label(label)
  for a in old:ea.destroy_actor(a)
 ring=[source_to_world(v) for v in lake_data['outer_cm']];island=[source_to_world(v) for v in lake_data['island_cm']];cx=sum(v[0] for v in ring[:-1])/(len(ring)-1);cy=sum(v[1] for v in ring[:-1])/(len(ring)-1);z=lake_data['water_z_cm']
 zones=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.WaterZone)]
 zone=zones[0] if zones else ea.spawn_actor_from_class(unreal.WaterZone,unreal.Vector(cx,cy,z))
 zone.set_actor_label('Piedmont lake water zone');zone.set_editor_property('zone_extent',unreal.Vector2D(40000,40000))
 lake=by_label.get('Lake Clara Meer') or ea.spawn_actor_from_class(unreal.WaterBodyLake,unreal.Vector(cx,cy,z+1))
 lake.set_actor_scale3d(unreal.Vector(1,1,1));lake.set_actor_location(unreal.Vector(cx,cy,z+1),False,True)
 lake.set_actor_label('Lake Clara Meer');lake.set_folder_path('Piedmont/Lake');lake.tags=[unreal.Name('RideWater')]
 spline=lake.get_component_by_class(unreal.WaterSplineComponent);spline.clear_spline_points(False)
 water_ring=ring[:-1]
 if sum(a[0]*b[1]-b[0]*a[1] for a,b in zip(ring,ring[1:]))<0:water_ring=list(reversed(water_ring))
 for v in water_ring:spline.add_spline_point(unreal.Vector(v[0]-cx,v[1]-cy,0),unreal.SplineCoordinateSpace.LOCAL,False)
 for i in range(len(ring)-1):spline.set_spline_point_type(i,unreal.SplinePointType.LINEAR,False)
 spline.set_closed_loop(True,True)
 component=lake.get_component_by_class(unreal.WaterBodyComponent);component.set_editor_property('affects_landscape',False)
 # Separate hazard/shore actors own gameplay collision; use the spline for water tiles.
 component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
 component.set_water_material(unreal.load_asset('/Water/Materials/WaterSurface/Water_Material_Lake'))
 component.set_water_zone_override(zone)
 component.set_water_info_material(unreal.load_asset('/Water/Materials/WaterInfo/DrawWaterInfo'))
 component.set_water_static_mesh_material(unreal.load_asset('/Water/Materials/WaterSurface/LODs/Water_Material_Lake_LOD'))
 component.set_water_body_static_mesh_enabled(True)
 component.set_editor_property('max_wave_height_offset',5.0)
 if not unreal.PiedmontWorldTools.refresh_water_body(lake):raise RuntimeError('Water geometry refresh failed')
 hazard=by_label.get('Lake Clara Meer water recovery') or ea.spawn_actor_from_class(unreal.PiedmontWaterHazard,unreal.Vector(cx,cy,z))
 hazard.set_actor_scale3d(unreal.Vector(1,1,1));hazard.set_actor_location(unreal.Vector(cx,cy,z),False,True)
 hazard.set_actor_label('Lake Clara Meer water recovery');hazard.set_folder_path('Piedmont/Lake')
 hazard.set_editor_property('polygon',[unreal.Vector(v[0]-cx,v[1]-cy,0) for v in ring[:-1]])
 hazard.set_editor_property('island_polygon',[unreal.Vector(v[0]-cx,v[1]-cy,0) for v in island[:-1]])
 # Hidden geometry provides solid contact; the polygon also catches airborne entry.
 name='SM_LakeClaraMeerShore';dest='/Game/PiedmontRide/Environment/Park'
 options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;options.static_mesh_import_data.remove_degenerates=False
 task=unreal.AssetImportTask();task.filename=str(data/'LakeShoreCollision.obj');task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=options
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name)
 mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
 se=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem);settings=se.get_nanite_settings(mesh);settings.enabled=True;settings.position_precision=8;settings.generate_fallback=unreal.NaniteGenerateFallback.ENABLED;settings.fallback_target=unreal.NaniteFallbackTarget.PERCENT_TRIANGLES;settings.fallback_percent_triangles=1.;se.set_nanite_settings(mesh,settings,True);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 wall=by_label.get('Lake Clara Meer solid shore') or ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
 wall.set_actor_label('Lake Clara Meer solid shore');wall.set_folder_path('Piedmont/Lake');wall.static_mesh_component.set_static_mesh(mesh);wall.static_mesh_component.set_collision_profile_name('BlockAll');wall.tags=[unreal.Name('RideWater')];wall.set_actor_hidden_in_game(True);wall.set_is_temporarily_hidden_in_editor(True)
 place_source_geometry(wall)
 level.save_current_level()
 unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(unreal.Vector(cx-11000,cy-12000,19000),unreal.Rotator(pitch=-55,yaw=48,roll=0))
 result={'status':'installed_pending_playtest','map':w.get_name(),'water_level_real_m':lake_data['water_elevation_real_m'],'outer_spline_points':spline.get_number_of_spline_points(),'island_points':len(island)-1,'landscape_components':len(land.get_components_by_class(unreal.LandscapeComponent)),'shore_collision':str(se.get_collision_complexity(mesh))}
except Exception:result={'status':'error','error':traceback.format_exc()}
(p/'Scripts/park-lake-built.json').write_text(json.dumps(result,indent=2))
