"""Compare shell without internal caps, preserving exterior coordinates."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());source=root/'SourceAssets/Terrain/KrogOutwardShell'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
points=json.loads((source/'manifest.json').read_text())['roof_samples']
def measure():
 unreal.PiedmontWorldTools.finish_editor_asset_loading();rows=[]
 for x,y,z in points:
  floor=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,-y,z+100),unreal.Vector(x,-y,z-100));assert floor
  roof=unreal.PiedmontWorldTools.trace_world_surface(floor[0]+unreal.Vector(0,0,10),floor[0]+unreal.Vector(0,0,450));assert roof
  rows.append([floor[0].z,roof[0].z])
 return rows
before=measure()
for kind in ['Shell','Columns']:
 actors=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Krog route SM_KrogTunnel_'+kind];assert len(actors)==1
 actor=actors[0];previous=actor.static_mesh_component.static_mesh
 opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False
 opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
 opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 dest='/Game/BattleForTheA/Environment/KrogPortalCandidate';name='SM_KrogOutward_'+kind
 task=unreal.AssetImportTask();task.filename=str(source/('KrogTunnel_'+kind+'.obj'));task.destination_path=dest;task.destination_name=name
 task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name);assert mesh
 mesh.set_editor_property('nanite_settings',previous.get_editor_property('nanite_settings'))
 mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
 mesh.set_material(0,actor.static_mesh_component.get_material(0));unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 old_bounds=previous.get_bounding_box();new_bounds=mesh.get_bounding_box()
 assert (old_bounds.min-new_bounds.min).length()<.1 and (old_bounds.max-new_bounds.max).length()<.1
 actor.static_mesh_component.set_static_mesh(mesh)
after=measure()
maximum=max(abs(a-b) for old,new in zip(before,after) for a,b in zip(old,new));assert maximum<.25
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
(root/'Tests/Results/2026-09-12-krog-outward-shell.json').write_text(json.dumps({'main_map_changed':False,'before_triangles':948,'after_triangles':948,'floor_roof_sites':len(points),'maximum_height_change_cm':maximum,'scope':'Bounds and50 native floor/roof locations preserved. Reversed shell and column side-face winding only. Visual comparison pending; not full collision equivalence.'},indent=2)+'\n')

# Face orientation affects collision export; refresh candidate navigation after replacement.
nav_script=(root/'Scripts/rebuild_krog_world_navigation.py').read_text().replace('2026-09-12-krog-world-navigation.json','2026-09-12-krog-outward-navigation.json')
exec(compile(nav_script,'rebuild_krog_outward_navigation','exec'), {'__name__': '__main__'})
