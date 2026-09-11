# Weapon inventory and crates

2026-09-11 [codex-maclaptop]

A new run starts with the pistol. Glowing labeled crates unlock the shotgun or
SMG and supply ammunition. The difficulty table's WeaponCrates field controls
the finite supply; types alternate during placement. Crates use the same actual
path/gravel/bridge, clearance, water and complete-navigation checks as health
pickups and share their minimum spacing. Collection works on bike or foot within
150 cm with a clear line of sight. Dead/unpossessed/paused players cannot collect;
a crate remains when the corresponding reserve is already full.

On foot, 1 selects the pistol, 2 the shotgun and 3 the SMG. Unowned selections do
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

The frisbee launcher/ricochets, enemy weapon drops, Meadow ammo source and final
weapon presentation remain required. Key 4 is reserved and does not unlock a
placeholder weapon. The broader game remains unfinished.
