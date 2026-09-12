# Weapon inventory and crates

2026-09-11 [codex-maclaptop]

A new run starts with the pistol. Glowing labeled crates unlock the shotgun or
SMG or frisbee launcher and supply ammunition. The difficulty table's WeaponCrates field controls
the finite supply; types cycle through all three during placement. Crates use the same actual
path/gravel/bridge, clearance, water and complete-navigation checks as health
pickups and share their minimum spacing. Collection works on bike or foot within
150 cm with a clear line of sight. Dead/unpossessed/paused players cannot collect;
a crate remains when the corresponding reserve is already full.

On foot, 1 selects the pistol, 2 the shotgun, 3 the SMG and 4 the frisbee launcher. Unowned selections do
nothing. The HUD shows the current gun, loaded rounds, reserve supply and which
weapons still need a crate. R reloads. Changing guns cancels an unfinished reload
without taking reserve ammunition. F melee and firing cannot overlap a reload.
The bike always uses the pistol. All magazines, reserves, ownership and the last
on-foot selection survive remounting, dismounting and checkpoint death, within
that run. This is not a save-to-disk system.

- Pistol: 12 loaded rounds, unlimited reserve; existing damage and fire behavior.
- Shotgun: 6 shells, a crate supplies 18 total rounds on first acquisition;
  0.85 seconds between shots, 2.2-second reload, eight pellets dealing up to
  18 damage each. Damage falls from full at 4 m to 15% at 24 m in game units.
  Aim tightens spread. Hits push back and interrupt surviving zombies.
- SMG: 30 loaded rounds, a crate supplies 90 total rounds on first acquisition;
  0.085 seconds between shots, 1.8-second reload, 14 damage per bullet.
  Aim tightens spread from 3 degrees to 1 degree.

Additional crates add reserves, capped at 60 shells or 300 SMG rounds. Magazines
refill only at reload completion, conserving finite ammunition. Pistol state is
kept separate when changing possession so a shotgun magazine cannot become bike
pistol ammunition. The legacy explorer's reload-completion hook remains a
12-round pistol refill; BattleRider overrides it for its current weapon.

Long guns use procedural receiver/barrel/stock/grip geometry with distinct pump
and magazine shapes, existing muzzle flashes/tracers/hit feedback, recoil, and
original synthesized reports. Final art, audio, balance and physical controls
need a rendered playtest. Scripts/generate_longgun_audio.py and
Scripts/prepare_longgun_assets.py reproduce the new audio assets.

Scripts/test_mac_inventory.py exercises actual crate layout, automatic bike and
foot pickup, numbered input keys, close shotgun combat, rapid SMG combat, timed
reload conservation, and loadout persistence through possession and death.
Enemies are frozen and positioned for controlled collision tests. Rendering,
human aim, sound quality and frame rate are outside the headless test's scope.

Build 024 adds slot 4, moving ricocheting discs and enemy weapon drops; see
Design/FRISBEE-LAUNCHER.md. The Meadow ammo source and final weapon presentation
still require work. The broader game remains unfinished.


## Hit confirmation — 2026-09-12 [codex-maclaptop]

Added confirmed-hit labels to real pistol/long-gun results: Zombie hit/down, police hit, attacker hit/down, person hit; multi-victim shotgun reports target count. Applied damage/previously-live target required, so world impacts do not produce these labels. Shared bike state retains last confirmation .75seconds across rider possession; hit X persists .35seconds. HUD puts message beneath aim target label.

Editor build and real-shot inventory fixture pass with added assertions for shotgun kill message, expiry, and first nonlethal SMG hit. Existing reload/ammo/death/zoom/remount assertions also pass. Friendly/multi-target rendered feedback still needs review; source-only, installed051 unchanged. Investigation also finds old APiedmontThreat gunman spawning belongs to APiedmontRideMode; current ABattleEnemyDirector handles zombies and does not integrate that gunman. Incoming-fire awareness therefore needs actual current-mode shooter integration, not merely a HUD overlay.


## Current-mode gunman core — 2026-09-12 [codex-maclaptop]

Added ABattleGunman using current shared rider-health system, existing character/pistol art and real tracer/muzzle flash/gunshot.1.8s warning before shot, aim locks at warning start;2200cm acquisition range,4s shot cooldown. Visibility trace checks both barrel path and shot. Successful uncovered player hit applies fatal shared damage; practice/countdown/end/pause prohibit acquisition/firing. Damage interrupts warning, gunman health80. HUD warning says GUNMAN AIMING—MOVE TO COVER. No natural spawn yet; only audit creates this prototype.

Editor build passes after removing duplicate inherited ShotsFired/LastShotEnd declarations. Native test GunmanAudit passes practice suppression, minimum windup, cover introduced after aim blocking shot, relocation dodging locked aim, stationary fatal hit/death count. Scope is one stationary fixture; rare spawn director integration, direction/audio/render/animation quality and broader death/reset rules remain. Existing character death presentation remains rough. Installed051 unchanged. Reproducer Scripts/test_gunman_core.py.
