import unreal,pathlib,json,traceback
r={};ctx=unreal.load_asset('/Game/BeltLineGlide/IMC_BeltLine')
move=unreal.load_asset('/Game/BeltLineGlide/IA_Move');brake=unreal.load_asset('/Game/BeltLineGlide/IA_Brake');reset=unreal.load_asset('/Game/BeltLineGlide/IA_Reset')
try:
    maps=[]
    for key,action,negative,swizzle in [('W',move,False,False),('S',move,True,False),('D',move,False,True),('A',move,True,True),('Up',move,False,False),('Down',move,True,False),('Right',move,False,True),('Left',move,True,True),('SpaceBar',brake,False,False),('R',reset,False,False)]:
        mods=[]
        if negative:mods.append(unreal.new_object(unreal.InputModifierNegate,outer=ctx))
        if swizzle:
            mod=unreal.new_object(unreal.InputModifierSwizzleAxis,outer=ctx);mod.set_editor_property('order',unreal.InputAxisSwizzle.YXZ);mods.append(mod)
        k=unreal.Key();k.set_editor_property('key_name',key)
        mapping=unreal.EnhancedActionKeyMapping();mapping.set_editor_property('action',action);mapping.set_editor_property('key',k);mapping.set_editor_property('modifiers',mods);maps.append(mapping)
    ctx.set_editor_property('mappings',maps);unreal.EditorAssetLibrary.save_loaded_asset(ctx);r['mappings']=str(ctx.get_editor_property('mappings'))
except Exception:r['error']=traceback.format_exc()
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if a.get_actor_label().startswith(('ScooterRider_','Pedestrian_')):
        old=a.get_actor_rotation();heading=old.pitch if abs(old.pitch)>1 else old.yaw
        a.set_actor_rotation(unreal.Rotator(pitch=0,yaw=heading,roll=0),False)
        loc=a.get_actor_location();a.set_actor_location(unreal.Vector(loc.x,loc.y,90),False,True)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
pathlib.Path(unreal.Paths.project_dir(),'Scripts','input-repair-result.json').write_text(json.dumps(r,indent=2))
