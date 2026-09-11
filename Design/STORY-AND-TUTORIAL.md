# The Battle of ATL

2026-09-11 [codex-maclaptop]

Latest user-approved title: **The Battle of ATL**. Supersedes Battle for the ATL and the briefly considered 2026 suffix. Player: Ellison. Girlfriend: Morgan.

Ellison dropped his cell phone while playing frisbee in Piedmont Park. Find My Lost Phone on his watch gives broad cardinal guidance. After recovering it, he travels the established route through Krog Tunnel and turns right to meet Morgan for a party and beers at fictional **98 Estoria**. The real reference is 97 Estoria, 727 Wylie Street, across from the tunnel. The previous home finish is superseded; reuse the bungalow for the start.

Start near a fictional bungalow off 13th Street, with free practice and contextual instructional popups. Ride to the 14th Street stone gateway to begin the timer. Include an opening cinematic, believable Midtown buildings, a park-side restaurant named Billy’s, a nearby corner bar and condos. Streets beyond the playable boundary should announce a future Midtown expansion. Those landmarks still need location verification and construction.

Implementation status: title changed in project configuration, menu and Mac packaging script. Installed build remains 041 until the full next build is verified and installed. Uncommitted 042 work contains the bar route, story text, imported original photo-based celebration art and partial tutorial implementation. It is not ready to package: tutorial mode integration, introductory sequence, help, boundary handling, Midtown surroundings and native verification are still pending. Do not describe generated celebration artwork as live character animation or the bar blockout as final realism.

2026-09-11 [codex-maclaptop] User confirmed Billy’s as the fictional restaurant name. Practice streets must permit travel along 13th and turns both ways beside the park, leading to the 14th Street stone gateway; avoid forcing one narrow prescribed route. Additional street branches remain to be built.

2026-09-11 [codex-maclaptop] Visually inspected OpenStreetMap at zoom17, 33.7857,-84.3807. Confirmed the two practice approaches: 13th east to Piedmont Avenue, or 13th west to Juniper then 14th east to the same mapped gate. Retained complete local OSM XML via official API in References/tutorial-block.osm after Overpass queries failed. prepare_tutorial_block.py derives the 25406-cm alternate route and six expansion closures. The game permits both practice directions; this is not a real-road cycling navigation recommendation (Juniper has one-way restrictions).

2026-09-11 [codex-maclaptop] Tutorial is integrated with a stopped clock, protected health/time, practice hints and F1 visibility, cardinal/map guidance to the gateway, and a once-only gateway start/countdown. Added construction fences with visible collision bars, UNDER CONSTRUCTION signs and a coming-soon message. Removed the previous distance-triggered teleport. Native two-route audit added; build and tests pending at this note. Intro cinematic, Billy’s, corner bar/condos and detailed streetscape still remain.

2026-09-11 [codex-maclaptop]: Both routes now have native driving evidence: direct 7899.3 cm/max error 69.8 cm; Juniper alternate 25370.7 cm/max error 177.4 cm, including untimed protection, actual dismount/remount, physical construction fence sweeps and one-time gate start. Replaced vertex-dependent smoothing with distance-based intersection curves. First 720p native tutorial capture exposed overflowing help heading and mounted timer caption; shortened the heading and reduced the caption size. Fresh visual acceptance pending. Wider streetscape remains sparse and unfinished; these tests do not prove Midtown appearance or complete expansion boundaries off-road.
