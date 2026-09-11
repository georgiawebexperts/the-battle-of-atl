# Finite pistol ammunition — build 031

2026-09-11 [codex-maclaptop]

Each new run starts with ten loaded pistol rounds and zero reserve. The magazine holds twelve when resupplied. Twelve single-use pistol ammo crates, six in the park and six along the trail, each supply ten reserve rounds; placement uses the existing reachable-path, spacing, ground and water checks. Reserve caps at sixty; a full player leaves the crate available. Partial capacity accepts only what fits.

Foot reload uses R and conserves rounds. Bike firing while empty initiates its existing 1.5-second automatic reload only if reserve is available. Dismount cancels a pending bike reload. Switching weapons cancels a foot reload. Ammo is shared between possession states and persists through the existing checkpoint death system. A new run resets it. HUD shows loaded/reserve counts.

BattleAmmoAudit exercises real shots, empty behavior, collection on both pawn types, repeat guards, both reload paths, reserve cap and dismount during bike reload. The ammo fixture freezes unrelated enemy spawns and removes traffic; this verifies ammo behavior, not live combat balance. An initial unisolated Hard run failed a firing step amid spawned enemies, so that failure is not treated as proof of an ammo defect or a passing live-combat test. The weapons and ammo crates still use interim art. Police/escalation, rifle/AK zoom and remaining full-game requirements are separate unfinished work.
