# Ellison crash presentation


## Ellison crash physics preparation — 2026-09-12 [codex-maclaptop]

Asset inspection confirms current ridden Casual mesh uses CharacterArmature/Root/Hips skeleton with no PhysicsAsset. Existing same-skeleton clips include death, roll and hit reactions but no standing get-up; City recovery clips use a different skeleton and cannot be directly substituted. Current two-second bike/rider tilt remains live.

Added noninteractive editor helper using PhysicsUtilities CreateFromSkeletalMesh with bSetToMesh=false, producing a separate candidate asset. First MinBoneSize15 fit yielded only Hips/Chest (2bodies/1constraint) and is rejected for full-body fall. MinBoneSize0.1 candidateV2 yields20 bodies/19constraints including pelvis,torso,head,arms,hands,legs,feet. Both candidates retained; onlyV2 is suitable for the next simulation review. Mesh assignment and main/gameplay remain unchanged. Native editor build and generation succeed.

Next simulate candidateV2 from current mounted pose in an isolated native test; verify scale/stability/floor clearance before integrating fall state. Recovery animation retargeting, bike separation, camera, death/knife semantics and realistic crash acceptance remain pending. Desktop048 unchanged; full game not complete.
