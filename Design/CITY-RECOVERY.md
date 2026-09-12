# City crowd recovery assets

2026-09-11 [codex-maclaptop]

Epic Game Animation Sample includes four five-second standing recovery sequences, M_ragdoll_getup_stand_F/B/L/R, on its UEFN mannequin. Scripts/retarget_city_recovery.py exports copies for the male and female City Sample normal-weight bodies using Epic's RTG_UEFN_to_Metahuman_nrw and Unreal 5.8 RunBatchRetarget. Output is /Game/BattleRetarget/City/{Male,Female}; both use the game's City metahuman_base_skel. Source animations and rigs are unchanged. Output overwrite is refused.

Removed sample-specific animation notify tracks from the exported copies. The game's pose sampler does not execute their sample-project Foley notifies; carrying those Blueprint dependencies would not implement contact audio. Recovery audio remains required integration work.

Eight exports migrated into the game with recursive dependency and SHA256 verification. SourceAssets/Manifests/epic-crowd-selection.json includes them; the migration manifest records all related packages. Retargeted asset paths are outside CitySampleCrowd: add BattleRetarget to explicit cooking before runtime string-path loading.

Scripts/render_city_recovery_review.py samples four positions per clip on fully dressed bodies in a transient native Metal scene. Use -AllowCommandletRendering. Verify all images, especially floor pose, hand/foot contacts, clothing and final stance. This preview does not prove ragdoll simulation, crash detection, pose matching, capsule repositioning, get-up transitions, navigation or gameplay timing.

Required runtime work: simulate a skeletal physics body from the current pose on hard contact; keep clothes and hair following it; choose front/back/side recovery from settled orientation; check a clear standing capsule; align recovery root and blend from the simulated pose; restore collision and AI after recovery. Prevent repeated triggers during recovery and preserve death semantics. High-speed impacts still use the rejected legacy response until this is implemented and tested.

2026-09-11 [codex-maclaptop]: Eight clips exported successfully and 32 explicit pose images rendered. Sampled poses across all directions/both bodies show coherent clothing/body positions from floor to standing. Camera widened after initial foot crop. Hand/sole contact remains approximate; no continuous sequence or runtime transition acceptance. Editor Python completed, but render process exit1 records an HttpListener port30010 bind conflict. Test report distinguishes actual image evidence from process success. No gameplay integration or desktop replacement yet.

API reference: https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/IKRetargetBatchOperation
