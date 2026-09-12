# Mac playtest 053

2026-09-12 [codex-maclaptop]

Candidate: moving-character clarity and physical zombie falls.

- Motion blur disabled; temporal antialiasing retained.
- Punk/vendor zombie deaths now use their own physics assets and current pose.
- Root-parented visible shoes follow physical shins; fitted shoe collision boxes supply ground support.
- Moderate damping reduces prolonged limb motion. Existing corpse lifetime/drop/score rules retained.

Native slope tests found and corrected deep shoe penetration; final native contact checks pass. Packaged contact, gameplay and install checks pending. Full-route balance, scenery obstacles, multi-corpse performance and overall requested game quality remain unfinished.

2026-09-12 [codex-maclaptop] Installed 0.53.0, strict ad-hoc signature and desktop link verified; backup052 retained. Packaged punk/vendor slope contact checks pass with minimum skin clearance +0.92/+0.18 cm and no vertices more than 1 cm below terrain; selected settled frames inspected. Corrected old combat fixture to skip tutorial, wait for physical recovery, draw the holstered weapon, and separate the headshot target from the previous corpse. Packaged combat passes navigation, attack warning/damage/wipeout, actual pistol body/head hits, rewards and corpse cleanup. Installed health/cardinal phone hunt/death resets/checkpoints pass. These fixtures do not establish full-route balance, arbitrary obstacle contacts, animation quality, or game completion.
