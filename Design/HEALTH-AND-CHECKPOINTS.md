# Health and checkpoint flow

2026-09-11 [codex-maclaptop]

The V3 specification supersedes the old one-bullet/two-stab death rules. The
bike stores the player's shared health, damage cooldown and current checkpoint.
The possessed on-foot rider displays and damages that same health. Dismounting
and remounting preserve both health and the regeneration delay. An empty bike
cannot take damage for a rider who has dismounted.

After five seconds without damage, health regenerates at four points per second,
up to100. At zero health, deduct ten seconds once and start a two-second wipeout.
Restore the player on the bike at the checkpoint, retaining Artifact/checkpoint
progress, ammunition and bike-owned stats. Clearance is checked before teleport;
if the checkpoint is obstructed, retry until it is clear. Two seconds of damage
protection prevent immediate repeat death. If the time penalty exhausts the
clock, the run ends instead of recovering. The initial checkpoint is the start.

The sourced Kroger and Krog Street Market landmark centres are projected to the
nearest installed trail samples. These are gameplay checkpoint locations, not
surveyed plaza entrances. Their buildings/architecture remain required world
work. Source: References/checkpoint-landmarks-osm.json, OSM way741961704 and
relation5413435. Regenerate anchors with Scripts/prepare_battle_checkpoints.py;
world coordinates remain east/south/up. Checkpoints activate only after Artifact
pickup and in order, within320cm horizontally and180cm vertically with line of
sight. Radar target and gold route then advance to the next checkpoint. The
Cabbagetown finish remains unimplemented, so the final current target is the
southern Krog exit, with no shortcut victory.

The health bar and nitro meter are placed at the lower left in both bike and
FPS modes. Coke pickups are implemented in BattlePickup and documented in COLA-PICKUPS.md.
Expanded enemy attacks, full death presentation,
checkpoint artwork, final HUD styling and rendered acceptance remain pending.

The opt-in cooked BattleHealthAudit exercises damage in both modes, empty-bike
immunity, health/cooldown continuity across possession, real delayed regeneration,
Artifact pickup, checkpoint ordering, both death modes, single ten-second
penalties, timed recovery, post-respawn protection and retained quest progress.
It uses teleport fixtures to reach interactions; it does not validate travel
between checkpoints or human input/window rendering. Existing Mac driving and
quest regressions cover separate movement and mode behavior.
