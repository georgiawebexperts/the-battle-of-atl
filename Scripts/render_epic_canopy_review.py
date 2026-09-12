"""Fresh-process review of saved Mac tree settings at existing geographic stations.
Transient actors only: never saves the park or replaces its PCG data.
"""
import unreal, pathlib, json
root = pathlib.Path(unreal.Paths.project_dir())
out = root / 'work/epic-canopy-review'
out.mkdir(parents=True, exist_ok=True)
manifest = json.loads((root/'SourceAssets/Manifests/epic-tree-migration.json').read_text())
meshes = [unreal.load_asset(p) for p in manifest['roots']]
for mesh in meshes:
    assert mesh and not mesh.get_editor_property('nanite_settings').get_editor_property('enabled')
texture_checks=[]
for path in manifest['packages']:
    if not path.startswith('/Game/EuropeanHornbeam/Textures/'): continue
    texture=unreal.load_asset(path)
    cap=int(texture.get_editor_property('max_texture_size'))
    assert cap==(4096 if '/TwoSided/' in path else 2048),path
    texture_checks.append({'path':path,'max_texture_size':cap})
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
old=[a for a in ea.get_all_level_actors() if 'BattleParkCanopy' in [str(t) for t in a.tags]]
assert old, 'Existing canopy missing'
rows=json.loads((root/'SourceAssets/Terrain/park-tree-stations.json').read_text())['trees']
accepted=[];rejected=[]
for index,row in enumerate(rows):
    x,y=row['xy_cm']
    hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000),0)
    if not hit or not isinstance(hit[1],unreal.Landscape):
        rejected.append(index);continue
    point,_=hit
    mesh=meshes[1 if row['placement']=='mapped_tree' or index%3==0 else 0]
    bounds=mesh.get_bounds();scale=row['height_game_cm']/(2*bounds.box_extent.z)
    location=unreal.Vector(x,y,point.z-(bounds.origin.z-bounds.box_extent.z)*scale)
    actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,location,unreal.Rotator(yaw=row['yaw']))
    actor.static_mesh_component.set_static_mesh(mesh)
    actor.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    actor.set_actor_scale3d(unreal.Vector(scale,scale,scale))
    accepted.append(index)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector())
c=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
t=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',t),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',85)]:c.set_editor_property(prop,value)
for a in old:c.hide_actor_components(a)
views=[('gate-oblique',[-17400,-5200,600],[-14000,-5200,100]),('park-aerial',[-16000,16000,18000],[-5000,-2000,-400]),('lakeshore',[-6000,2000,450],[0,-4000,450])]
for name,position,target in views:
    pos=unreal.Vector(*position)
    cam.set_actor_location(pos,False,False)
    cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(pos,unreal.Vector(*target)),False)
    for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
    unreal.RenderingLibrary.export_render_target(world,t,str(out),name+'.png')
(out/'manifest.json').write_text(json.dumps({'map_saved':False,'mesh_settings_reloaded':True,'texture_checks':texture_checks,'instances':len(accepted),'rejected_indices':rejected,'automatic_lod':True,'lights_modified':False,'scope':'Transient static geographic canopy appearance only; PCG integration, trunk collision and gameplay performance pending'},indent=2)+'\n')
print('BATTLE_EPIC_CANOPY_REVIEW_COMPLETE',len(accepted))
