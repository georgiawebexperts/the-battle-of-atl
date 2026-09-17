import unreal,json,pathlib
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
h=unreal.SystemLibrary.line_trace_single(w,unreal.Vector(0,0,10000),unreal.Vector(0,0,-10000),unreal.TraceTypeQuery.ECC_VISIBILITY,False,[],unreal.DrawDebugTrace.NONE)
r={'hit':str(h),'attributes':dir(h),'doc':h.__doc__}
(pathlib.Path(unreal.Paths.project_dir())/'Scripts/hit-result-api.json').write_text(json.dumps(r,indent=2))
