# The Battle of ATL — App Store and TestFlight metadata draft

Last updated 2026-09-16 [codex-maclaptop]

This is a working submission draft for the macOS app record. It reflects Build 096 (`0.96.0`, build `96`) and must be checked against the exact uploaded binary before submission. Text marked **REQUIRED** needs an owner decision or final account detail. Do not paste internal notes, development caveats, or the hidden Murder K/ghost encounter into public marketing copy.

## Release identity

- App name: **The Battle of ATL**
- Developer / seller brand in copy: **Web Experts**
- Platform: **macOS**
- Bundle ID: `com.webexperts.battleofatl`
- Apple Team ID: `8HAG5A4GS7`
- App Store Connect Apple ID: `6812874355`
- Current candidate: Version `0.96.0`, build `96`
- Copyright: `© 2026 Web Experts. All rights reserved.`
- Category recommendation: **Games > Action**
- Secondary category recommendation: **Games > Racing**
- Price: **Free during TestFlight**. App Store price remains an owner decision.

## App Store copy

### Subtitle

`Race Atlanta. Find the phone.`

29 characters including spaces. App Store limit: 30.

### Promotional text

`An experimental Atlanta indie game: race through Piedmont Park and the BeltLine, recover Ellison’s lost phone, survive the city, and reach the party in time.`

157 characters including spaces. App Store limit: 170. Promotional text can be changed without a new binary.

### Description

Ellison lost his phone while playing frisbee in Piedmont Park. His watch has a signal, Morgan is waiting across town, and the clock starts when he rides through the 14th Street gate.

The Battle of ATL is an experimental indie action game from Web Experts. Ride from Midtown through Piedmont Park and along an Atlanta-inspired BeltLine route toward Krog Street Tunnel and the fictional 98 Estoria. Search the park using broad directions from Ellison’s watch, recover the phone, and finish the run before time expires.

Choose arcade or realistic bike handling. Build speed on Atlanta’s hills, jump ramps, use limited horn charges to clear a path, and decide when to leave the bike to explore on foot. Spare bikes, ammunition, weapons, bonus-time pickups, speed boosts, scooters, pedestrians, drones, police encounters, zombies, and other unpredictable hazards can change each run.

The route is built from real geographic reference data and adapted for play. Piedmont Park, the Eastside Trail, the Fourth Ward skatepark area, Krog Street Tunnel, and parts of Midtown and Cabbagetown form one connected game world. It is a fictional game environment, not a navigation guide or a complete reconstruction of Atlanta.

Features:

• A timed phone hunt with Easy, Medium, and Hard modes
• Bike and on-foot play with mouse-and-keyboard controls
• Arcade and realistic bicycle handling options
• A connected ride from Midtown to Piedmont Park, the BeltLine, Krog Tunnel, and 98 Estoria
• Weapons, limited ammunition, crowd reactions, police response, drones, and hostile encounters
• Skatepark ramps, time bonuses, speed boosts, spare bikes, music controls, and hidden events
• Fullscreen and multiple resolution options on supported Macs

The Battle of ATL is Version 1 of an evolving indie game. Web Experts plans to improve gameplay, refine the world, and expand the map based on player feedback.

Contains fictional violence and blood. Requires an Apple silicon Mac with macOS 14 or later. A mouse and keyboard are recommended.

### Keywords

`Atlanta,bike,BeltLine,Piedmont Park,indie,action,racing,open world,zombies,adventure`

84 ASCII characters including commas. App Store limit: 100 bytes.

### URLs

- Marketing URL: `https://webexperts.com/battleofatl` — live and verified HTTP 200 on 2026-09-16.
- Support URL: `https://webexperts.com/battleofatl` — use while the page contains the feedback form and basic playtest guidance.
- Company contact fallback: `https://webexperts.com/contact` — live and verified HTTP 200 on 2026-09-16.
- Privacy Policy URL: `https://webexperts.com/privacy-policy` — live and verified HTTP 200 on 2026-09-16. **REQUIRED:** confirm that this policy explicitly covers The Battle of ATL and the beta feedback form before submission.

## TestFlight beta information

### Beta description

The Battle of ATL is an experimental Mac indie game from Web Experts. Play as Ellison, recover his lost phone in Piedmont Park, and race an Atlanta-inspired BeltLine route through Krog Street Tunnel to meet Morgan at 98 Estoria. This beta focuses on bike handling, on-foot controls, combat, route readability, balance, performance, and the full start-to-finish game loop.

Build 096 includes arcade and realistic bike modes, Easy/Medium/Hard difficulty, a timed phone hunt, bike and on-foot play, multiple weapons and ammunition, spare bikes, scooters, drones, police response, crowd reactions, ramps, speed boosts, music controls, a finish sequence, and hidden encounters.

This is an unfinished indie beta. Art, animation, environment detail, balance, and performance are still being improved.

### What to test

1. Start a new ride and learn the controls in the untimed Midtown practice area.
2. Use either street approach to reach the 14th Street stone gate and confirm the timed run begins once.
3. Find the phone using the watch’s broad directional signal without an exact map pin.
4. Follow the route through the checkpoints, Krog Street Tunnel, and the 98 Estoria finish.
5. Try arcade and realistic bike handling, downhill speed, steering, braking, reversing away from obstacles, jumping, crashing, and remounting.
6. Get off the bike and test camera look, aiming, movement, sprint, jump, crouch, weapon draw/holster, firing, and reload.
7. Report HUD text that overlaps, clips, fades, or feels too crowded at any supported resolution.
8. Report freezes, crashes, routes that trap the bike, unreachable objectives, missing finish behavior, severe frame-rate drops, or audio problems.

Please include the build number, Mac model, Apple chip, memory, macOS version, selected difficulty, and the action immediately before the problem. Short screen recordings are especially useful for steering, aiming, animation, collision, and camera issues. Do not include passwords or private information in feedback.

### Beta App Review notes

The game is a standalone offline macOS title and does not require an account, login, subscription, network connection, microphone, camera, location service, or external hardware. It does not contain in-app purchases.

Recommended review path:

1. Launch the app and select **Start Park Ride**.
2. The opening sequence can be allowed to finish or skipped with Enter/Escape.
3. The Midtown practice area is untimed. Ride to the 14th Street stone gate; crossing it starts the timed phone hunt.
4. Follow the watch’s cardinal direction and flashing proximity cue to find the phone in Piedmont Park.
5. After collection, follow the gold route through the required checkpoints and Krog Street Tunnel to the 98 Estoria finish area.

Primary controls:

- `W` / Up: pedal; `S` / Down: brake and reverse; `A` / `D` or Left / Right: steer
- `J`: jump; Left Shift: boost; `H`: horn; `E`: dismount/remount; `P`: switch bike physics; `Tab`: bike camera
- On foot, mouse turns/aims; WASD/arrows move; Shift runs; Space jumps; `C`/Control crouches
- `G`: draw/holster; right mouse: aim; left mouse: fire; `R`: reload; number keys select acquired weapons
- `F1`: show/hide expanded control help; `M`: cycle music; Escape: pause menu

The app writes saves and settings through Unreal’s standard macOS user directories inside the App Sandbox container. Development audits, automated test drivers, debug menus, cheat keys, and console paths are excluded from Shipping builds. Only the production `PiedmontWorld` map is cooked for release.

The game includes stylized fictional gunfire, melee attacks, blood effects, zombies, hostile characters, police tasers, bike collisions, fires, and characters falling or being knocked off a bicycle. There is no gambling, sexual content, user-generated content, chat, advertising, account creation, tracking, or online multiplayer.

### Beta review contact

- First name: **REQUIRED**
- Last name: **REQUIRED**
- Phone: **REQUIRED — use a monitored number that Apple may call during review**
- Email: `elliott@webexperts.com` **REQUIRED: confirm this is the preferred monitored review address**

## App Privacy draft

The Shipping game binary is designed for offline play and has no requested network entitlement. It does not require an account and does not intentionally collect data from inside the game.

Proposed App Privacy response for the app binary: **Data Not Collected**.

Before answering in App Store Connect, verify the exact uploaded build with a network and entitlement audit. The separate `webexperts.com/battleofatl` feedback form may collect contact details, device information, feedback, IP-derived security/rate-limit data, and reCAPTCHA signals under the website privacy policy. Website form collection does not automatically become app-binary collection, but its policy and disclosures must be accurate.

## Age-rating and content-disclosure worksheet

Answer the live App Store questionnaire from the uploaded build, not from this draft. Use the conservative choices below as the review baseline:

| App Store content area | Proposed answer | Evidence / action |
|---|---|---|
| Cartoon or Fantasy Violence | Frequent / Intense | Zombies, hostile encounters, weapons, bike impacts, drones, and stylized combat recur during play. Confirm Apple’s current frequency labels. |
| Realistic Violence | Infrequent / Mild | Human pedestrians and police can be shot or struck; police use tasers. Escalate if final presentation reads as realistic rather than stylized. |
| Prolonged Graphic or Sadistic Realistic Violence | None | No torture, dismemberment, or prolonged graphic violence is documented. Recheck final effects. |
| Horror / Fear Themes | Infrequent / Mild | Zombies, pursuit, threats, and surprise encounters may create fear. |
| Mature or Suggestive Themes | Infrequent / Mild | Urban danger, bar/party framing, homelessness-inspired fictional characters, and insults. Review final dialogue. |
| Alcohol, Tobacco, or Drug Use or References | Infrequent / Mild | The story ends at a fictional bar and refers to beer/adult beverages; no drug gameplay is documented. |
| Profanity or Crude Humor | Infrequent / Mild | Ambient insults and dark comedy may qualify. Audit every subtitle/voice line before rating. |
| Medical or Treatment Information | None | No medical advice or treatment content. |
| Sexual Content or Nudity | None | None documented. |
| Gambling / Contests | None | None documented. |
| Unrestricted Web Access | None | The game has no browser or web access. |
| User-Generated Content / Messaging | None | No accounts, chat, uploads, or player publishing. |
| Loot Boxes / In-App Purchases | None | Pickups are gameplay items and are not sold for money. |

**REQUIRED:** inspect the final Shipping build’s blood effects, corpse presentation, dialogue, music lyrics, end sequence, and every reachable encounter before locking the questionnaire. Apple calculates the final rating from the answers; do not promise a particular age rating in marketing copy.

## Export compliance

Current code and entitlements indicate no network use and no custom cryptography. If the final uploaded binary contains only encryption supplied by Apple’s operating system and ordinary engine/platform libraries, answer the App Store export-compliance questions accordingly and use Apple’s current wording.

Before submission:

- Search the final binary, linked frameworks, enabled Unreal plugins, and entitlements for networking, TLS, VPN, security, or custom cryptography functionality.
- Confirm no analytics, crash-reporting SDK, online service, authentication library, or third-party plugin added encryption after this draft.
- If any non-exempt encryption is present, stop and obtain the correct export classification/documentation rather than guessing.
- `ITSAppUsesNonExemptEncryption` should only be set after the final binary audit supports the answer.

## Screenshot plan

Use clean captures from the exact submitted Shipping build. Hide expanded help unless a shot specifically demonstrates controls. Every image should show the same current build badge and avoid unfinished geometry, debug text, test fixtures, error messages, copyrighted third-party signage, and marketing spoilers.

1. **Opening aerial / hero:** Ellison on the bike with Piedmont Park and Atlanta context visible. Caption: `Your phone is lost. Morgan is waiting. The clock is about to start.`
2. **Piedmont phone hunt:** Active riding view with readable watch direction and park scenery. Caption: `Search Piedmont Park with only a direction and a signal.`
3. **BeltLine speed:** A downhill or fast trail segment showing riders, pedestrians, and the connected route. Caption: `Build speed through an Atlanta-inspired park and BeltLine world.`
4. **Bike action:** A controlled jump at the skatepark or ramp, with clean landing space visible. Caption: `Choose your line, catch air, and earn back precious time.`
5. **On-foot combat:** Clear over-the-shoulder aiming at an unmistakable zombie or hostile drone, with no civilian in the crosshair. Caption: `Dismount, explore, and defend the run when trouble finds you.`
6. **Krog tunnel:** Lit tunnel approach with bike headlight and route guidance. Caption: `Recover the phone, then race through Krog Street Tunnel.`
7. **Finish:** Ellison and Morgan at the fictional 98 Estoria celebration. Caption: `Reach 98 Estoria before Atlanta takes the time back.`
8. **Options / accessibility support:** Fullscreen, resolution, music, sensitivity, persistent controls, and bike-physics options visible and readable. Use only if an additional screenshot slot helps explain the Mac experience.

Capture at the largest App Store Connect macOS size currently requested, then export any additional exact sizes shown in the live upload form. Do not upscale a small gameplay capture. Keep separate text-free masters in the release-assets folder so captions can be changed without recapturing gameplay.

## App Review notes for known implementation details

- Minimum target: Apple silicon Mac (M1 or newer), macOS 14 or later.
- Input: mouse and keyboard recommended; current documentation does not claim controller support.
- No additional installation is required in a TestFlight/App Store distribution. Players do not need Unreal Engine, Aura, Xcode, or an Apple Developer account.
- The route uses OpenStreetMap and USGS geographic reference data but is intentionally adapted for play and is not a real-world navigation tool.
- The two included music tracks were supplied for the project. **REQUIRED:** retain written ownership/distribution confirmation before review.
- Third-party asset/license inventory is maintained in `Distribution/MAC-APP-STORE-RELEASE.md` and `Distribution/CREDITS.txt`. **REQUIRED:** archive the exact Fab listing receipts and Mixamo source-download records before submission.

## Submission preflight

- [ ] Upload is the final signed and provisioned Shipping build, not the ad-hoc playtest app.
- [ ] Version/build in App Store Connect match `CFBundleShortVersionString` and `CFBundleVersion`.
- [ ] Bundle ID is `com.webexperts.battleofatl` and the correct Web Experts team is selected.
- [ ] App Sandbox entitlement is present; no unneeded network, microphone, camera, location, or file-picker entitlement is present.
- [ ] Shipping launch, new game, phone pickup, both checkpoints, tunnel order, finish, pause/resume, focus loss, fullscreen, resolution changes, save/load, Quit, and clean relaunch pass on the uploaded binary.
- [ ] Debug menus, console commands, automated drivers, cheats, review maps, and development-only assets are absent or unreachable.
- [ ] No placeholder text, `TODO`, error banner, missing material, gray-box prop, test level, or stale product/build name is reachable.
- [ ] All HUD and menus fit at every supported resolution with readable contrast and line spacing.
- [ ] App icon renders correctly at 16, 32, 128, 256, 512, and 1024 pixels.
- [ ] App Privacy answer is checked against the exact binary and the website feedback privacy policy is current.
- [ ] Age-rating answers are checked against final blood, violence, dialogue, music, and alcohol references.
- [ ] Export-compliance answer is checked against the exact binary and enabled plugins.
- [ ] Music ownership, Fab receipts, Mixamo records, OpenStreetMap attribution, and all third-party licenses are archived.
- [ ] Screenshots and optional preview video come from the submitted build and contain no spoilers, debug UI, or unsupported claims.
- [ ] Support, marketing, and privacy URLs are live and show Web Experts branding.
- [ ] Beta review contact fields are complete and monitored.
- [ ] TestFlight internal test completes before inviting external testers.
- [ ] External beta review notes include the full review path and content disclosures above.

## Remaining metadata decisions

1. Final App Store price and release method after TestFlight.
2. Review contact first name, last name, phone, and confirmed monitored email.
3. Whether `https://webexperts.com/privacy-policy` explicitly covers the game and feedback form; update the policy if needed.
4. Final app age-rating answers after a complete content pass.
5. Final export-compliance answer after inspecting the uploaded Shipping binary.
6. Final screenshots/video from the exact signed build.
7. Written music ownership/distribution confirmation and complete Fab/Mixamo license archive.
