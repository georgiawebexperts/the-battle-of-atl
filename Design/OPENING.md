# Opening sequence

2026-09-11 [codex-maclaptop]

The Battle of ATL opens a new ride with a twelve-second in-world camera move around Ellison at the 13th Street bungalow. The title and three readable story captions introduce the phone dropped after frisbee, the watch signal in Piedmont Park, and Morgan waiting at 98 Estoria. The watch card is a story UI element, not a claim that a physical watch prop has been animated. Current background/character geometry remains unfinished.

The world is paused for the sequence. The game HUD is hidden. Movement input is flushed and ignored; no practice time or position is consumed. The normal player camera, HUD and controls return when the sequence finishes. Enter, Escape or the visible skip button return immediately to untimed practice. It plays once per new ride, not every time the pause menu resumes. Timer activation remains exclusively at the 14th Street gate.

Implementation: BattleOpening.cpp with controller lifecycle/input hooks. A real-time ticker animates the camera while the world is paused; its weak controller reference and active flag stop work after skip or level teardown. Development audit/review flags normally skip this opener so prior fixtures keep their intended scope; BattleOpeningAudit and BattleOpeningReview exercise it explicitly. Native verification is pending at this entry: full duration, actual Enter skip, player/timer protection, restoration and three 720p story-frame captures.

This is an in-world opener with camera motion and story UI. It is not a completed voiced film, animated watch performance, final character animation or final Midtown streetscape.

2026-09-11 [codex-maclaptop]: First native full-duration and Enter-skip tests passed timer/location/control protection, but rendered frames exposed a stationary cached camera and default rider pose. Added explicit paused-world camera updates with prior camera-mobility restoration, and refreshed the riding pose before pausing. Strengthened acceptance to measure the actual player view movement, prevent replay on resume, and test Escape as well. Fresh build/captures pending; initial still-looking render is not accepted as camera motion.

2026-09-11 [codex-maclaptop]: Corrected full-duration, actual Enter-skip and Escape-skip audits pass, including measured actual camera movement, restored controls/view, unchanged player position/timer and no replay. Three rendered 720p frames now show the camera moving and the rider posed on the bike with readable story UI. Remaining art/audio limitations above still apply.
