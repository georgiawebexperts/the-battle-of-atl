"""Six modest utility fixtures in the isolated tunnel review."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for actor in ea.get_all_level_actors():
 if actor.actor_has_tag('KrogUtilityLightReview'):ea.destroy_actor(actor)
dest='/Game/BattleForTheA/Environment/KrogArt';name='M_KrogUtilityLens'
material=unreal.load_asset(dest+'/'+name)
if not material:material=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew())
lib=unreal.MaterialEditingLibrary;lib.delete_all_material_expressions(material)
color=lib.create_material_expression(material,unreal.MaterialExpressionConstant3Vector)
color.set_editor_property('constant',unreal.LinearColor(1.6,1.35,.95))
assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(material);assert unreal.EditorAssetLibrary.save_loaded_asset(material)
cube=unreal.load_asset('/Engine/BasicShapes/Cube');assert cube
points=json.loads((root/'SourceAssets/Terrain/KrogPortalCandidate/manifest.json').read_text())['roof_samples']
placed=[]
for index in [4,12,20,28,36,44]:
 x,y,z=points[index];y=-y
 light=ea.spawn_actor_from_class(unreal.PointLight,unreal.Vector(x,y,z+238))
 light.set_actor_label('Krog utility light '+str(index));light.tags=[unreal.Name('KrogUtilityLightReview')]
 component=light.get_component_by_class(unreal.PointLightComponent)
 component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
 component.set_editor_property('intensity_units',unreal.LightUnits.CANDELAS)
 component.set_editor_property('intensity',8.)
 component.set_editor_property('attenuation_radius',680.)
 component.set_editor_property('use_temperature',True);component.set_editor_property('temperature',3800.)
 component.set_editor_property('cast_shadows',True)
 fixture=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,z+262))
 fixture.set_actor_label('Krog utility lens '+str(index));fixture.tags=[unreal.Name('KrogUtilityLightReview')]
 fixture.set_actor_scale3d(unreal.Vector(.5,.16,.04));fixture.static_mesh_component.set_static_mesh(cube)
 fixture.static_mesh_component.set_material(0,material);fixture.static_mesh_component.set_collision_profile_name('NoCollision')
 placed.append({'xyz':[x,y,z+238],'candela':8,'radius_cm':680})
unreal.PiedmontWorldTools.finish_editor_asset_loading();assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-krog-utility-lights.json').write_text(json.dumps({'lights':placed,'main_map_changed':False,'scope':'Authored utility fixtures, not surveyed real lighting. Review-only, visual balance and performance pending.'},indent=2)+'\n')
