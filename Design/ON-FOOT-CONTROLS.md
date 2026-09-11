# Ellison on foot

2026-09-11 [codex-maclaptop]

User direction: dismount without automatically holding a gun; draw deliberately. Show natural empty-handed arm swing. Provide walking, running, jumping and crouching.

Controls: WASD/arrows move, Shift runs, Space jumps, C toggles crouch, Control holds crouch, G draws/holsters the selected weapon, 1–5 select and draw owned weapons, E remounts near the bike. Preserve existing ammo/loadout across mount cycles, but every newly dismounted rider starts holstered. Left/right mouse while holstered neither draw nor fire/aim. Drawing has a short raise delay before firing. Holstering cancels aiming/reload without consuming ammunition. Swimming or incapacitation can put the weapon away; it must not automatically reappear afterward.

First-person empty arms swing in opposite phases according to actual movement speed and ease toward rest when stopped. Hands use a relaxed curl instead of a firing grip. Armed/melee arm targets blend from that pose. These are procedural view-model poses; rendered animation review is required and numeric target movement alone is not visual acceptance.

Crouching reduces the actual capsule, lowers the view, and limits movement speed even with Shift held. Standing remains blocked under a low ceiling. Preserve timed-game foot multiplier and existing health/death rules. Native checks cover dismount state, deliberate draw, fire protection, walking/running, jump/landing, crouch clearance, and ammo retention through remount/dismount. Initial implementation awaiting cooked acceptance at this entry.

2026-09-11 [codex-maclaptop] Cooked native acceptance passes: hands-free dismount, G draw/holster and draw delay, protected firing, walking/running, jumping/landing, C crouch/camera, speed cap, blocked standing under a ceiling, and inventory-preserving remount. Rendered walking pair/run inspected after raising empty-hand targets for visibility. Arms alternate visibly; procedural poses and motion blur remain below final animation quality. Control hold is bound but not independently exercised. Inventory regression also passed.
