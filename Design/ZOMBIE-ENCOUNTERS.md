# Zombie encounters

2026-09-11 [codex-maclaptop]

BattleEnemyDirector uses the actual difficulty row for zombie population,
shambler/sprinter speed, sprinter fraction, attack damage, warning time and
replacement interval. It selects collision-clear, navigation-reachable positions
near the player, rejects the lake, and retires distant live actors beyond7500cm.
Spawning waits until the countdown ends. Existing opt-in terrain/quest/health
fixtures freeze this new subsystem to retain their intended scope; separate
zombie audits exercise it. These fixture flags do not disable enemies in Shipping.

Zombies use AAIController and pathfinding movement. Attacks have a visible windup
and require proximity, height agreement and line of sight again when they land.
A moving player can evade the attack. A hit damages shared rider health and a
successful grab wipes out a mounted bike. Pistol body hits deal34; head-region
hits receive a3x multiplier. Death removes collision, stops AI, awards the normal
pistol kill/nitro reward once, and removes the corpse after5seconds. The last1.5
seconds animate a masked dissolve. The rigged CC0 Casual mesh has a green-gray
material and emissive eyes, with emergence, twitching and hit reactions.

Hard's TunnelWaveSize drives waves restricted to the tunnel darkness volumes.
A45second wave cooldown permits additional waves while the player remains in the
tunnel. The population cap allows the configured regular population plus one
wave, with additional queued wave spawns waiting for capacity. Leaving the tunnel
cancels unspawned wave members so normal spawning resumes. Normal spawns are
not guaranteed to be hidden by scenery; authored bushes/dumpsters/wall entrances
are still required world/encounter presentation work.

SourceAssets/Data/ZombieLines.txt contains30original lines, used at spawn and
periodically during nearby pursuit, with HUD subtitles. Sprinters have an
additional charge warning. Scripts/generate_zombie_sources.py regenerates the
header and original procedural growl. The growl is not a recording of those
lines:30spoken performances, a real sprinter shriek and headshot crunch remain
pending audio work. Full ragged clothing/head-pop presentation and rendered
appearance are not accepted by the current headless tests.

Scripts/prepare_zombie_assets.py creates skeletal-mesh-compatible masked skin,
emissive eye material and imports the growl. It requires the new compiled
BattleDifficultyRow before Scripts/install_difficulty.py imports updated CSV
values. SourceAssets/Data/Difficulty.csv remains the tuning source.

BattleZombieAudit uses a real director spawn and checks navigation travel,
telegraph, shared health damage/wipeout, actual FPS pistol body/head hits, kill
rewards and timed corpse cleanup. Its later firing fixtures freeze target pursuit
and reposition it for precise damage checks. BattleZombiePopulationAudit keeps
normal spawning active but freezes pursuit so it can count actual spawned Easy,
Medium and Hard populations, validate their applied tuning, then trigger and
check a covered Hard tunnel wave, a timed repeat wave, and normal spawning after
leaving a partially spawned wave. Neither establishes visual crowd density,
real-world game balance, audio quality, human keyboard input or frame rate.


## Geographic restriction — 2026-09-12 [codex-maclaptop]

New user requirement, not implemented: Farmers Market Vendor zombies appear only inside Piedmont Park, including pursuit boundaries. Homeless-themed and punk-rocker variants can appear anywhere in the playable world. Market beside Billy’s at 12th Street motivates the vendor variant. See WORLD-EXPANSION.md, Farmers market at 12th Street, for reference photos and tutorial entrance blocking.
