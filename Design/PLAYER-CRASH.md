# Ellison crash presentation


## Ellison crash physics preparation — 2026-09-12 [codex-maclaptop]

Asset inspection confirms current ridden Casual mesh uses CharacterArmature/Root/Hips skeleton with no PhysicsAsset. Existing same-skeleton clips include death, roll and hit reactions but no standing get-up; City recovery clips use a different skeleton and cannot be directly substituted. Current two-second bike/rider tilt remains live.

Added noninteractive editor helper using PhysicsUtilities CreateFromSkeletalMesh with bSetToMesh=false, producing a separate candidate asset. First MinBoneSize15 fit yielded only Hips/Chest (2bodies/1constraint) and is rejected for full-body fall. MinBoneSize0.1 candidateV2 yields20 bodies/19constraints including pelvis,torso,head,arms,hands,legs,feet. Both candidates retained; onlyV2 is suitable for the next simulation review. Mesh assignment and main/gameplay remain unchanged. Native editor build and generation succeed.

Next simulate candidateV2 from current mounted pose in an isolated native test; verify scale/stability/floor clearance before integrating fall state. Recovery animation retargeting, bike separation, camera, death/knife semantics and realistic crash acceptance remain pending. Desktop048 unchanged; full game not complete.


## Mounted fall runtime candidate — 2026-09-12 [codex-maclaptop]

Added opt-in BattlePlayerCrashReview and render_player_crash.py: copy actual mounted bone transforms to an independent simulated skeletal component, launch at (300,90,60) cm/s, capture four frames and measure settling. No production trigger or mesh assignment changes. V2 failed: generated identical radius/length0.505 capsules on every bone, with runtime scale100; two runs showed buried or floating body and failed clearance/drop. Preserved first and scale diagnostic reports.

V3 fits explicit anatomical radii (4–13 world cm) in bone space and segment-aligned capsules, retaining generated constraints. Editor build and asset generation pass. First native V3 test: hip drop77.155cm, limb span98.354cm, hip above traced floor33.538cm, final velocity0.127cm/s. Physics predicate passed; frames3/4 inspected, body connected and prone beside upright bike. This is not final visual acceptance. Render report independently records capture vs physics results. Need close-up contact/skin clearance, high-speed impact tests, bike fall, compatible get-up animation, camera and production damage/recovery integration. Pawn collisions still ignored in test. Desktop048 unchanged. Full goal remains incomplete.
