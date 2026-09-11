# Timer rules — build 030

2026-09-11 [codex-maclaptop]

Implemented: ten single-use +30-second tokens scattered on reachable park/trail surfaces; +10 seconds for an actual damaging zombie shot; -10 seconds for an actual damaging pedestrian shot. Shotgun pellets count once per victim per trigger pull. Frisbee launcher projectiles retain their existing once-per-victim collision guard and now apply the same timer rule. Misses and already-dead targets award nothing. Melee does not award shooting time.

Bike countdown remains 1x. On-foot countdown defaults to 1.25x, including swimming; this is the previously proposed tuning value, not individually playtested by Elliott. The HUD labels the faster rate and shows signed timer changes in a contrast panel.

AdjustRunTime centralizes mutations, refuses countdown/paused/ended-run rewards, rejects non-finite values and ends the run if a penalty exhausts time. A bonus cannot revive an ended run. Tokens require a live player, range and visibility, and are consumed only after the award succeeds.

Native BattleTimeAudit checks actual generated token count, pickup consumption, pause/countdown guards, possessed bike/foot clock behavior, real pistol hits on zombie/pedestrian targets, shotgun pellet deduplication, misses and end-of-run behavior. Visual captures require separate inspection. No physical-input or game-feel acceptance is implied.

Still outstanding: finite starting pistol ammo and findable pistol rounds; police spawning after three people hit; rifle/AK zoom; violence/Artifact escalation; drone and duck encounters; dog waste; bike jumping and +10 air rewards; the skate park and its activities. Police (-60) and animal (-10) categories have centralized timer hooks, but no claim that those actors/events are implemented. Final art, full game completion and overall timer balance remain unaccepted.
