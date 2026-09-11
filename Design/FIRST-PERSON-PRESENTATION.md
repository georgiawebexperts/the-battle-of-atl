# First-person presentation revision

2026-09-11 [codex-maclaptop]

Build 028 lowers and moves the resting pistol forward to leave the center of the view clear. Right mouse smoothly brings it toward the center; the existing aiming field-of-view transition remains. Aim blending uses exponential decay so its response does not depend on frame rate. Aiming reduces walking weapon bob.

The former 0.65 hand-only scale broke the relationship between hands and forearms. Restore native hand proportions and move the camera-attached arm rig forward so the pistol grip targets remain reachable at rest, while aiming and during the reload motion. Do not stretch the skeleton to make a render pass.

The native development-only BattleRigReview fixture captures resting, aiming and reload views, checks both wrist target distances in each phase, then checks reload completion and remount. Scripts/test_mac_presentation.py --build 028 --rig invokes it on the cooked app. Every run writes to a fresh unique sandbox capture directory, so older images cannot satisfy a failed capture. Screenshots still require visual inspection.

This is a correction to view obstruction and the existing rig, not approval of final character quality. The Quaternius CC0 low-poly arms and NPCs remain placeholders rejected by Elliott. No realistic reload/magazine animation, recoil animation overhaul, character replacement, crash/stab/ragdoll overhaul, pedestrian movement overhaul, incoming-fire feedback or revised ammunition/escalation mechanics is claimed here. Long-gun and melee fitting still need dedicated visual review.
