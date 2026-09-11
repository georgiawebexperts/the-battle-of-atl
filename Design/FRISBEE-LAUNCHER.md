# Frisbee launcher and enemy weapon drops

2026-09-11 [codex-maclaptop]

Glowing crates now cycle through shotgun, SMG and frisbee launcher types. The
launcher uses slot 4, the existing persistent inventory and R reload. Its first
crate supplies 8 loaded discs and 8 reserve discs, with a 32-disc reserve cap.
It fires once every 0.55 seconds. Its view model includes a glowing disc hopper;
the moving disc also glows and spins. Launch and ricochet sounds are original
synthesized effects from Scripts/generate_disc_audio.py.

ABattleDisc moves at 2,600 game cm/s. Swept sphere collision follows the complete
frame's travel, including remaining distance after a hit or ricochet, with a
12-contact-per-frame cap to bound collision work. It deals 65 ordinary damage to each character
it encounters and continues onward. Each disc remembers struck actors so a
returning ricochet cannot damage the same target again. Surviving zombies are
interrupted and nudged back; kills award the existing statistic and nitro reward.
The reward belongs to the persistent bike, so changing possession does not lose
credit or leave a dangling shooter reference.

Walls reflect its velocity. After four ricochets it is destroyed on the next
solid impact. Initial penetration destroys it; an eight-second age limit and
run-end cleanup bound its lifetime. The muzzle path is swept before spawning so
a gun touching a wall cannot create a projectile on the far side. The launch
pawn, bike and current rider are ignored to avoid self-hits after remounting.

Defeated zombies have a 25% chance to drop a random secondary weapon/ammo crate.
Drops require a nearby ground hit, acceptable slope and clearance. They expire
after 60 seconds and use the same range, line-of-sight, state, capacity and
single-use pickup rules as placed crates. Death can create at most one drop.
The probability is configurable on the zombie; native validation forces it to
one only for its controlled drop fixture.

Scripts/test_mac_disc.py tests real crate acquisition and slot-4 input, then
moves the player and two frozen zombies to a floating collision fixture. A real
launched disc hits both zombies, reflects off a wall, and does not damage them
again on return. A second disc kills both and awards rewards. The test also
checks reload conservation and age cleanup. A separate grounded zombie fixture
checks the normal drop placement, pickup and duplicate protection.

Rendering, animation feel, sound quality, human aiming and performance require a
visible playtest. The Meadow frisbee-player ammo source is still missing. This
launcher and the new drops do not complete park-life groups or the full V3 game.
