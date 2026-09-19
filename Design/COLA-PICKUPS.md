
## Trademark rename

2026-09-19 [codex-windows]: visible can lettering renamed from the branded cola text to
'Soda Pop' (trademark - do not restore brand text). Internal code names unchanged.

# Soda Pop health pickups

2026-09-11 [codex-maclaptop]

BattlePickupDirector places a finite, per-run supply of cans on verified paved,
gravel or bridge surfaces. Every placement has a complete navigation route from
the player start, clearance and water checks, plus1600cm separation. The two
checkpoint locations are prioritized. The remaining supply is divided between
park paths and the return trail. Difficulty.csv/DT_Difficulty tune count and heal
amount:18/12/8cans on Easy/Medium/Hard, healing35points each. The current splits
are12park+6trail,8+4and6+2. No regeneration of consumed cans is implemented.

Cans bob/rotate, have a red material, Soda Pop lettering, a small glow and an
original generated can-opening/fizz sound. Collection is automatic within150cm
of the controlled player with an unobstructed sight line. Both bike and foot use
the bike-owned shared health. Healing caps at100and does not reset the damage
cooldown or respawn protection. Full-health players leave the can available.
An empty parked bike, a distant player, a dead player, a paused/countdown state or
an ended run cannot collect. Successful collection is single-use and increments
the bike-owned health-pickup statistic, with a short HUD notice.

Dedicated pre-existing health/quest/terrain/zombie audit flags suppress pickup
placement so those fixtures retain their intended scope. BattlePickupAudit
leaves actual layout active and freezes enemies only. It validates all three
actual counts against the source CSV, both checkpoint supplies, bike and on-foot
healing, capped healing, preserved full-health cans, duplicate protection, wall
occlusion, unoccupied bike exclusion and death guards. Automatic collection is
exercised through actor ticks; teleports and a temporary wall isolate cases.
This does not prove human driving pickup feel, artwork, sound quality or FPS.

Scripts/generate_cola_audio.py creates original audio;
Scripts/prepare_cola_assets.py creates/imports the render/sound assets.
The Mac remains locked at this milestone, so final visual/audio acceptance is
pending. Other weapons/crates, remaining enemies and full game content are still
required; health pickups do not complete the game.
