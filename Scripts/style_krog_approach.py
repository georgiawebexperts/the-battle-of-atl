"""Prepare the approach's own concrete material with its required mesh usage."""
import unreal
base='/Game/BattleForTheA/Environment/KrogApproach'
mat=unreal.load_asset(base+'/M_KrogApproachConcrete') or unreal.EditorAssetLibrary.duplicate_asset('/Game/PiedmontRide/Materials/M_ParkConcreteWorld',base+'/M_KrogApproachConcrete')
assert mat
mat.set_editor_property('used_with_nanite',True)
unreal.MaterialEditingLibrary.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
mesh=unreal.load_asset(base+'/SM_KrogApproach');assert mesh
settings=mesh.get_editor_property('nanite_settings');settings.position_precision=8
settings.generate_fallback=unreal.NaniteGenerateFallback.ENABLED;settings.fallback_target=unreal.NaniteFallbackTarget.PERCENT_TRIANGLES;settings.fallback_percent_triangles=1.0;settings.fallback_relative_error=0
mesh.set_editor_property('nanite_settings',settings)
mesh.set_material(0,mat);assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
