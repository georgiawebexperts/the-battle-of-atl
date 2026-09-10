import unreal,json,pathlib,traceback
r={}
def read(k,fn):
 try:r[k]=str(fn())
 except Exception:r[k]=traceback.format_exc()
ctx=unreal.load_asset('/Game/BeltLineGlide/IMC_BeltLine')
read('mappings',lambda:ctx.get_editor_property('mappings'))
for n in ['IA_Move','IA_Forward','IA_Brake','IA_Reset']:
 a=unreal.load_asset('/Game/BeltLineGlide/'+n)
 if a:read(n,lambda a=a:a.get_editor_property('value_type'))
bp=unreal.load_asset('/Game/BeltLineGlide/BP_BeltLineBike')
read('graph nodes',lambda:unreal.BlueprintEditorLibrary.find_graph(bp,'EventGraph').get_editor_property('nodes'))
for n in ['get_blueprint_graph','get_asset_graph']:
 read(n,lambda n=n:unreal.UnrealMCPBlueprintNodeCommands.handle_command(n,json.dumps({'blueprint_name':'/Game/BeltLineGlide/BP_BeltLineBike','asset_path':'/Game/BeltLineGlide/BP_BeltLineBike','graph_name':'EventGraph'})))
r['asset classes']=[n for n in dir(unreal) if n.startswith('UnrealMCP')]
pathlib.Path(unreal.Paths.project_dir(),'Scripts','input-inspection.json').write_text(json.dumps(r,indent=2))
