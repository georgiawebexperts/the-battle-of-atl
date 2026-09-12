# Ellison crash presentation


## Ellison crash physics preparation — 2026-09-12 [codex-maclaptop]

Asset inspection confirms current ridden Casual mesh uses CharacterArmature/Root/Hips skeleton with no PhysicsAsset. Existing same-skeleton clips include death, roll and hit reactions but no standing get-up; City recovery clips use a different skeleton and cannot be directly substituted. Current two-second bike/rider tilt remains live.

Added noninteractive editor helper using PhysicsUtilities CreateFromSkeletalMesh with bSetToMesh=false, producing a separate candidate asset. First MinBoneSize15 fit yielded only Hips/Chest (2bodies/1constraint) and is rejected for full-body fall. MinBoneSize0.1 candidateV2 yields20 bodies/19constraints including pelvis,torso,head,arms,hands,legs,feet. Both candidates retained; onlyV2 is suitable for the next simulation review. Mesh assignment and main/gameplay remain unchanged. Native editor build and generation succeed.

Next simulate candidateV2 from current mounted pose in an isolated native test; verify scale/stability/floor clearance before integrating fall state. Recovery animation retargeting, bike separation, camera, death/knife semantics and realistic crash acceptance remain pending. Desktop048 unchanged; full game not complete.


## Mounted fall runtime candidate — 2026-09-12 [codex-maclaptop]

Added opt-in BattlePlayerCrashReview and render_player_crash.py: copy actual mounted bone transforms to an independent simulated skeletal component, launch at (300,90,60) cm/s, capture four frames and measure settling. No production trigger or mesh assignment changes. V2 failed: generated identical radius/length0.505 capsules on every bone, with runtime scale100; two runs showed buried or floating body and failed clearance/drop. Preserved first and scale diagnostic reports.

V3 fits explicit anatomical radii (4–13 world cm) in bone space and segment-aligned capsules, retaining generated constraints. Editor build and asset generation pass. First native V3 test: hip drop77.155cm, limb span98.354cm, hip above traced floor33.538cm, final velocity0.127cm/s. Physics predicate passed; frames3/4 inspected, body connected and prone beside upright bike. This is not final visual acceptance. Render report independently records capture vs physics results. Need close-up contact/skin clearance, high-speed impact tests, bike fall, compatible get-up animation, camera and production damage/recovery integration. Pawn collisions still ignored in test. Desktop048 unchanged. Full goal remains incomplete.


## Ellison recovery retarget and scale correction — 2026-09-12 [codex-maclaptop]

Created explicit City-to-Casual IK chains and exported four licensed City recovery clips (F/B/L/R, 5 seconds each) to BattleRetarget/Ellison/RecoveryCandidate. First actual native playback caught pelvis translation in incorrect units: head about9006cm and feet8851cm above road. Raw candidate preserved. Bone-pose inspection showed pelvis end Y-89.375 despite scaled armature; child local translations were already correct.

Baked separate RecoveryScaled/GetUp_F/B/L/R copies at30fps with pelvis translation divided100 and armature scale100. Original clips/skeleton unchanged. Native build and 16-frame playback capture pass. All four finish head157.79–157.89cm, feet2.79–3.26cm above surface; standing-height gate added independently from capture gate. Inspected mid-recovery for all four and F standing: connected mesh, correct overall scale. This does not prove full animation continuity, initial contact alignment or transition from ragdoll. Need blend/alignment from settled physical pose, falling bike, player camera, collision/damage/recovery integration. Desktop048 unchanged. Full game goal incomplete.


## Physical fall to get-up handoff — 2026-09-12 [codex-maclaptop]

Added FBattlePlayerRecoveryBlend and opt-in --recovery path in render_player_crash.py. Samples all four scaled clips, aligns head/pelvis heading and traces a floor for each, selects minimum six-bone position error. Transfers every settled physical bone into a separate poseable mesh without moving the landed pose, then blends local transforms over0.35s before playing the5s get-up. Original live bike behavior remains unchanged.

Native editor build and combined fall/recovery capture pass. Physical hip drop71.312cm, final physical speed1.106cm/s. Selected right-side clip; initial candidate pose RMS46.089cm (remaining blend-quality concern), exact handoff maximum bone displacement0.0000cm. Finishes head157.886cm, feet3.256/2.875cm above traced floor. Inspected recovery frames1/3/6, showing connected landed/start/standing poses. This is narrow fixture evidence, not completed realistic crash. Need video/contact evaluation, all impact directions, standing overlap safety, actual bike fall, production ownership/interruption/death handling, player camera and transition to controllable walking/remount. Desktop048 unchanged. Full game incomplete.


## Fallen bike and recovery clearance candidate — 2026-09-12 [codex-maclaptop]

Added ABattleFallenBike: copies16 direct Visual static-mesh children, excludes pistol and nested editor camera visualization, uses welded thin wheel/frame/bar/saddle box hulls,22kg mass, CCD, linear/angular damping. Opt-in --bike --recovery runtime review starts an independently simulated bike with Ellison's physical fall. Production triggers not changed.

First capture exposed copied editor camera mesh and floor query hitting fallen bike. Fixed direct-child filter; recovery floor trace ignores fallen bikes, then tests standing capsule against static/dynamic/physics/Pawn objects and selects nearest viable candidate offset. Second native build and capture pass numerical gates: bike upright-dot0.201, speed0,16parts; player handoff displacement0; final head157.886cm and feet3.256/2.875 above road. Player landing hip2.138cm despite physics predicate passing. Inspected fall4/recovery3/recovery6: bike side-down, editor mesh removed, final standing beside bike, BUT player torso clips into road/bike during landing and candidate alignment RMS62.121cm may produce an unnatural blend. Explicit visual_acceptance=false. Must improve physical skin clearance and transition before production; these broad numerical tolerances are insufficient for realism. Also pending actual riding-speed impacts, camera/input, fallen-bike interaction/remount, damage/death/cancellation, all-direction tests. Desktop048 unchanged. Full game unfinished.


## Crash skin diagnostics and Aura clarification — 2026-09-12 [codex-maclaptop]

Added native per-body transform/AABB and CPU skin-position diagnostics to crash review. Initial GetCPUSkinnedVertices diagnostic was unsuitable because it refreshes animated bones; replaced with GetSkinnedVertexPosition against current transforms. ComponentTransformIsKinematic prevents simulated root updating the component frame and reduced a uniform26.7cm displayed/physical bone discrepancy to~0.35cm. Floor query now ignores fallen bike. Latest floor StaticMeshActor_123/StaticMeshComponent0 is QueryAndPhysics and blocks PhysicsBody, yet skin extends17.67cm below hip's flat floor reference (2231/3273 vertices); stricter skin clearance gate now fails the test despite old settling/recovery predicates passing. Candidate still clips and is not live. Next inspect road simple vs complex contact and sample floor per vertex; no blind body-padding acceptance. Native build passes. Desktop048 unchanged.

Elliott asked whether Aura was being used. Answered candidly: current crash work uses Unreal C++/assets directly, not Aura; this is not evidence of Aura's usefulness, which remains unestablished. User did not cancel ongoing game work.


## Per-vertex road clearance and tool preference — 2026-09-12 [codex-maclaptop]

Elliott explicitly says continue with whatever is most effective and forget Aura if not needed. Direct Unreal development remains authorized; no need to spend time on Aura evaluation.

Crash review now compares each CPU-skinned vertex against its own downward complex floor trace, ignoring rider and fallen bike, rather than assuming a flat floor. Still fails1976/3273 vertices, minimum-18.339cm. Simple and complex collision query both report road324.092 (SM_TenthStreet_Road, UseComplexAsSimple); actual physics body bounds extend to~304–307 while pelvis326.65. Native build passes; stricter visual clearance test correctly exits1. Road query agreement does not prove physics contacts; next inspect triangle sidedness and collision filtering. No game crash fix accepted or packaged. Desktop048 unchanged; goal incomplete.


## Road-sidedness and solid-support isolation — 2026-09-12 [codex-maclaptop]

Created a separate double-sided copy of graded10th road via create_crash_road_candidate.py; source double-sided=false, copy=true. Runtime-only --double-road swaps hit road mesh. It still fails skin clearance (2321vertices below, min-21.398cm); do not install this candidate as a fix. Added --solid-support with a simple static blocking box at the road surface; same fall still fails (2725below, min-21.01cm), pelvis remains ~2.14cm above support. This narrows the problem toward the character physics geometry/contact setup, not exclusively triangle sidedness. Need inspect actual Chaos shape dimensions/filtering and imported skeleton scale; authored capsule radii and broad body bounds are insufficient proof. Native builds pass; both stricter clearance tests correctly fail. Main map/live meshes unchanged; Desktop048 unchanged; full goal incomplete.
