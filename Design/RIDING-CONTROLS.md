# Riding controls — build 033

2026-09-11 [codex-maclaptop]

W/Up pedal, S/Down brake, A/Left and D/Right steer. Space remains brake/drift. Q shifts down and R up while riding; R still reloads on foot. The in-game HUD and Instructions explain the new bindings.

Digital steering now feeds an exponential response at rate8 per second. Heading, front wheel and bank use that smoothed value; lean has a second rate10 response. Horizontal velocity follows the requested direction exponentially as well. Steering no longer pivots the bicycle while stationary; low-speed yaw builds with speed. Dismount/checkpoint recovery clear residual steering and lean. These improve the existing arcade handling; the requested selectable realistic physics mode is still unfinished.

Native SteeringAudit exercises actual player-controller key events and world movement: Up pedal without gear changes, Down/S braking, W acceleration, right/left reversal without snapping, lean and heading changes, Q/R gears, remount reset and stationary steering. A separate mathematical check compares only the steering response at30/120Hz. Full physical input, frame-rate behavior and game-feel acceptance still require playtesting.
