# The Battle of ATL — Mac playtest 047

2026-09-12 [codex-maclaptop]

Installed on the desktop as version 0.47.0. Build 046 retained as TheBattleOfATL-build046.app. Packaged traffic frames, both collision-direction tests, health/checkpoints and strict code-sign verification passed. Full gameplay and performance acceptance remain pending.

This build adds bounded two-way 10th Street traffic, road join repairs and a cycling signal at the mapped Piedmont bike crossing. Cars brake for obstructions, queue at the signal and recycle only when out of sight and away from the rider. Direct bike-to-car and unavoidable moving-car contact trigger existing traffic recovery.

Ride along 10th Street and inspect both traffic directions. Approach the signal, check whether its state is readable from the normal riding camera, and confirm cars stop without entering the occupied crossing. Compare a distant obstruction (car should brake) with a late cut-in (impact should interrupt riding). Check that traffic does not visibly appear or disappear nearby.

Still incomplete: cars currently share bright orange paint, crash recovery is not realistic rider ejection, moving cars do not yet have a finished on-foot injury response, and routes stop before Monroe traversal. Monroe and Krog traffic intersections, occasional scooter-wreck scene, Peachtree race dressing and full performance/visual polish are pending. This is an intermediate playtest, not completion of the game.
