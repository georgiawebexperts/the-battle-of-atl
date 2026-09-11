# Full-game acceptance ledger

This ledger supplements V2-SPEC.md and WORLD-EXPANSION.md; it does not narrow them. Last reviewed 2026-09-10 [codex-maclaptop]. No full-game acceptance has been granted.

| Requirement | Current evidence / remaining work |
|---|---|
| E-bike, visible rider, seven gears, WASD/arrows, brakes, lean, slope/grass/collision | Native prototype and lab checks exist. Production rider animation, convincing crashes/ragdolls and final bike quality remain incomplete. |
| Independent walking/swimming rider; bike stays at lake entry | Actual lake integration passes swim/return/remount. Surface swimming is provisional; complete shoreline and island exploration remain to verify. |
| USGS terrain and full OSM park path web | Measured landscape, 236 editable path pieces, baked pavement and two public plazas installed. Path-only Recast audit reaches 675/708 sampled points (225/236 pieces), with no search-limit failures; 11 pieces retain isolated samples. Full junction connectivity, boundaries and every path's end-to-end ride are not accepted. |
| All mapped bridge ways | All 14 OSM bridge ways have installed deck coverage and scoped live riding evidence, including the wood spur and transverse Park Drive movement. Final-five, lake, underpass and bike regressions pass. Full network/junction acceptance and final architecture remain incomplete. |
| Park identity | Lake shape exists. Boathouse/dock, aquatic center, tennis courts, Active Oval, Meadow/Oak Hill, Promenade/Legacy Fountain, 12th/14th/Charles Allen gates, Magnolia Hall, Greystone, community center, Noguchi Playscape, dog park, bocce, garden/greenhouse and Botanical Garden boundary/canopy walk still need faithful construction/verification. No supplied reference photos were actually available in the attachment set. |
| Skyline and living landscape | Recognizable Midtown buildings, species-appropriate trees, grass, matched benches/fences/lights, and perimeter street activity remain incomplete. Quixel/PCG visual requirements remain unaccepted. |
| Full BeltLine to Krog | Source OSM data exists. Connector, Virginia/ Kanuga sections, 725 Ponce Kroger/plaza, PCM, Fourth Ward Park/pond, Ralph McGill/Freedom crossing, Krog Market and traversable graffiti tunnel must still be built/verified in correct order. |
| Whole game loop | Prototype countdown/timer/item/cardinal hints/death/restart exist. Real 12th Street start, menu, difficulty selection, 15/10/5 minute settings, off-direct-exit item constraint, guaranteed reachable random placement, Kroger checkpoint/bonus, finish, results and persisted best times remain incomplete. |
| Route guidance | Cardinal-only item hints pass checks. Post-pickup route guidance/minimap, UI polish and full objective completion flow remain incomplete. |
| Traffic everywhere | Walkers/groups/stopping, joggers, ordinary bikes, three scooter behaviors, occasional 50 mph hyperbikes, rollerskaters, dog/owner/leash groups and loiterers remain incomplete. Density/difficulty scaling, reactions and crash penalties still need verification. |
| Rare combat with continuous clock | Prototype rare encounters, first-stab dismount/chase, second-stab death before remount, fatal bullets, on-foot-only gun defense, blood particles and item reset have live/unit integration evidence. Full-world NPC navigation, production shooting/hit/death effects and convincing animation are not accepted. |
| Horn and automatic lights | Lab checks pass. Real tunnel darkness volume and lighting quality still need placement/verification. |
| Beyond Krog | Tunnel traversal and provision for transition into a future different game area remain incomplete. |
| Presentation and performance | Audio layers, final lighting/post-process, polished HUD/menu, streaming, packaged build, credit/OSM attribution and 60 fps at 1080p acceptance remain incomplete. |
| Hard-mode balance | Must demonstrate a clean full run with correct gears and little margin. No narrow physics or timer test proves this requirement. |

Tests/Results contains scoped evidence only. Import bounds and sampled collision checks do not establish complete navigation, visual fidelity or performance. Do not treat this ledger's existence, generated geometry, or green subset tests as completion of the finish-game goal.
