import unreal,json,pathlib
m=unreal.load_asset('/Game/BattleForTheA/Police/Swat');s=m.skeleton
rows=[]
# Editor bone enumeration through a transient poseable component.
c=unreal.new_object(unreal.PoseableMeshComponent);c.set_skinned_asset_and_update(m)
for i in range(c.get_num_bones()):rows.append(str(c.get_bone_name(i)))
(pathlib.Path(unreal.Paths.project_dir())/'work/police-bones.json').write_text(json.dumps(rows,indent=2))
unreal.SystemLibrary.quit_editor()
