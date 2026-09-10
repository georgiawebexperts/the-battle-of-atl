import unreal, pathlib, json, traceback
r=[]
for w in unreal.EditorLevelLibrary.get_pie_worlds(False):
    p=unreal.GameplayStatics.get_player_pawn(w,0)
    pc=unreal.GameplayStatics.get_player_controller(w,0)
    d={'world':w.get_name(),'pawn':p.get_name() if p else None,'location':str(p.get_actor_location()) if p else None,'rotation':str(p.get_actor_rotation()) if p else None,'controller':pc.get_name() if pc else None}
    if pc:
        d['hud']=str(pc.get_hud())
        d['camera_location']=str(pc.player_camera_manager.get_camera_location())
        d['camera_rotation']=str(pc.player_camera_manager.get_camera_rotation())
    r.append(d)
pathlib.Path(unreal.Paths.project_dir(),'Scripts','play-state.json').write_text(json.dumps(r,indent=2))
print('PLAY_STATE '+json.dumps(r))
