import unreal,json,pathlib,traceback
root=pathlib.Path(unreal.Paths.project_dir())
try:
    level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    level.new_level('/Game/PiedmontRide/Maps/BikePhysicsLab')
    world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property('default_game_mode',unreal.load_class(None,'/Script/AuraPlayground.PiedmontRideMode'))
    cube=unreal.load_asset('/Engine/BasicShapes/Cube')
    mats={}
    for name,color in [('Asphalt',(0.08,.1,.12)),('Grass',(.11,.23,.09)),('Concrete',(.4,.42,.4)),('Safety',(.95,.38,.04)),('Stripe',(.87,.86,.71))]:
        path='/Game/PiedmontRide/Materials/M_'+name
        m=unreal.load_asset(path)
        if not m:
            m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+name,'/Game/PiedmontRide/Materials',unreal.Material,unreal.MaterialFactoryNew())
            c=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector)
            c.set_editor_property('constant',unreal.LinearColor(*color))
            unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
            unreal.MaterialEditingLibrary.recompile_material(m)
            unreal.EditorAssetLibrary.save_loaded_asset(m)
        mats[name]=m
    def box(name,pos,scale,material='Concrete',tag='',rot=(0,0,0)):
        a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos),unreal.Rotator(pitch=rot[0],yaw=rot[1],roll=rot[2]))
        a.set_actor_label(name);a.static_mesh_component.set_static_mesh(cube);a.set_actor_scale3d(unreal.Vector(*scale));a.static_mesh_component.set_material(0,mats[material])
        if tag:a.tags=[unreal.Name(tag)]
        return a
    box('Grass terrain — friction test',(30000,0,-55),(900,160,1),'Grass','RideGrass')
    box('Main acceleration strip',(30000,0,-10),(800,12,.2),'Asphalt','RidePath')
    box('Wide turning / traction pad',(0,4000,-10),(100,60,.2),'Asphalt','RidePath')
    box('Obstacle test lane',(1000,-2000,-10),(80,12,.2),'Asphalt','RidePath')
    box('Solid wall — crash test',(2200,-2000,120),(.4,12,2.4),'Safety')
    box('Uphill ramp',(5500,0,251),(20,12,.4),'Concrete','RidePath',(15,0,0))
    box('Raised platform',(7500,0,260),(20,12,5.2),'Concrete','RidePath')
    box('Downhill ramp',(9500,0,251),(20,12,.4),'Concrete','RidePath',(-15,0,0))
    # Real Water plugin actor; separate solid shoreline barrier prevents riding on its surface.
    lake=actors.spawn_actor_from_class(unreal.WaterBodyLake,unreal.Vector(16000,0,-15))
    lake.set_actor_label('Water Body Lake — shoreline test');lake.tags=[unreal.Name('RideWater')]
    spline=lake.get_component_by_class(unreal.WaterSplineComponent)
    if spline:
        spline.clear_spline_points(False)
        for v in [(-2800,-2200,0),(-2800,2200,0),(2800,2200,0),(2800,-2200,0)]:spline.add_spline_point(unreal.Vector(*v),unreal.SplineCoordinateSpace.LOCAL,False)
        spline.set_closed_loop(True,True)
    for x in [13100,18900]:box('Solid lake edge', (x,0,60),(.3,46,1.2),'Safety','RideWater')
    for y in [-2300,2300]:box('Solid lake edge',(16000,y,60),(58,.3,1.2),'Safety','RideWater')
    for x in range(-3000,70000,500):
        if 4000<x<19000:continue
        box('Pavement lane stripe',(x,0,2),(1.8,.12,.015),'Stripe').static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    for y in [-7800,7800]:box('Course boundary',(30000,y,100),(900,.25,2),'Concrete')
    start=actors.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(0,0,110));start.set_actor_label('V2 test start')
    sun=actors.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,3000),unreal.Rotator(pitch=-28,yaw=-40,roll=0));sun.set_actor_label('Late afternoon sun');sun.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE);sun.light_component.set_editor_property('intensity',4)
    actors.spawn_actor_from_class(unreal.SkyAtmosphere,unreal.Vector())
    sky=actors.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,1000));sky.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
    sky.light_component.set_editor_property('real_time_capture',True)
    actors.spawn_actor_from_class(unreal.ExponentialHeightFog,unreal.Vector(0,0,-300))
    level.save_current_level()
    (root/'Scripts/v2-lab-built.json').write_text(json.dumps({'map':world.get_name(),'actors':len(actors.get_all_level_actors()),'water':str(lake),'build':'0.2.0-dev'},indent=2))
except Exception:
    (root/'Scripts/v2-lab-error.txt').write_text(traceback.format_exc())
    raise
