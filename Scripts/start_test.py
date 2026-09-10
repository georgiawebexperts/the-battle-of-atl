import unreal,time,json,pathlib,traceback
glide_test_start=time.monotonic()
glide_test_last=0
glide_test_rows=[]
def glide_record(delta):
    global glide_test_last,glide_test_handle
    now=time.monotonic()-glide_test_start
    if now-glide_test_last<1:return
    glide_test_last=now
    try:
        worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
        if worlds:
            w=worlds[0];p=unreal.GameplayStatics.get_player_pawn(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
            row={'t':round(now,1),'pawn':str(p),'location':str(p.get_actor_location()) if p else None,'rotation':str(p.get_actor_rotation()) if p else None}
            if pc:row['hud']=str(pc.get_hud())
            if p:
                row['speed']=p.get_velocity().length()
            row['traffic']=[{'name':a.get_name(),'loc':str(a.get_actor_location())} for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) if 'ScooterRider' in a.get_name() or 'Pedestrian' in a.get_name()]
            glide_test_rows.append(row)
        pathlib.Path(unreal.Paths.project_dir(),'Scripts','live-test.json').write_text(json.dumps(glide_test_rows,indent=2))
    except Exception:
        pathlib.Path(unreal.Paths.project_dir(),'Scripts','test-error.txt').write_text(traceback.format_exc())
    if now>120:unreal.unregister_slate_post_tick_callback(glide_test_handle)
glide_test_handle=unreal.register_slate_post_tick_callback(glide_record)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_begin_play()
