import unreal,pathlib,json
r={}
for cls in [unreal.UnrealMCPBlueprintCommands,unreal.UnrealMCPBlueprintNodeCommands]:
    r[cls.__name__]={n:str(getattr(cls,n).__doc__) for n in dir(cls) if ('command' in n or 'graph' in n or 'node' in n or 'property' in n) and not n.startswith('_')}
bp=unreal.EditorAssetLibrary.load_blueprint_class('/Game/BeltLineGlide/BP_BeltLineBike')
cdo=unreal.get_default_object(bp)
r['cdo_components']=[c.get_name() for c in cdo.get_components_by_class(unreal.ActorComponent)]
r['character_movement']=str(cdo.get_editor_property('character_movement'))
pathlib.Path(unreal.Paths.project_dir(),'Scripts','blueprint-api.json').write_text(json.dumps(r,indent=2))
