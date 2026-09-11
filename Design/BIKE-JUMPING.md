# Bike jumping — build 034

2026-09-11 [codex-maclaptop]

J hops while riding at least 500 cm/s (use second gear or higher). W/Up pedal, S/Down and Space brake, Q/R shift. The jump sets upward velocity to 650 cm/s and uses CharacterMovement falling, gravity and collision. No jump from stationary, in midair, parked, stunned, recovering, dead, countdown, paused or ended runs.

A clean landing awards +10 seconds once after at least 0.25 seconds airborne and 65 cm upward clearance, with sufficient takeoff speed. Natural elevated takeoffs can qualify too. Water, dismounts, stuns and wipeouts do not qualify. This avoids awarding time for tiny pavement seams or repeated midair presses. Timer notices use the existing AIRTIME display. Dedicated rider takeoff/landing animation and authored ramps/skatepark remain unfinished.

Native audit exercises actual J input and world movement, clearance, one landing award and stationary/midair/stun/end guards. Controlled test coverage is not final game-feel, ramp or animation acceptance.
