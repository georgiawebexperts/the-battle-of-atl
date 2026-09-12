# Mac playtest 053

2026-09-12 [codex-maclaptop]

Candidate: moving-character clarity and physical zombie falls.

- Motion blur disabled; temporal antialiasing retained.
- Punk/vendor zombie deaths now use their own physics assets and current pose.
- Root-parented visible shoes follow physical shins; fitted shoe collision boxes supply ground support.
- Moderate damping reduces prolonged limb motion. Existing corpse lifetime/drop/score rules retained.

Native slope tests found and corrected deep shoe penetration; final native contact checks pass. Packaged contact, gameplay and install checks pending. Full-route balance, scenery obstacles, multi-corpse performance and overall requested game quality remain unfinished.

2026-09-12 [codex-maclaptop] Installed 0.53.0, strict ad-hoc signature and desktop link verified; backup052 retained. Packaged punk/vendor slope contact checks pass with minimum skin clearance +0.92/+0.18 cm and no vertices more than 1 cm below terrain; selected settled frames inspected. Corrected old combat fixture to skip tutorial, wait for physical recovery, draw the holstered weapon, and separate the headshot target from the previous corpse. Packaged combat passes navigation, attack warning/damage/wipeout, actual pistol body/head hits, rewards and corpse cleanup. Installed health/cardinal phone hunt/death resets/checkpoints pass. These fixtures do not establish full-route balance, arbitrary obstacle contacts, animation quality, or game completion.

2026-09-12 [codex-maclaptop] Post-install source work: mounted first knife hit now invokes physical wipeout; attacker stops during recovery and retains a 1.5-second reaction window afterward. Native editor build and Easy/Hard rendered knife fixtures pass first fall, recovery/remount escape, lethal second strike, protection/cover and actual gun defense. First-stab image inspected, character similarity and knife pose still rough. Not included in installed053. Existing local knife audit improvements preserved; recovery assertions extended to physical fall.

2026-09-12 [codex-maclaptop] Source-only crash HUD: distinguish settling fall from active get-up using actual recovery pose; explain automatic recovery and running clock, suppress reticle/target/hit labels while control is unavailable, remove misleading short stun countdown during the longer physical recovery, suppress knife escape instructions until control returns. Native compile and rendered Hard knife sequence pass. First-fall frame reviewed at1280x720: bottom explanation fits and reticle absent; upper status phrase crowded panel, shortened to Recovering afterward and rebuilt successfully. Final shortened label not recaptured yet. Installed053 unchanged.

2026-09-12 [codex-maclaptop] Source-only knife appearance: dedicated rust top/shoes, charcoal shorts and dark hair material overrides, preserving shared player asset. Generator Scripts/create_knife_appearance.py and three materials retained. Native compile and Hard rendered encounter pass; first-stab frame confirms distinct palette and final shortened Recovering HUD label fits at1280x720. Model silhouette/attack pose still rough; installed053 unchanged.
