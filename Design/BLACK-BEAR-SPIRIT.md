# The Black Bear spirit

2026-09-11 [codex-maclaptop]

User-requested fictional encounter for The Battle of ATL. Keep the personal inspiration out of game text, credits, publicity and implementation notes. Present the scene quietly and respectfully.

Place a small, subtle flower memorial beside the BeltLine just before Murder K on the southbound approach. A rare spectral black bear can appear in that vicinity. Its visual treatment should feel mysterious and beautiful; it is a benevolent spirit, never a combat target.

The encounter must be difficult to obtain and catch. Only a living mounted rider can catch it. Catching restores health and refills the difficulty's starting timer; preserve any already-earned time above that value. This does not reset the phone or checkpoints, or erase elapsed run time used for records.

Bullets/projectiles pass through; the spirit cannot be damaged or counted as a zombie, pedestrian or police target. Dismounting, swimming or being knocked off while the encounter is active immediately ends it. Remounting must not respawn it. Track attempted/active/resolved state for the whole run, and permit at most one opportunity per run. Restarting the game run permits a fresh opportunity; health death or reacquiring the phone does not.

Unimplemented at this entry. Required evidence: native mounted catch and timer/health restoration, no reward on foot, irreversible disappearance after dismount/remount/knockoff, projectile noninteraction, one opportunity per run, rare eligibility and expiry, and native visual review of spirit/memorial placement. Avoid claiming visual quality from logic tests alone.

## Presentation and encounter direction — 2026-09-11 [codex-maclaptop]

Keep the memorial a small arrangement of flowers set beside the trail on the approach to Murder K. No name plaque, quest marker, explanatory caption or spectacle. It remains present when the spirit is absent. Keep bicycles and pedestrians from trampling the flowers without obstructing the through trail.

The spirit should read as a black bear with a dark translucent body, a fine silver-blue rim and a faint drifting trail. Give its appearance a slow reveal and its departure a gentle dissolve. Avoid a bright collectible outline, health bar, target reticle response or combat hit effects. Use a restrained, original ambient sound cue, with a visual equivalent so the encounter remains discoverable without audio. Do not substitute an unreviewed low-poly animal or primitive placeholder and call the scene finished.

Proposed first balance pass, subject to playtesting: make one eligibility roll on the first approach during a timed run, with a 15 percent chance. Consume that chance even if it does not appear, preventing repeated visits from farming the encounter. If the player approaches on foot, consume the opportunity without spawning it. When it does appear, allow about 12 seconds to reach it on the bike along a short, safely rideable route near the memorial. Difficulty should come from noticing it and riding accurately; do not require an impossible speed or crossing a wall. Catch distance, route and speed need native playtesting before these values are accepted.

Use an explicit whole-run encounter state: untried, absent, appearing, active, fading, resolved. A transition away from mounted riding cancels appearing/active immediately, even if the visual dissolve continues briefly. No catch or reward during a dissolve. Track the transition itself so dismounting and remounting between encounter ticks cannot bypass the rule. Apply the same cancellation to knockoffs, water entry, death and run end. Pause freezes the encounter. A new full run clears its state; checkpoint recovery and phone recollection do not.

On a valid catch, restore living Ellison to full health and refill the timer to its starting value, preserving any greater balance already earned. Keep the phone, checkpoints and elapsed-time record intact. A short readable notification may say “TIME RESTORED”; the memorial receives no explanatory story text. The spirit then fades away and cannot reward the player again that run.

Implementation and rendered Mac acceptance remain pending. This is the encounter specification, not evidence that it is in the installed build.

## Native rules implementation — 2026-09-11 [codex-maclaptop]

`ABattleSpirit` now exists once per park run. The actor cannot take damage or collide. It owns untried/absent/appearing/active/fading/resolved state, a 1.5-second reveal, 12-second opportunity, 1.5-second fade and the proposed 15% eligibility boundary. A valid mounted catch restores health and the selected difficulty's starting time while preserving a greater time balance, phone/checkpoints and elapsed record. Bike `UnPossessed` cancels synchronously, closing the dismount/remount-between-ticks loophole. Death cancels immediately; stun/recovery, water occupancy and ended runs invalidate the mounted predicate. Pause freezes advancement.

The native Easy and Hard rule audits each pass 29 assertions: live-run gates, rarity boundary, invalid input, failed-roll persistence, reveal, pause, mounted reward, extra-time preservation, quest/elapsed invariants, one reward, fade/expiry, on-foot chance consumption, same-frame remount cancellation, stun and death. A sphere attached to the spirit is transparent to the weapon trace; temporarily enabling collision makes the same fixture block, establishing that this assertion exercises a real collision shape. These checks call the encounter entry/advance methods in a controlled fixture. They do not establish real-world rarity frequency, a rideable chase, actual drone/water-entry choreography, restart/recollection behavior across a full run, or visual/audio quality.

**Not enabled for players:** `bPresentationReady` remains false. No bear mesh, spectral material, flowers, sound or geographically verified chase has been integrated. The installed desktop remains build 042; this is work toward candidate 043. Enable only after configuring and reviewing the presentation and route. Do not claim the spirit is playable or finished from the rules audit.

Asset leads for subsequent inspection (not acquired or approved): [Nyilonelycompany rigged bear](https://sketchfab.com/3d-models/bear-ce0d5eb86cf5459bb6bd20244cb44b27) lists 12.1k triangles and CC Attribution; browser download/access remains to investigate. [Louisiana American Black Bear](https://godotmarketplace.com/shop/louisiana-american-black-bear/) by the same creator lists Unreal FBX and 15 animations; paid option, not purchased. Inspect species shape, motion, license and actual source files before use. No third-party files were imported this increment.

## Memorial and route increment — 2026-09-11 [codex-maclaptop]

The memorial is now authored and present in the cooked candidate: thirteen ivory/lavender flowers with curved petals, leaves/stems, centers and a muted tie on a shallow stone. Original source geometry and reproducible import are in `SourceAssets/Spirit` and `Scripts/prepare_spirit_scene.py` / `import_spirit_memorial.py`. It has no plaque, label, quest marker or explanatory caption. It is a separate `ABattleMemorial` actor, so encounter resolution does not remove it.

Placement is on the right of the southbound retained Eastside route, 2,200 game cm before the Murder K checkpoint, about 260 cm from trail centerline. This is a fictional placement near the requested landmark, not a surveyed site. A compact collision base protects the arrangement. Native review confirms six cooked parts/materials, protective collision, terrain embedding and 63 unobstructed riding-line capsules (ambient characters ignored for this fixed-geometry check). Corrected wide and close native screenshots were inspected. The flowers are recognizable and quiet; the surrounding terrain/landscape and material detail still need work.

The adjacent route passed actual W/A/D bike traversal in both directions: 6,055.32 cm travelled, 20.33 cm maximum centerline deviation, 590/590 paved ground samples, zero wipeouts. This proves the trail is rideable with the memorial present, not that a bear chase is balanced or complete. `ABattleSpirit::BeginPlay` now receives the sourced approach and chase points. `bPresentationReady` remains false pending bear model/animation, spectral appearance, audio and full encounter playtest. Desktop build 042 is unchanged.

Import caveat: UnrealEditor saved all assets and produced its import manifest, then encountered a shutdown fatal error in editor ModeManagerInteractiveToolsContext teardown. The owned failed editor process was closed. Subsequent packaging succeeded and fresh native renders verified the saved assets. Do not describe the import process itself as a clean editor exit.

## Bear candidate found and rendered — 2026-09-19 [codex-maclaptop]

The blocker has always been the mesh: the CC0 GitHub mirror used for the rifle and
SMG has no bear anywhere in its 2049 files, and Fab/Sketchfab need Elliott's signed-in
browser session, which is unreachable while the Mac is locked. OpenGameArt serves its
files over plain HTTP with no login, so the hunt moved there. Two candidates actually
read as bears:

1. **Low Poly Bear** by mathildelea, **CC-BY 4.0**, `https://opengameart.org/content/low-poly-bear`.
   Binary FBX, 35,852 bytes, plus a base-colour PNG. Staged at
   `SourceAssets/Spirit/BearCandidate/` with the sha256s and licence in `SOURCE.json`.
2. **Animal Figurines** (CC0), whose bear is a carved-wood figurine — second choice,
   squared muzzle and no ears in silhouette.

Rejected: **White Bear Low Poly** (CC0) ships only as `Bear.blend`, and Blender is not
installed on this Mac, so it cannot be converted here.

`Scripts/preview_bear_candidate.py` imports the candidate into a scratch folder,
builds a flat-brown material and a spectral preview material (near-black translucent
body, Fresnel silver-blue rim), stages the mesh on the Fourth Ward grass and renders
four views to `work/bear-candidate-review/`. Native results, both inspected: the
authored view reads unmistakably as a quadruped bear at 180 cm nose to tail (the file
is authored 569 cm long and the preview scales it), and the spectral view reads as a
translucent ghost that lets the grass through, which is the intended treatment.

What this is not: the mesh is a **static** import with no skeleton and no animations,
so a bear that walks or turns is not available from this file. Two things have to be
decided before it ships. First, does Elliott accept a low-poly mesh for a figure that
only appears for about twelve seconds, halfway to translucent? The note above forbids
calling an unreviewed low-poly substitute finished — this is the review material, not
the acceptance. Second, CC-BY 4.0 requires a visible credit for mathildelea, which is
a credits-file change and a separate decision from the rule that the personal
inspiration behind the spirit stays out of credits and game text.

`ABattleSpirit::bPresentationReady` is untouched and remains false. Nothing was
installed, placed or enabled; the staged actors are transient and the map was not
saved.

## The body replaces the card — 2026-09-19 [codex-maclaptop]

Elliott's "the bear or angel or whatever still doesnt look good" was aimed at a
**flat painted card**: a 2.7 x 1.85 m unlit plane (`SpiritCard`) standing in the
trail. No art on a plane fixes that, because a plane reads as a cut-out from
every angle except the one it was drawn for.

The spirit now carries the 3D bear staged above as its primary body
(`SM_SpectralBear`, `BearBody` in `ABattleSpirit`): yawed so the nose points down
the actor's forward, scaled to **1.90 m nose to tail and 1.12 m at the withers**
beside a 1.8 m rider. The file's pivot is not at the animal — its bounding box
sits 279 cm along local X with the feet 7 cm above the origin — so the body is
centred from its own bounds rather than from a hard-coded nudge.

Three findings, all from renders rather than reasoning, and all worth keeping:

1. **A translucent unlit material renders nothing in this project.** The body
   was genuinely drawn — `WasRecentlyRendered` returned true — and left no
   pixels. Two screenshots with no numbers cannot tell that apart from "the bear
   is not there", which is how a pass was lost. `-BattleSpiritReview` now logs
   state, component visibility, world bounds, camera position and
   `WasRecentlyRendered` at each capture.
2. **A material built by these editor scripts still does not come out right.**
   `M_SpectralBearBody` (kept in the repo for whoever sorts that out) drew as
   default lit grey with the moonlight picking out a blue edge. The body is
   therefore tinted from `/Engine/BasicShapes/BasicShapeMaterial` at
   `(.015, .020, .045)` — the one material this project has repeatedly seen draw
   in a cooked build.
3. **Writing a material graph and looking at a picture is not verification.**
   The first opaque pass was reported as working because a bear-shaped thing
   appeared in frame; it was the default material.

The card and the nine-primitive figure remain as hidden fallbacks, and
`-BattleSpiritCard` still swaps the card back in for comparison. The encounter's
rules, the approach, the chase route and `bPresentationReady` are unchanged —
only the thing being looked at changed.

Still open, and both are Elliott's call rather than an implementation detail:
the mesh is **static, with no skeleton and no animations**, so a bear that walks
or turns its head is not available from this file; and **CC-BY 4.0 requires a
visible credit for mathildelea**, which is a credits-file change.

## The drifting trail — 2026-09-19 [codex-maclaptop]

The note above asks for "a fine silver-blue rim and a faint drifting trail". The
rim comes from the moon glow. The trail is now real: while the encounter is
visible, the body records its position every 0.22 s and the last five samples
are placed behind it as shrinking wisps. They are **world-space**, so they hang
back along the route the animal took instead of following it around, and they
reset when the encounter ends. Five more wisp components were added for this;
the three ambient ones still drift upward as before.

`-BattleSpiritReview` captures it: at the Murder K approach the bear is a
near-black body with the cold light along its upper edge and a line of small
blue points trailing back along its path.
