# Gameplay revision after Elliott's live Mac playtest

2026-09-11 [codex-maclaptop]. User decisions below override V3 where they conflict. The full park/BeltLine game remains the objective. The old goal was stopped for this discussion; Elliott has now authorized resuming implementation.

## User decisions

All three experiences matter: explore and ride through a believable Piedmont Park to find the Artifact; finding it sharply increases danger during the trip home; fighting is an integral option. Start with 10 bullets, with more available to find. Every shot attracts trouble, including more zombies and random violence. Characters can be shot. Gunfire and deliberately running people over can attract an occasional police officer with a taser. Taser hits cause a stun/fall off the bike and a time penalty, followed by recovery; no arrest or automatic checkpoint reset.

The countdown runs noticeably faster on foot and at normal speed on the bike. A 1.25x foot multiplier was proposed by the assistant but has not been individually confirmed or playtested. The taser time penalty amount is still a tuning decision.

Controls and fonts must be readily readable; getting off the bike must be obvious. Aiming, outgoing hits, incoming shots, and attacker locations need clear feedback. Current character appearance, artificial movement, silly bump/wipeout animation, jerky steering and leaning, slow zombies, and bare landscapes are rejected. The desired quality reference is GTA; this does not constitute a claim that GTA production quality has been achieved or can be promised.

User requests a switch between real bike physics and arcade handling. Both must feel smooth. Great impact/fall/stab/recovery sequences are required, not the current placeholder tumble.

## Immediate acceptance order

User's latest correction prioritizes visual quality and readable text. Rebuild the HUD, render actual gameplay screenshots and review them before further mechanic expansion. Improve scene composition and foliage, with truthful asset provenance. The interim Epic template tree does not satisfy the specified Megascans oak/magnolia species or photo-matching requirement. Character animation and collision feel still need a separate overhaul and a real playtest.

Technical test success never establishes fun, readability, animation quality or resemblance to Atlanta. Do not label the full game complete while those are unproven.

## 2026-09-11 — Battle for the ATL expansion [codex-maclaptop]

User renamed the game **Battle for the ATL** and the existing store **Murder K**. Preserve the store geometry and all other store details.

Add occasional, sparse drones that swoop down. A drone collision harms the rider, knocks them off the bike and costs recovery/remount time. Add ducks that fly into the pond; duck collisions while swimming briefly force the swimmer under and cost time. Add park benches, bike jumping and usable ramps. Zombies should have ragged, worn clothing and a visibly undead appearance, distinct from ordinary pedestrians.

Allow finding different guns, including a rifle/AK with zoom while aiming. Police begin appearing after the player hits three people; this refines the earlier gunfire/runover escalation request. Add the real skate park on the right when heading south after Murder K near the Freedom Parkway underpass, with rideable features, skateboarders, sightseers and bonus-time opportunities. Research identified Historic Fourth Ward Skatepark (now also listed by the BeltLine as Thomas Taylor Memorial Skatepark); geography and photo references still need implementing.

Time rules requested: scattered pickups add 30 seconds; catching air on the bike adds 10 seconds; shooting a zombie adds 10 seconds; shooting a pedestrian, pet or duck subtracts 10 seconds; shooting a police officer subtracts 60 seconds; riding through dog waste in the grass subtracts 10 seconds. These rules are not implemented merely by documenting them. Avoid repeat overlap deductions every frame and tiny ground-contact jitter triggering repeated air rewards. Exact drone/duck collision penalties remain tuning choices; preserve the specified numeric rewards/penalties above.

References located: https://www.h4wpc.org/skate-park/ (Conservancy location/features); https://beltline.org/parks-trails/eastside-trail/ (current naming and trail connection); https://www.atlantaareaparks.com/directory-parks/listing/historic-fourth-ward-skatepark/ (photo reference); https://www.ajc.com/news/photos-sunday-fun-historic-fourth-ward-skatepark/VBgIUZOx8a96jI2dcI4o7I/ (photo gallery near Freedom Parkway). Images are references, not licensed game textures.
