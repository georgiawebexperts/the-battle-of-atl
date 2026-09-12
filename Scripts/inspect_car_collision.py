import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());mesh=unreal.load_asset('/Game/Vehicles/SportsCar/SM_SportsCar');s=mesh.get_editor_property('body_setup').get_editor_property('agg_geom')
r={'simple_shapes':sum(len(s.get_editor_property(n)) for n in ['box_elems','sphere_elems','sphyl_elems']),'convex_hulls':len(s.get_editor_property('convex_elems')),'trace_flag':str(mesh.get_editor_property('body_setup').get_editor_property('collision_trace_flag'))}
(root/'Tests/Results/2026-09-12-car-collision-geometry.json').write_text(json.dumps(r,indent=2)+'\n')
