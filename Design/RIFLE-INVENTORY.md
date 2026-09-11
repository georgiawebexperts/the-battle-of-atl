# Rifle inventory and zoom — build 039

2026-09-11 [codex-maclaptop]

A fifth inventory slot is a findable rifle. Every difficulty includes rifle crates within its existing weapon-crate budget. Picking one up gives 30 loaded rounds and 30 reserve; reserve caps at 120. Enemy weapon drops may also contain a rifle. Starting equipment remains ten pistol bullets, with no rifle unlocked.

On foot, key 5 selects an owned rifle. Hold right mouse for a 35-degree field of view, smoothly interpolated from the normal 85 degrees. Other guns retain 65-degree aiming. Release aim to restore normal view; reload and melee suppress zoom. R reloads in 2.4 seconds and transfers only available reserve. Rifle magazine and ownership persist across riding, dismount and checkpoint death.

Initial tuning: 30-round magazine, 0.12-second firing interval, 28 body damage, 150-metre trace range, 0.15-degree aimed spread / 3-degree hip spread. Existing headshot, obstruction, trouble and victim-specific time rules apply. Gunfire remains a risk even when no target is hit.

Visual source: Quaternius Zombie Apocalypse Kit rifle, CC0; see SourceAssets/Weapons/SOURCE.md. This is a rifle, not a claim to reproduce a particular AK. Authored scope optics, weapon-specific reload animations and dedicated rifle audio remain unfinished. Native integration and rendered fitting must pass before installing.

Native Hard inventory audit passed: six real weapon crates including rifle, selection via key 5, 35-degree zoom, reload restoring normal view, finite reserve conservation and remount persistence. First rendered check found unreachable support-hand targets; rifle-specific lower grips replace those targets. A second check cleared the aiming view but found a 2.6 cm reload overreach; the support grip and reload excursion were shortened again. Final native refit passes all wrist checks and reload/remount. Aim and reload screenshots inspected: reticle clear and rifle visible; full animation quality remains open. Existing frisbee supply/cycle and grass-riding regression also pass.
