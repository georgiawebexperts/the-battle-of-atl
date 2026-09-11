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
