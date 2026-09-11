# Park benches — build 037

2026-09-11 [codex-maclaptop]

References describe wooden park benches and swing benches around Lake Clara Meer: https://www.atlantaareaparks.com/parks/piedmont-park/ and https://piedmontpark.org/home/img_4770/ . These inform a slatted timber bench with dark tubular supports and arms; no reference photograph is used as a game texture. This is authored park seating, not a surveyed replica of every real bench or a finished swing bench.

Runtime placement samples the existing eligible park path splines and shuffles candidates with a fixed seed. Up to48 benches are placed outside the path width, checking all nearby paths, four-corner dry grass support, surface slope/height variation, solid-object clearance, water and spacing. Paths and terrain are not edited. Seat/back wood and frames share instanced mesh components with collision to limit draw overhead.

Native audit counts installed benches and tests physical sweep blocking on each. Native camera review checks one installed bench. Final geographic placement, surface texture quality and seated people remain open; this is not final landscape acceptance.
