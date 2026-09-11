"""The single boundary between retained ENU source geometry and Unreal ESU.

Raw DEM rows, baked OBJ geometry and source network points retain their original
east/north/up centimetres. In Unreal, east is +X, north is -Y, up is +Z.
Geographic source geometry therefore needs a reflected actor transform. Assets
already authored for Unreal (riders, trees, props) only need converted placement.
"""
WORLD_MARKER='BattleGeography_ESU_v1'

def source_to_world(point):
    return [point[0],-point[1],*point[2:]]

def source_yaw_to_world(yaw):
    return -yaw

def require_converted_world(world):
    assert WORLD_MARKER in [str(t) for t in world.get_world_settings().tags], 'Convert the source-space park before editing its geographic placement.'

def place_source_geometry(actor, location=(0,0,0), rotation=(0,0,0), scale=(1,1,1)):
    import unreal
    pitch,yaw,roll=rotation
    actor.set_actor_location_and_rotation(unreal.Vector(*source_to_world(location)),unreal.Rotator(pitch=pitch,yaw=-yaw,roll=-roll),False,True)
    actor.set_actor_scale3d(unreal.Vector(*source_to_world(scale)))

def source_vector(point):
    import unreal
    return unreal.Vector(*source_to_world(point))


def import_source_landscape(filename, meta):
    """Import ENU DEM samples with a positive physics transform when configured."""
    import unreal, array, pathlib, sys
    source=pathlib.Path(filename);width,height=meta['size']
    yaw=0
    if meta.get('landscape_import_layout')=='transpose_xy':
        values=array.array('H');values.frombytes(source.read_bytes())
        if sys.byteorder!='little':values.byteswap()
        assert len(values)==width*height
        transposed=array.array('H')
        for x in range(width):transposed.extend(values[x::width])
        if sys.byteorder!='little':transposed.byteswap()
        target=source.parent/'ImportFrames'/(source.stem+'-transposed.r16')
        target.parent.mkdir(exist_ok=True);target.write_bytes(transposed.tobytes())
        filename=str(target);width,height=height,width;yaw=-90
    land=unreal.PiedmontWorldTools.import_measured_landscape(str(filename),width,height,unreal.Vector(*meta['world_location_cm']),unreal.Vector(*meta['world_scale']))
    if land:
        land.set_actor_rotation(unreal.Rotator(pitch=0,yaw=yaw,roll=0),False)
        assert unreal.PiedmontWorldTools.refresh_landscape_collision(land)
    return land
