# Zombie encounters

2026-09-11 [codex-maclaptop]

BattleEnemyDirector uses the actual difficulty row for zombie population,
shambler/sprinter speed, sprinter fraction, attack damage, warning time and
replacement interval. It selects collision-clear, navigation-reachable positions
near the player, rejects the lake, and retires distant live actors beyond7500cm.
Spawning waits until the countdown ends. Existing opt-in terrain/quest/health
fixtures freeze this new subsystem to retain their intended scope; separate
zombie audits exercise it. These fixture flags do not disable enemies in Shipping.

Zombies use AAIController and pathfinding movement. Attacks have a visible windup
and require proximity, height agreement and line of sight again when they land.
A moving player can evade the attack. A hit damages shared rider health and a
successful grab wipes out a mounted bike. Pistol body hits deal34; head-region
hits receive a3x multiplier. Death removes collision, stops AI, awards the normal
pistol kill/nitro reward once, and removes the corpse after5seconds. The last1.5
seconds animate a masked dissolve. The rigged CC0 Casual mesh has a green-gray
material and emissive eyes, with emergence, twitching and hit reactions.

Hard's TunnelWaveSize drives waves restricted to the tunnel darkness volumes.
A45second wave cooldown permits additional waves while the player remains in the
tunnel. The population cap allows the configured regular population plus one
wave, with additional queued wave spawns waiting for capacity. Leaving the tunnel
cancels unspawned wave members so normal spawning resumes. Normal spawns are
not guaranteed to be hidden by scenery; authored bushes/dumpsters/wall entrances
are still required world/encounter presentation work.

SourceAssets/Data/ZombieLines.txt contains30original lines, used at spawn and
periodically during nearby pursuit, with HUD subtitles. Sprinters have an
additional charge warning. Scripts/generate_zombie_sources.py regenerates the
header and original procedural growl. The growl is not a recording of those
lines:30spoken performances, a real sprinter shriek and headshot crunch remain
pending audio work. Full ragged clothing/head-pop presentation and rendered
appearance are not accepted by the current headless tests.

Scripts/prepare_zombie_assets.py creates skeletal-mesh-compatible masked skin,
emissive eye material and imports the growl. It requires the new compiled
BattleDifficultyRow before Scripts/install_difficulty.py imports updated CSV
values. SourceAssets/Data/Difficulty.csv remains the tuning source.

BattleZombieAudit uses a real director spawn and checks navigation travel,
telegraph, shared health damage/wipeout, actual FPS pistol body/head hits, kill
rewards and timed corpse cleanup. Its later firing fixtures freeze target pursuit
and reposition it for precise damage checks. BattleZombiePopulationAudit keeps
normal spawning active but freezes pursuit so it can count actual spawned Easy,
Medium and Hard populations, validate their applied tuning, then trigger and
check a covered Hard tunnel wave, a timed repeat wave, and normal spawning after
leaving a partially spawned wave. Neither establishes visual crowd density,
real-world game balance, audio quality, human keyboard input or frame rate.


## Geographic restriction — 2026-09-12 [codex-maclaptop]

New user requirement, not implemented: Farmers Market Vendor zombies appear only inside Piedmont Park, including pursuit boundaries. Homeless-themed and punk-rocker variants can appear anywhere in the playable world. Market beside Billy’s at 12th Street motivates the vendor variant. See WORLD-EXPANSION.md, Farmers market at 12th Street, for reference photos and tutorial entrance blocking.


## Vendor park boundary implementation — 2026-09-12 [codex-maclaptop]

Source now uses the retained OSM Piedmont Park polygon (way 208357832), converted through the terrain georeference to Unreal ESU at the existing 1:3 geographic scale. Scripts/generate_park_region.py regenerates BattleParkRegion.h and SourceAssets/Terrain/park-region.json; use Tools/terrain-venv/bin/python. A 35 cm inward margin keeps the vendor capsule inside. Exterior vendor choices become punk before mesh setup. Vendor pursuit requests reject partial or boundary-crossing navigation paths; attacks cancel when the player leaves the park. Character movement updates also reject crossing segments, restoring the last valid position. Punk behavior is unrestricted; other actor classes are unchanged.

Native build passed. Scripts/test_vendor_region.py passed exterior selection, interior vendor selection, cancellation of a pending attack on an outside player, and containment. Identical forced character movement stayed inside for the vendor and crossed outside for the punk. Collision/ground effects are disabled only in that comparative fixture. No rendered appearance, exhaustive boundary traversal, long in-park pursuit or packaged verification is claimed. Paths are sampled at 20 cm with a 35 cm margin. Full market geometry, tutorial closure and surrounding dressing remain pending. Desktop050 does not contain this source change.


## In-park pursuit verification — 2026-09-12 [codex-maclaptop]

Expanded the native vendor fixture beyond containment: spawn an actual vendor on a retained OSM park path, position Ellison about12m down that path, and allow normal AI/navigation ticks. Requires more than150cm approach within4seconds. Move Ellison outside: vendor must stop within5cm for2seconds with no attack telegraph. Return Ellison inside: vendor must move at least100cm again within3seconds. All stages and the existing vendor/punk comparative movement checks pass; native build succeeds. Fixture uses a stationary, damage-protected bike and one path; it does not establish crowd behavior, all routes, animated appearance or packaged acceptance. Desktop050 remains unchanged.


## Installed locomotion and motion clarity — 2026-09-12 [codex-maclaptop]

Captured installed052 punk/vendor walk, walk-step, run and idle. Fixture metrics pass: ~1440cm movement,~103degree knee motion, idle ankle gap~1.7cm; idle bind-forward dots left.518/right.993. Inspected punk walk-step/run/idle and vendor run: shoes not reversed in these frames. This does not prove every pose, view, or other character rig. No bone orientation changes justified.

Compared punk walk-step with motion blur disabled, and with blur disabled plus FXAA. FXAA variant is visibly noisier and rejected. Blur-only-off variant sharpens fingers/shoes while retaining temporal smoothing. Set r.MotionBlurQuality=0 at player controller BeginPlay with game-setting priority; editor build passes. Console-override comparison verified rendering effect in installed052; new default itself is source-only until next package. Full-motion quality, grainy materials and broader character art remain unfinished. Reproducer Scripts/review_zombie_locomotion.py, --punk, --no-motion-blur, diagnostic --sharp.


## Zombie death physics inspection — 2026-09-12 [codex-maclaptop]

Current zombie death still rotates whole visible body sideways and removes it after5seconds; this needs replacement. Located Farmer_PhysicsAsset and Punk_PhysicsAsset. Python PhysicsAsset interface did not expose skeletal_body_setups; added native opt-in review inventory instead. Editor build and punk locomotion fixture pass.

Punk imported asset reports13 capsule bodies/12constraints: Root, Hips, Torso, Chest, Head, UpperArm_L/R, Wrist_L/R, UpperLeg_L/R, LowerLeg_L/R. Hips and upper legs parent to Body; no separate foot bodies listed. This is not the Casual rider physics hierarchy, so copying V6 unchanged is not justified. Need inspect capsule fit/constraint mapping and shoe behavior, then build a zombie-specific physical fall or compatible authored death animation. This inspection does not prove imported asset unusable or establish death quality. Farmer physics layout has not yet been natively inspected.

Scripts/review_zombie_locomotion.py now supports --editor and retains native body inventory. Installed052 unchanged.


## Physical zombie death — 2026-09-12 [codex-maclaptop]

Enabled current-pose physical death for both vendor and punk using their own mesh physics assets. Visible poseable body mirrors physical transforms after physics, retaining BodyInstance scale; world/visibility collision and Pawn/Camera ignore. Existing5second lifetime, weapon-drop and score logic unchanged; missing physics falls back to old animation.

Initial punk trial exposed root-parented shoes drifting away from shins. Capture each animated shoe-to-lower-leg transform at death and reconstruct shoe world transforms from physical shins, then recompute local pose. Vendor asset has Body/Abdomen bodies instead of punk Hips/Torso; both have Chest. Changed tracking anchor and test to Chest rather than assuming a physical Hips body. Both have13capsules/12constraints.

Build passes. Rendered punk integrated normal death path and vendor candidate tests pass physical chest tracking within10cm, head drop>60cm and canceled attack warning. Settled frames inspected for both: prone body, shoes attached. Prototype failures retained as evidence; vendor initial failure was an invalid hip-body test assumption. Full slopes/obstacles/multi-corpse performance, exact fit and packaged acceptance remain. Installed052 unchanged. Scripts/test_zombie_death.py --render [--punk] exercises actual fatal damage; no feature-enabling flag now required.


## Sloped terrain death checks — 2026-09-12 [codex-maclaptop]

Added --slope to zombie death fixture: scan real Landscape inside mapped park for an8–31.8degree slope, spawn and kill the model there, then inspect eight visible joint positions against terrain (Head, Chest, wrists, shins, shoes). Weapon drops disabled only in fixture to keep the view clear. Captures precede clearance assertion so failed contact remains reviewable.

Editor build and both vendor/punk runs pass on27.82degree slope. Minimum sampled joint clearance punk5.75cm/vendor3.34cm; required >=-15cm. Chest tracking/head drop/canceled attack warning also pass. Inspected both2.5second frames: slope-conforming bodies, attached shoes, but awkward limb poses remain. Joint clearance does not measure mesh skin penetration, and fixed2.5second image does not prove fully settled physics. Need closer contact/settling and scenery-obstacle tests before broader visual acceptance. Installed052 unchanged.


## Settling and mesh-contact failure — 2026-09-12 [codex-maclaptop]

Strengthened death fixture: sample all11010 punk rendered vertices against terrain at3.8seconds, report per-bone penetration and maximum physical-body linear speed. Accept mesh penetration no worse than3cm and speed<40cm/s. Capture remains before assertion. These checks supersede confidence from joint-only clearance.

Three punk slope landings: first failed -16.06cm/184below-ground vertices with speed0; second passed -1.09cm/3vertices (Foot_L),speed2.93; third failed -11.96cm/140vertices,all Foot_L,speed0. This is intermittent actual shoe/terrain penetration, not unsettled motion. Visual shoe-to-shin attachment does not supply physical shoe support; imported asset lacks foot bodies. Need add compatible shoe collision/ankle support and rerun mesh contact, rather than lifting whole corpse or accepting a single good landing. Runtime source still has new falls; installed052 retains previous zombie death and is unchanged. Do not promote zombie falls on current evidence.

Native build passes; mesh-contact fixtures intentionally report failures. Reproducer Scripts/test_zombie_death.py --render --punk --slope, with per-bone diagnostics retained in report. Full game remains unfinished.


## Physical shoe support — 2026-09-12 [codex-maclaptop]

Added fitted shoe boxes welded to physical LowerLeg_L/R at the animated death pose. Bounds exported from dominant shoe mesh vertices in bone-local coordinates:208vertices each vendor shoe,281each punk shoe; no runtime vertex scan needed. Generated BattleZombieShoeBounds.h via Scripts/prepare_zombie_shoe_bounds.py and editor helper InspectZombieShoeBounds. Two0.8kg boxes,0.8cm fit margin, CCD, same world collision/Pawn-Camera ignore policy as body. Audit asserts both weld parents are the intended shins.

First punk support run passes skin contact(+1.97cm minimum); repeat also has no penetration(+1.00cm),but65.34cm/s peak body speed at3.8s fails settling check. Added linear damping.8/angular2 to physical bodies. Final punk passes+0.97cm minimum/speed0; vendor passes-0.81cm minimum/speed.78cm/s. No vertices more than1cm below terrain in final runs. Both final3.8second images inspected; no deep shoe clipping.

Native build passes. This fixes the reproduced unsupported-shoe mechanism; one final landing per style on27.82degree slope is not proof for all terrain, obstacle contacts or corpse performance. Packaged acceptance pending, installed052 unchanged. Old failure evidence retained.
