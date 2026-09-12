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


## Actual Chaos shape inspection — 2026-09-12 [codex-maclaptop]

Added PhysicsCore dependency for native read-locked inspection of actual Chaos shape geometry. Corrected ExecuteRead argument to GetPhysicsActor; final editor build passes. One premature runtime launch after failed link exited1 without loading gameplay; not accepted as a test. Successful rebuilt run logs20 capsule shapes with correct world dimensions (hips26cm wide, arms8cm, legs12cm) and all body WorldStatic responsesBlock. The post-correction geometry scale hypothesis is therefore unsupported. Skin clearance still fails2085/3273 vertices, minimum-18.298cm. Report now retains actual shape diagnostics. Next inspect simulation-shape flags/contact/constraint resolution, not blindly scale geometry again. No accepted crash or new playable package; Desktop048 unchanged; full game goal incomplete.


## Explicit physical pose display improves landing — 2026-09-12 [codex-maclaptop]

Added simulation-shape flags: all20 shapes are active simulation shapes. Isolated --free-joints runtime duplicate removes constraints (never saved/live); limbs separate as expected, yet displayed hips stay~2cm above support while actual pelvis body is342.490 and capsule bottom324.253 over support324.204. This exposes display/physics transform mismatch rather than merely insufficient collider radius. Free-joint test correctly fails and is diagnostic only.

Added --pose-mirror candidate: hides skeletal physics renderer, copies actual body-instance world transforms/scales to a separate poseable display each tick; unphysical bones follow reference local transforms from their parents. Recovery handoff accepts this display's actual landed pose. Same unchanged clearance threshold now sees63/3273 vertices below, min-2.811cm, versus2085/min-18.298 previously. Hip clearance15.179cm; connected limb span96.191. Handoff error0.0001cm, get-up final heights pass. Fall4 visually inspected: torso above road, limbs connected. This is meaningful improvement but still fails skin clearance and recovery pose RMS73.728cm suggests blend needs work. Next attribute remaining penetrating vertices to bones, improve collision fit locally, and address recovery motion. Native build passes, source mesh/main map unchanged, Desktop048 unchanged. Full goal incomplete.


## Vertex-fit collision candidate clears one landing — 2026-09-12 [codex-maclaptop]

Added dominant skin-bone attribution for penetrating vertices. V3 repeat exposed280below/min-7.216cm, mostly fingers/hands, with head/clothing/limb contributors; prior63-vertex result was one landing, not general clearance. Extended editor physics builder to fit imported mesh vertex clouds in bone space, attributing fingers to physical hand ancestors. V4 includes >20% influences and shrinks cylindrical spans; logged radii were too bulky at shared joints, rejected before runtime. V5 assigns each vertex to its dominant bone/nearest physical ancestor, fits axial extent and radial envelope with1.5cm margin.

Native build, asset generation and V5 mirrored-pose combined fall/recovery pass. One landing has0/3273 vertices below road, min clearance3.426cm; hip19.908cm, max limbspan101cm, finalspeed0.106. Transfer0.0001cm; get-up standing passes; bike settles. Fall4 inspected, no gross road burial. BUT shape total extents remain conservative/oversized along axes (head~59cm, hand~50cm, thigh~87cm), so no final contact realism acceptance; need tighter shape fitting (e.g. shorter fitted capsules or convex forms) without losing clearance. Recovery RMS72.653cm also needs improvement. No production integration, main/live mesh unchanged, Desktop048 unchanged. Full game incomplete.


## Tighter convex crash geometry — 2026-09-12 [codex-maclaptop]

V6 replaces overlong V5 capsules with20 convex hulls from dominant-weight vertex clouds in bone space, finger descendants assigned to hand,1.5cm radial expansion around cloud centroid. Constraints retained; live mesh assignment unchanged. Actual Chaos geometry confirms head25.686x31.907x28.881cm (previous capsule length58.657), hand21.261x28.133x8.993cm (previous~50.5length), closer to mesh surface. Native build and generation pass.

First mirrored-pose player+bike runtime clears all3273 skin vertices, minimum0.366cm above local per-vertex road. Hip12.278cm, limbspan97.529cm, finalspeed0.431. Pose handoff displacement0.0005cm; initial recovery RMS42.86cm; final standing gate and bike-settle gate pass. Fall4 inspected, body on/along fallen bike without gross ground burial. This remains one low-speed fixture: require high-speed and directional variation, video/contact/recovery evaluation and production ownership, input/camera/damage/remount integration. Desktop048 unchanged. Full goal incomplete.


## Faster fall probes, bounds and CCD — 2026-09-12 [codex-maclaptop]

Added configurable review launch speed150–2200cm/s and side±1, separate report names, following review camera for speed>600. Two1500cm/s side launches initially passed skin/settling/recovery. First inspected image lacked visible rider: poseable component bounds remained near launch point. Fixed display frame to follow physical pelvis minus90Z while preserving physical world bone transforms, then update bounds. New fall4 inspection confirmed rider visible near sidewalk; test found86finger vertices below raised sidewalk edge, min-15.918cm. Enabled CCD on every physical body before launch.

Latest1500/+1 CCD run clears3273vertices, min0.250cm, limbspan114.673cm, physicalspeed9.47cm/s at4.5s, handoff displacement0, recovery and bike settle pass. Launches are fixture velocities, NOT real collision-triggered riding. Final camera screenshot backs far away to contain separated body/bike and is obstructed by scenery; visual/camera acceptance false. -1 run predates bounds/CCD change, so not acceptance of latest build in both directions. Need player-focused camera with occlusion handling, additional actual impacts and gameplay ownership/damage/controls/remount. Native build passes, Desktop048 unchanged, goal incomplete.


## Rider-focused crash camera — 2026-09-12 [codex-maclaptop]

Added reusable UpdateBattleCrashCamera helper. Focuses pelvis+25cm, fixed(-320,-420,300) offset, smooth interpolation12/s; sphere12cm sweep onCamera channel shortens boom around blocking scenery, ignores rider owner/fallen bike. Clamps both target and smoothed position to avoid interpolation through walls. Replaces old zoom-to-fit rider+bike review framing.

Native build and latest1500/-1 mirrored V6+CCD fall pass:0/3273 below, min0.299cm, body settles, recovery and bike settle pass. Inspected fall4/recovery6: rider clearly visible at useful framing, standing at correct height. This validates that camera view in this fixture, not arbitrary wall occlusion or full motion/video. +1 last test used older camera. Next connect sequence to actual collision triggers and controllable on-foot recovery/remount, with damage/death interruptions and cleanup; wall-occlusion and camera transitions need runtime coverage. Desktop048 unchanged. Full game incomplete.


## Crash lifecycle cleanup — 2026-09-12 [codex-maclaptop]

Fallen-bike actor now remembers each source part's original visibility, restores it on EndPlay, and rejects repeated initialization. Recovery Reset destroys its owned pose component and clears clip, pose samples, timing and selection; Begin resets prior state and rejects invalid owners/physics. These are preparation for production collision integration, which remains unfinished.

Native Mac build and 1500cm/s left-side mirrored crash fixture pass: all3273 skin samples clear the road (minimum0.376cm), standing recovery and fallen-bike settle gates pass. New native lifecycle gate verifies duplicate initialization rejected, exact original part visibility restored after actor destruction, second recovery begins with fresh bone array/clock and old pose unregistered, interruption followed by repeated Reset leaves no registered pose, and null Begin fails safely. Does not yet prove production death/camera/controller cleanup, actual collision-triggered falls or remount. Desktop048 unchanged.


## Live collision-to-foot recovery and remount — 2026-09-12 [codex-maclaptop]

Added ABattlePlayerCrash owning hidden V6 skeletal simulation, physical-pose display, fallen bike, recovery blend and camera. Non-water Wipeout now captures incoming velocity and starts the physical sequence for a living mounted rider. Controls/movement stay disabled while down; the player target follows the body, damage remains applicable, and the countdown uses the existing off-foot multiplier. After settled motion and recovery animation, a clearance-checked ABattleRider is possessed with health/loadout retained and weapon holstered. The fallen bike remains where it landed; E searches nearby clear upright positions (within220cm of the fallen frame and240cm of the rider), restores original bike parts and possession, and removes temporary crash actors. Checkpoint death and EndPlay clean up the sequence. Water, taser, drone and knife-specific flows remain separate.

First real W-key car collision test reached on-foot control with health70/ammo10 and correct timer, but failed remount; camera sweep also collided with its own physical rider. Fixed own-body Camera response toIgnore and added nearby upright clearance search. Subsequent native main-world collision-to-E-remount passes (speed~1008cm/s, exactly1 wipeout, holstered foot state, ammo10/health70 retained, resumed grounded bike, crash actor removed). Latest fall-to-foot elapsed7.802s, countdown loss9.723s consistent with1.25 multiplier and frame tolerance. Mid-fall lethal-damage test passes checkpoint return, health100, no crash actor, mounted control. Both reports are under Tests/Results/2026-09-12-player-crash-live*.json. Native build passes.

Inspected actual fall/remount images. Camera frames rider/car/bike and remounted rider/bike is visible. HUD now says KNOCKED OFF / getting back up and shows ON FOOT1.25x instead of fixed recovery2.0/get-off prompt. These are gameplay integration checks, not final visual acceptance: get-up start alignment varied (latest77.709cm RMS; earlier142.955), so blending/contact motion still needs polishing and video inspection. Need more terrain/impact directions, blocked recovery handling, post-recovery death/repeat collision coverage, full other-hazard regressions, shipping package/performance and controller/camera transition acceptance. Desktop048 unchanged; full game goal remains incomplete.


## Repeated crashes and closer recovery alignment — 2026-09-12 [codex-maclaptop]

Recovery now evaluates three planar alignments per get-up clip: original head/hip direction, least-squares yaw across six landmarks anchored at hips, and centered rigid fit across hips/head/hands/feet. All candidates retain actual floor and standing clearance tests; the original remains eligible. Native logs compare original and selected RMS on the same landed pose. Latest repeat fixture improves76.273→27.134cm and57.224→50.234cm. On-foot-death fixture improves132.045→92.363cm. Exact landed-pose handoff retained; lower fit error is not full motion/animation acceptance.

Expanded actual keyboard collision audit to two cycles in one world, verifying restored health/ammo/control and zero remaining crash/fallen-bike actors after each remount, and to lethal damage after on-foot recovery with the fallen bike still present. Both latest reports pass. Initial repeat failed when ambient enemies reduced health70→50 (valid gameplay damage, invalid fixture assumption); fixture now uses DamageGrace100 until explicit lethal probe clears grace. A subsequent remount test pressed E at short XY distance while rider was still vertically out of range; approach now uses actual3D distance, matching interaction semantics. No production range relaxation. Latest build passes; Desktop048 unchanged.

Still pending: transition video/contact polish (one selected RMS remains92cm), occupied/blocked get-up clearance, more terrain/directions/other hazards, packaged Mac validation and full-game visual/performance acceptance. Native source is committed groundwork for the next Mac playtest, not game completion.


## Mac playtest049 installed — 2026-09-12 [codex-maclaptop]

Built/cooked/staged049 successfully (AutomationTool exit0,221s), including V6 player physics/get-up clips and installed mixed canopy/hornbeam shaders. New test runner supports packaged executable and writes captures inside staged app's macOS sandbox container before copying results to project work. Initial packaged gameplay checks passed but screenshot files were absent due wrong writable directory; corrected capture location and required expected image count.

Packaged two-cycle collision/recovery/E remount passes with six screenshots and zero leftover crash/fallen-bike actors. Post-get-up death cleanup passes. Packaged health/cardinal hunt/death/recollection/ordered checkpoint test passes. Three packaged as-installed canopy views capture at1920x1080; median frame times12.117/15.722/14.296ms, p9513.277/16.077/14.905ms. Inspected lake/canopy and crash/remount images; this proves installed assets/rendered frames and narrow state checks, not final animation realism, art quality or sustained gameplay FPS.

Preserved old app as /Volumes/Adam Assets/Unreal/Builds/BattleForTheA/Mac/TheBattleOfATL-build048.app. Finalized branded0.49.0 app, strict local signature verification and desktop symlink/version passed. Desktop /Users/elliottinspace/Desktop/The Battle of ATL.app points to external-drive /Volumes/Adam Assets/Unreal/Builds/BattleForTheA/Mac/TheBattleOfATL.app. Re-ran health/checkpoint startup on the installed signed executable: pass, both deaths and two checkpoints.

Full game remains incomplete: blocked get-up/more impacts, animation video/contact polish, other hazard integration, broader route/art/characters/combat/audio/performance and original full-scope acceptance. No Aura dependency added.


## Drone and taser physical knockdowns — 2026-09-12 [codex-maclaptop]

Mounted drone and taser hits now call the shared non-water Wipeout/physical crash flow rather than safe dismount. Preserved drone15 damage, two-second minimum stun, taser three-second minimum stun, -10s penalty,12s taser grace and repeat-hit guards. Actual fall/settle/get-up adds recovery time before on-foot control; these stun values are not a promise of total recovery duration. On-foot taser behavior remains its existing stun. Knife-specific chase and water flows remain separate.

Native rendered integration fixtures use actual warned police firing and drone swept contact. Both pass physical fall, disabled mounted actions/repeat hits, expected health, timer multiplier, holstered on-foot recovery and E remount. Taser health100/ammo10 retained; drone85/ammo10 retained in fixture with regeneration held. New hazard regression runner explicitly skips damage-free tutorial; first drone regression timed out with health100 because tutorial protection was still active. Corrected fixture, not gameplay protections.

Broader native headless drone audit passes warning/contact/damage, physical recovery/remount, wall obstruction and shooting/destruction. Updated police audit waits for physical recovery after initial person impact, explicitly draws its gun for the existing police-shot test, and waits/approaches before remount. Full police audit passes:3 distinct people alert police, navigation pursuit683.8cm, real cop-shot -60s, artifact escalation, wall-blocked taser, windup, local physical knockoff, grace, and death during another stun with checkpoint return. Build passes. Reports: player-crash-live-{taser,drone}.json and native-{drone,trouble}-physical-regression.json dated2026-09-12.

These are post049 source changes; Desktop049 remains installed and does not yet include physical drone/taser falls. Need packaged verification, animation/contact/video polish, moving target/hill hazard cases and broader original game scope. Full goal incomplete.


## Lamps follow the fallen bicycle — 2026-09-12 [codex-maclaptop]

Fixed automatic head/rear lamps remaining attached to the rider capsule while the separate bike frame fell. ABattleFallenBike temporarily reparents the two real lamp components to its physical Frame with world transforms preserved, recording original parent/socket/local transform. EndPlay restores both attachments/transforms, including remount and death cleanup. No duplicate lights are spawned. Automatic dark-zone detection uses the actual headlamp location rather than the rider actor location, so lamps stay responsive to where the bicycle is.

Native build passes. Rendered collision/get-up/light fixture places a small dark zone around the fallen headlamp while the bike actor origin remains outside: both lamps attached to Frame, lamp-only dark probe and visible automatic lights pass. After E remount, exact original parent/local transforms and delayed daylight switch-off pass, with no crash actors left. Mid-fall death fixture also passes restored lamp parents/local transforms and checkpoint control. Reports2026-09-12-player-crash-live-lights.json and player-crash-lights-death.json. This is attachment/state/cleanup evidence, not full nighttime visual acceptance.

Post049 source only; desktop049 unchanged. Pending packaged update with physical drone/taser falls, blocked recovery and animation/video/night-scene polish, plus the wider original game scope. Goal incomplete.
