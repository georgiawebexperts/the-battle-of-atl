# BATTLE FOR THE A — V3 acceptance ledger

> ## STATUS BANNER — read this first, added 2026-09-19 [codex-maclaptop]
>
> **This ledger is historical.** It was reviewed on 2026-09-10 against the V3
> spec, and the project has moved far past every gate it describes: the current
> build is **BUILD 146 / 0.146.0-dev**, shipped to a Desktop playtest shortcut
> and swept 30 of 30 on the cooked app. The gates below were never formally
> re-adjudicated, so their wording is the *original* requirement set rather than
> a statement of what is now true.
>
> Live status lives in `Brain/Battle of ATL.md` (Elliott's Brain vault) with the
> per-block evidence in `Brain/History/*.md` and `Tests/Results/` in this repo.
> The one gate in here that has been continuously maintained as a real gate is
> exit criteria expressed as audits: `Scripts/run_quick_audits.sh` is the list,
> and a build only replaces the Desktop shortcut when all 30 pass.
>
> Do not treat the "Incomplete" cells below as an open task list without
> checking them against the vault first — several describe work that has since
> shipped under a different name (the Krog tunnel, the Kroger plaza, the run's
> finish and saved records, the spirit encounter).

Active spec: V3-SPEC.md plus V3-DECISIONS.md (native Mac delivery overrides Windows packaging). Reviewed 2026-09-10 [codex-maclaptop]. No V3 milestone or full-game acceptance yet. Old evidence is scoped/historical, not V3 acceptance.

| Gate | Required evidence / current state |
|---|---|
| 1: Fun arcade bike/FPS test map | Character Movement bike stays upright at full speed on hills, grass, roots, curbs and stairs; five gears, snappy steering, cosmetic lean, drift, grass -25%, wall bounce, glancing nudges, direct-hit two-second wipeout, two-second water return, 50 moving NPCs. E switches to FPS and back; pistol works. New Character Movement bike and isolated ArcadeBikeLab pass 14 live foundation checks (build 005). FPS dismount/remount and pistol foundation pass 14 additional live checks (build 006). Build 007 adds bike fire, shared muzzle/tracer/hit feedback and near-miss/kill nitro with 11 action checks plus FPS/terrain regressions. Build 008 adds H horn/automatic lights and 14 passing checks for curb/glancing/blocked-exit and lighting/horn behavior, plus 14 updated-course regressions. Build 009 adds visible skinned FPS arms, recoil/reload motion and eight passing integration checks. Build 010 adds automatic fast-turn drift, pooled skid/splash effects, terrain bob/camera/impact feedback, six ride sounds and repaired visible lab water, with 11 feedback and 14 terrain checks passing. Final presentation/audio mix and actual fun review remain; this gate is not accepted. |
| Rider and controls | Rigged pedaling/cosmetic lean/wipeout, Braves/Falcons/Hawks outfit selection; W pedal, arrows gears/steer, Space brake/drift, E, Tab, Esc, Shift boost with near-miss/kill refill. FPS WASD/sprint/jump/mouse/aim/reload/1–4/F melee. Incomplete. |
| 2: Geographic park | Real elevation and sourced OSM paths provide groundwork. Build 011 installs V3 bike/FPS mode on PiedmontWorld and a sourced, runtime-verified start just inside the mapped 14th Street gate. Thirty directional runs across the fourteen sourced bridge ways, wood spur and cross-deck route pass; park water/FPS integration passes. The actual stone gate and landmark art remain unfinished. 14th Street stone gate and correct facing/path split; lake Water Body, boathouse/dock, Aquatic Center, tennis, Active Oval, Meadow/Oak Hill, Promenade/Legacy Fountain, other gates, Magnolia Hall/Greystone/Playscape/dog park/bocce/garden/Botanical boundary/canopy walk and skyline must match references. Photos are currently missing. |
| Foliage | Required oak/magnolia/hardwood assets and PCG tree lines/canopy, Nanite lawn grass, shrubs/flower beds/leaves; at least 30 trees in park screenshot. Incomplete. |
| 3: Park life | Every V3 group: frisbee/football with flying props, blankets/phones/eating, drum circle/DJ/dancers, guitarist/saxophone, leashed and dog-park/crossing dogs, children/playground, oval runners/tennis/pool sound, trick skaters, vendors, path crowds. All animated, audible and collidable. Initial 24 local walkers/joggers/pairs with horn/physical contacts exist; remaining life and density/performance are incomplete. |
| 4: Full route | 10th/Monroe connector, apartments/Kanuga, 725 Ponce Murder Kroger checkpoint and dangerous plaza, PCM/Ponce bridge, Fourth Ward Park, Ralph McGill/Freedom, Krog Market/Irwin checkpoint, fully traversable graffiti tunnel, Cabbagetown houses/murals/Stacks and named-mailbox bungalow finish. Incomplete. |
| 5: Weapons and combat | FPS pistol/infinite ammo, shotgun/knockback, SMG, ricocheting multi-target frisbee launcher, chained U-lock; glowing crates/enemy drops, muzzle/tracer/hit feedback/headshots/ammo HUD, bike pistol with speed inaccuracy. Health/regen/Coca-Cola and checkpoint respawn -10 sec. Existing single gun is only groundwork. |
| Enemies | Cartoon nonhuman zombies/shamblers/sprinters, 30+ voiced/subtitled jokes, telegraphed charges/rear-wheel grab/headshot/dissolve/waves; rooftop/bridge/car shooters with laser/shout and required Kroger counts; hostile illegal e-bikers with speakers/weaving/ramming/Hard guns; three scooter types/packs; Kroger knife behavior by difficulty. Existing attacker pursuit is groundwork, not acceptance. |
| 6: Complete front end/loop | Title/logo/golden-hour skyline pan/original music; working Start/Instructions/Level Select/Options/Quit; keyboard diagrams/enemy roster/difficulty table/pause instructions; difficulty cards/best times/outfits; 14th-gate countdown/Easy contextual tutorial; artifact retrieval, checkpoints/home finish, win/loss/retry/grade/stats and persisted bests. Incomplete. |
| Radar and HUD | Circular path map/player triangle/parked bike/enemy dots, ranged pulsing artifact blip/rim arrow; post-pickup gold route/checkpoints. Timer red under one minute, health/nitro, gear/speed or weapon/ammo/crosshair, objective/subtitles. Incomplete. |
| 7: Difficulty/audio/performance | Data table controls every V3 difficulty-table variable, including 15/10/5 minutes and radar range. Full spatial park/traffic/combat/tunnel audio and original dynamic soundtrack. World Partition park/BeltLine/Cabbagetown, high-density crowd solution, 60 fps at 1080p on target Mac evidence. Incomplete. |
| 8: Mac delivery | Native shipping BattleForTheA.app with custom icon and desktop shortcut. Double click launches title screen, all menus and game work without editor. Incomplete. |
| Final play acceptance | Full run of each difficulty; geography/reference screenshots, Meadow activity, zombie combat/subtitles, radar retrieval-to-home flow, terrain-proof riding/FPS/remount and standalone launch verified. Actual fun/playability feedback required; subset tests do not prove this. |

Increment the project build version at each saved milestone. Preserve the complete scope and the build order.


## Build 012 — 0.12.0-dev — 2026-09-11 [codex-maclaptop]

Created a native arm64 Development app on Adam Assets and a desktop symlink named Battle for the A.app. Added a standalone launch menu, Start Park Ride, Instructions, graphics presets, Quit, and Escape pause/resume. Editor PIE remains immediately playable. The menu explicitly identifies the unfinished park playtest; this is not the complete V3 front end or campaign. Default Mac startup is PiedmontWorld, with 1080p windowed settings and a 60 FPS cap, not measured 60 FPS acceptance.

Three packaging issues were diagnosed and fixed: missing editor GameFeatureData asset-manager cook rule; UAT archive copying the executable-only bundle rather than the complete staged app; and a standalone startup crash in UConversationRegistry/UGameFeaturesSubsystem, pulled in by Aura's AllToolsets dependencies. Aura now has TargetAllowList Editor, preserving development use while excluding its dependency chain from the Game target. Finalization checks for cooked IoStore content, copies the complete staged app, applies an original icon, signs locally, and installs the desktop link. Config/DefaultEngine.ini remains untouched/untracked; AndroidFileServer settings are denied from staged config.

Native Game/Editor builds and BuildCookRun pass. The packaged app passes display-free startup: loads PiedmontWorld, selects BattleParkMode, initializes the Home menu, and exits 0 with no errors. Codesign verification passes. The first rendered launch exposed the now-fixed plugin crash; the Mac locked before visual retesting. Manual unlock requested. Start/instructions/options/pause/quit, packaged movement/dismount/fire/remount, audio and performance remain unverified in the corrected rendered app. Report: Tests/Results/2026-09-11-v3-mac-package.json.

Final app: /Volumes/Adam Assets/Unreal/Builds/BattleForTheA/Mac/BattleForTheA.app (~917 MiB). Full V3 world, foliage/life, route, enemies/weapons, retrieval/checkpoints/radar/difficulty/saves/audio/performance remain unfinished. Goal stays active.


## Build 013 — 0.13.0-dev — 2026-09-11 [codex-maclaptop]

Added reflected FBattleDifficultyRow and the actual cooked DT_Difficulty asset, imported from SourceAssets/Data/Difficulty.csv. Easy/Medium/Hard set 900/600/300 seconds and 20/50/90 nearby walking/jogging visitors; the director reads the configured jogger ratio. The isolated lab remains at 50 visitors. Table fields also preserve counts/speeds/fractions for the remaining V3 NPC/enemy/weapon/radar consumers, which are not implemented merely by defining their values. SourceAssets/Data/README.md identifies active versus pending consumers.

The standalone menu now offers Level Select with row-derived time/warning labels. Selection opens a fresh park with the chosen profile. The HUD shows countdown/timer in both bike and FPS modes, with red timer below a minute. Timeout pauses into THE A WINS THIS TIME with Retry, Level Select and Quit; Escape cannot bypass an expired run. Actual rendered menu navigation and Retry reload acceptance remain pending because the Mac is locked.

Native editor build, Data Table import/save and Mac BuildCookRun pass. Scripts/test_mac_difficulty.py runs the real cooked executable with an opt-in development-only headless probe. Each profile passes eight checks: timer ticking after countdown, actual live/desired crowd count, dismount, pistol ammo consumption, remount, pause, resume and timeout-to-menu. All three processes exit 0; results in Tests/Results/2026-09-11-v3-native-difficulty.json. The first report collector looked for a file redirected by Unreal's packaged file sandbox; corrected collection to the explicit structured native log record and reran all three successfully. Tests invoke gameplay methods, not physical keyboard/mouse events, and do not establish rendered UI, audio or performance.

Updated complete, locally signed desktop app to 0.13.0; original build 012 retained under Builds/BattleForTheA/Mac/Previous. Default branded-app headless startup selects Easy, initializes Home and exits cleanly. Source commit and user guide preserve remaining scope. Full V3 game is not complete; goal stays active. Native visual/control test still awaits unlock, while other implementation can proceed.


## Build 014 — 0.14.0-dev — 2026-09-11 [codex-maclaptop]

Added ABattleQuest and a floating, rotating, glowing lost-phone Artifact. The current park supplies 446 potential sampled path positions before filtering. Placement excludes the first 5000 game cm around the start and a 1200 cm corridor along the direct start-to-exit line; requires a paved hit, dry bike footing, clear capsule space, and nonpartial navigation both from start and toward the exit waypoint. Pickup requires proximity plus clear line of sight, works on bike or foot, sets shared item state and removes the phone. No abbreviated win condition was added.

Added a north-up circular radar with 1176 cached resampled source-path segments, analytic circle clipping, player heading, parked-bike marker, hostile-dot input, and the Artifact's pulsing gold blip only within the difficulty's radar range. Outside range a rim arrow shows direction. After pickup, a gold navigation path and target arrow point toward the park's BeltLine side; route refreshes every two seconds. RadarRange is now an active Data Table consumer. Actual rendering, hostile dots and visual/pulse readability remain unverified while the Mac is locked.

The first return waypoint is the nearest currently installed centerline to mapped park-side connector node 5674504871 (way 741964055). It lies on way 182302109 at XY 11322.24111595, -8938.93870108. The mapped connector is still about 66 real metres away: this is not the completed 10th/Monroe connection or the full home route. Scripts/prepare_battle_exit.py and SourceAssets/Terrain/battle-park-exit.json preserve that projection and limitation. Continue the actual connector/BeltLine/Cabbagetown work; do not present reaching this waypoint as winning.

Native Game/Editor compilation and Mac packaging pass. Expanded real packaged headless checks pass in Easy, Medium and Hard: previous timer/crowd/dismount/fire/remount/pause/resume/timeout checks, quest readiness, range gating, placement exclusion, segment clipping, pickup, route generation, and rejection of pickup through a temporary blocking wall. Medium explicitly collects on foot; Easy and Hard collect on the bike. Tests teleport for the collection fixtures and do not prove physical keyboard input, full route ride-through, visual rendering, final audio or performance. Evidence: Tests/Results/2026-09-11-v3-native-quest.json. Fixed native pointer/Canvas access compile errors before passing. No failing build is installed.

Installed complete signed desktop app 0.14.0, retained build 013 under Previous, and confirmed normal branded headless Home startup with a ready Artifact and clean exit. Cleaned two known failed temporary app copies. Mac remains locked on recheck; visual acceptance still awaits unlock. Full V3 world/foliage/life, complete route, enemies/weapons, health/checkpoints, saves/results/outfits/audio/performance/World Partition and final shipping acceptance remain required. Goal stays active.

2026-09-14 [codex-maclaptop] — Native continuous movement evidence: Tests/Results/2026-09-14-full-ride-diagnostics.json passes one park-start → phone → MurderK → KrogStreetMarket → tunnel → 98Estoria win run, no teleport,494.03s/195440.06cm. Combat/crowd removed, road traffic retained; tutorial skipped. This supplements earlier teleport-based objective checks and does not accept the full game, all routes/phone sites, visuals, combat or fun. Earlier fullattempt stalled before KrogMarket; retained as unresolved intermittent issue.


## Current status — Build 099 — 2026-09-17 [codex-maclaptop]

This ledger's per-gate rows above still describe the build-014 era. The current
state, in one place, so the remaining work is not read off stale notes:

**Built and passing today (native Development and signed share build 099)**

- Bike/FPS modes, gear and steering model, drift, nitro, horn, automatic lights,
  camera modes, dismount/remount, swimming with stroke animation, crash and
  taser recovery presentation, health/checkpoints/respawn penalty.
- Quest loop: randomized phone placement, watch proximity signal, radar with
  range gating and rim arrow, post-pickup route, Murder K, Krog Market, tunnel,
  98 Estoria finish, win/loss and level select, best-time records.
- Weapons: pistol, shotgun, SMG, rifle with zoom, ricocheting frisbee launcher,
  U-lock melee, crates, enemy drops, ammo supplies, reload/zoom presentation.
- Enemies and hazards: zombie waves by difficulty, sleepers and bench fires,
  police taser telegraph with dodge window, drones, knife encounters, scooters
  and road traffic, spare bikes, boost pickups.
- Mac delivery: branded 0.99.0 app, sandbox-only entitlements, icon set, BOA
  desktop shortcut, focus-loss pause, resolution presets, quit, App Store
  guardrail audit.
- Player-facing aim sensitivity (25%-200%, Options or `[` / `]`) with a passing
  scale/persistence audit, and held instruments for the park musicians.

**Named requirements that are still missing (unchanged scope)**

- Landmarks: no boathouse/dock, fountain, promenade, tennis courts, Magnolia
  Hall/Greystone/Playscape, dog park, bocce, garden, Aquatic Center, Active Oval
  or skyline geometry exists in code or content today. Gate 2 cannot be accepted
  until these exist and match references; reference photos are still missing.
- Foliage: Epic European hornbeam and HillTree canopy stand-ins are in the park;
  the specified oak/magnolia/hardwood species and photo matching are not.
- Park life gaps: leashed/crossing dogs, children and playground, football, oval
  runners, tennis/pool ambience and food vendors do not exist yet. Walkers,
  joggers, pairs, dancers, musicians, picnics and skaters do.
- Front end: title art, level select, instructions, options and celebration
  exist; a full title/logo/golden-hour pan and original dynamic soundtrack do
  not.
- Measured performance: rendered 60 fps at 1080p on the target Mac is not yet
  established for the current build.
- Final play acceptance per difficulty, and the Apple-side steps (review contact
  phone, price/release, license confirmations, notarization) remain with
  Elliott.

Two audits are worth knowing before they mislead anyone: `-BattleMusicAudit`
fails when the run passes `-nosound`, because no audio device ever reaches
Playing. Without that flag it passes (`{"passed":true,"tracks":2,...}`,
2026-09-17).
