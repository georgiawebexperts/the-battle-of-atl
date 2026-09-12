# Mac playtest051 — 2026-09-12 [codex-maclaptop]

Candidate, not installed yet. Ten farmers-market stalls near12th Street; closure directs practice riding to14th and clears once countdown starts. Both street approaches retained, extended practice branch reaches market. Farmers Market Vendor zombies confined to park; punk variants unrestricted. Weathered asphalt improves park-path readability. Build050 physical crash/recovery changes retained.

Required before installation: successful Mac cook/package, cooked tutorial entry/closure removal, cooked vendor region test if supported, main health/quest reset check and rendered market review. Preserve installed050 as backup. Full art, movement, combat and world acceptance remain incomplete; this is another development playtest.

Packaging exits0 (147.76seconds). Cooked vendor containment/pursuit and health/phone/checkpoint tests pass. Cooked direct tutorial route fails at(-17915.681,-4911.675,224.758), segment69, distance7235.3cm/error200.3cm; fence237 sweeps pass but gate is not reached. Do not promote. Diagnose cooked/editor route difference without loosening the200cm threshold. No installed app replaced.

Diagnostic rebuild exits0. Packaged failure reproduces atdistance7247.8cm/error205cm. Trace shows fixed .0167s ticks,700cm/s, steering overshoot oscillation through bend; no TutorialImpact records. Floor remains BattleTutorial. Failure is not shown to be collision with market fence. Next compare editor timing/trace and assess driver speed/lookahead versus actual steering response. Do not loosen route acceptance.

Editor trace comparison: uncapped editor runs approximately0.002s ticks and passes; explicit60fps reproduces packaged failure (204cm deviation). Changed audit driver only: speed-scaled lookahead170+Speed*.125cm and .1s prediction of eased steering yaw. Real W/A/D inputs retained,700cm/s gear2 retained,200cm route tolerance unchanged. Isolated fence rays now ignore pawns after one pedestrian occluded a ray; actual driving crowd collision unchanged. Editor60fps now passes7707.4cm/maxerror25.5cm, fence checks and countdown/removal. Rebuild/retest packaged candidate next; installed050 untouched.
