# Limited horn and supplies

2026-09-11 [codex-maclaptop]

Elliott requested a five-use horn because it can make visitors yield, with extra horns to find around the map. Build 041 starts with five uses and carries at most five. H spends one use only when a horn actually sounds; the existing 0.65-second cooldown remains. Holding H does not repeatedly trigger the pressed binding. Empty use displays a find-pickup message and makes no horn sound or visitor call.

Six single-use horn pickups are placed on reachable dry paths: three park candidates and three trail candidates. Placement shares ground, water, obstruction, navigation and spacing validation with existing supplies. Each restores three uses, capped at five; a full inventory leaves the pickup intact. Bike and on-foot collection share the bike's inventory, which is retained across remounts and health recovery. A visible silver bulb horn and HORN +3 lettering distinguish these from time/ammo pickups. The HUD displays remaining uses on bike and foot.

Invalid-state guards prevent spending while parked, dead, stunned, recovering, paused, counting down or after the run ends. Refills also reject dead, paused, countdown and ended-run states. There is no passive horn recharge. Existing visitor response distance/line-of-sight/navigation and cooldown rules are retained. Skaters now advance the inherited horn cooldown even though they use their own route tick.

Initial tuning: +3 per pickup, six world pickups, five carried maximum. Native tests cover actual H presses, rapid retry, exhaustion, one-time collection, cap, leaving a full pickup, on-foot collection, remount and invalid-state guards. Visual inspection and installation results are recorded separately. Final pickup art and encounter balance remain open.

During review, the two-sided floating label overlapped against the bright sky. Added an opaque backing with separated front/back text planes. The first key test asserted before Unreal processed queued input; the final test waits for the next input frame and checks the resulting charge/sound counters before testing the cooldown.

Final Easy and Hard native audits both pass. Cooked bike/foot screenshots inspected at 1280×720: HORN 5/5 is readable, and the pickup has a recognizable bulb/bell with separated, backed lettering. Foot capture verifies the HUD rather than pickup framing.
