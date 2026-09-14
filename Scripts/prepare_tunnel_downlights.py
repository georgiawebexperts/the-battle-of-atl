"""Create review lighting or promote with guarded -InstallTunnelDownlights."""
import unreal,json,hashlib,shutil
from pathlib import Path
root=Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest()
install='-InstallTunnelDownlights' in unreal.SystemLibrary.get_command_line()
if install:
 assert before=='acef57c53bcd678037f99bd30b1a4b7457181f616065d954516a2888ee418e89'
 assert json.loads((root/'Tests/Results/2026-09-14-native-downlight.json').read_text())['visual_accepted']
 backup=root/'work/map-backups'/('PiedmontWorld-before-downlights-'+before[:12]+'.umap');shutil.copy2(main,backup);assert hashlib.sha256(backup.read_bytes()).hexdigest()==before
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
originals=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PointLight) and a.actor_has_tag('KrogUtilityLightReview')];assert len(originals)==6
rows=[]
for a in originals:
 old=a.get_component_by_class(unreal.PointLightComponent);spot=ea.spawn_actor_from_class(unreal.SpotLight,a.get_actor_location(),unreal.Rotator(pitch=-90,yaw=0,roll=0));spot.set_actor_label(a.get_actor_label()+' downlight');spot.tags=list(a.tags)+[unreal.Name('KrogDownlight')]
 c=spot.get_component_by_class(unreal.SpotLightComponent)
 for k in ['mobility','intensity_units','intensity','attenuation_radius','use_temperature','temperature','cast_shadows']:
  c.set_editor_property(k,old.get_editor_property(k))
 c.set_editor_property('inner_cone_angle',50.);c.set_editor_property('outer_cone_angle',75.)
 rows.append({'label':a.get_actor_label(),'intensity':old.get_editor_property('intensity'),'radius':old.get_editor_property('attenuation_radius'),'location':str(a.get_actor_location())});ea.destroy_actor(a)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/'+('PiedmontWorld' if install else 'PiedmontTunnelDownlightReview'))
assert install or hashlib.sha256(main.read_bytes()).hexdigest()==before
(root/'Tests/Results'/('2026-09-14-tunnel-downlight-install.json' if install else '2026-09-14-tunnel-downlight-candidate.json')).write_text(json.dumps({'lights':rows,'main_sha256':before,'main_changed':install,'after_sha256':hashlib.sha256(main.read_bytes()).hexdigest(),'visual_accepted':install,'scope':'Six downward broad spotlights replace utility points; intensity and radius preserved. main_changed identifies installation.'},indent=2)+'\n')
