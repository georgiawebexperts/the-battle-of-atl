import unreal,json,pathlib,traceback
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors={a.get_actor_label():a for a in sub.get_all_level_actors()}
bike=actors['BeltLineBike_Start_LakeLoop'];r=[]
def do(name,fn):
    try:r.append({name:str(fn())})
    except Exception:r.append({name:traceback.format_exc()})
do('movement reference',lambda:bike.set_editor_property('CharMoveComp',bike.get_character_movement()))
do('bike rotation',lambda:bike.set_actor_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0),False))
do('start position',lambda:bike.set_editor_property('StartLocation',bike.get_actor_location()))
do('start rotation',lambda:bike.set_editor_property('StartRotation',bike.get_actor_rotation()))
cam=actors['Glide_ChaseCamera']
do('camera transform',lambda:cam.set_actor_location_and_rotation(bike.get_actor_location()+unreal.Vector(-650,0,350),unreal.Rotator(pitch=-18,yaw=0,roll=0),False,True))
for name in ['Glide_Sun','Glide_SkyLight']:
    a=actors.get(name)
    if a:do(name+' movable',lambda a=a:a.get_component_by_class(unreal.LightComponent).set_mobility(unreal.ComponentMobility.MOVABLE))
for label,shape,offset,scale,rot in [
 ('Glide_BikeFrame','Cube',(0,0,-5),(1.1,.14,.18),(0,0,0)),
 ('Glide_BikeSeat','Cube',(-25,0,25),(.32,.26,.10),(0,0,0)),
 ('Glide_BikeHandle','Cube',(45,0,40),(.10,.65,.10),(0,0,0)),
 ('Glide_BikeStem','Cube',(45,0,15),(.10,.10,.50),(0,0,0)),
 ('Glide_BikeRearWheel','Cylinder',(-55,0,-40),(.65,.65,.12),(0,0,90)),
 ('Glide_BikeFrontWheel','Cylinder',(55,0,-40),(.65,.65,.12),(0,0,90))]:
    a=actors.get(label) or sub.spawn_actor_from_class(unreal.StaticMeshActor,bike.get_actor_location()+unreal.Vector(*offset),unreal.Rotator(pitch=rot[0],yaw=rot[1],roll=rot[2]))
    a.set_actor_label(label)
    c=a.static_mesh_component;c.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/'+shape));c.set_mobility(unreal.ComponentMobility.MOVABLE);c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    a.set_actor_scale3d(unreal.Vector(*scale));a.attach_to_actor(bike,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
do('save',lambda:unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level())
r.append({'MCP_classes':[n for n in dir(unreal) if n.startswith('UnrealMCP') and ('Blueprint' in n or 'Input' in n)]})
pathlib.Path(unreal.Paths.project_dir(),'Scripts','player-repair-result.json').write_text(json.dumps(r,indent=2))
print('PLAYER_REPAIR '+json.dumps(r))
