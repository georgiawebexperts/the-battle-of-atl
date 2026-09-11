import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
(root/'work/police-material-inspection.json').unlink(missing_ok=True)
mesh=unreal.load_asset('/Game/BattleForTheA/Police/Swat')
rows=[]
for slot in mesh.get_editor_property('materials'):
 m=slot.material_interface;rows.append({'slot':str(slot.material_slot_name),'material':m.get_path_name() if m else None,'skeletal_usage':m.get_editor_property('used_with_skeletal_mesh') if isinstance(m,unreal.Material) else 'inherited' if m else None})
cls=unreal.load_class(None,'/Script/AuraPlayground.BattlePolice')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);actor=ea.spawn_actor_from_class(cls,unreal.Vector(0,0,500));body=actor.get_editor_property('body')
result={'saved_mesh':rows,'actor_mesh':body.get_skinned_asset().get_path_name(),'actor_materials':[body.get_material(i).get_path_name() if body.get_material(i) else None for i in range(body.get_num_materials())]}
(root/'work/police-material-inspection.json').write_text(json.dumps(result,indent=2)+'\n');ea.destroy_actor(actor)
unreal.SystemLibrary.quit_editor()
