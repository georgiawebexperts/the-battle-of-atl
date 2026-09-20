# Mac App Store release record

Last updated 2026-09-19 [codex-maclaptop]

## Release identity

- Product: The Battle of ATL
- Platform: macOS App Store only
- Bundle identifier: `com.webexperts.battleofatl`
- Apple Team ID: `8HAG5A4GS7`
- Apple account: `elliott@webexperts.com`
- App Store Connect Apple ID: `6812874355`
- App Store Connect record and explicit bundle ID are registered. Build 096 is uploaded, processed, assigned to the Public Beta group, and **approved by Beta App Review** — verified live on 2026-09-19. It expires 90 days after upload, on or about 2026-12-15.
- Public beta page: `https://webexperts.com/battleofatl`
- Public TestFlight invitation: `https://testflight.apple.com/join/tpHVaTXK` — **still active on 2026-09-19**, with zero testers joined. Elliott is undecided between keeping it public and moving to private per-person email invitations, so it was deliberately left on. Retire it with TestFlight → External Testing → `Public Beta` → `Manage` → `Disable Public Link`.
- Distribution state, read live on 2026-09-19: Build 96 in `Web Experts Internal` + `Public Beta`, **0 testers, 0 invites, 0 installs, 0 sessions, 0 crashes, 0 feedback**. Nobody has played the TestFlight build. Note that TestFlight serves Build 096, which is behind the local playtest build 153.
- The "macOS 1.0 — Prepare for Submission" status in the apps list is the App Store release version, not the beta. Build 096 being approved for TestFlight does not submit 1.0.

## Submission guardrails

- Runtime data must use Unreal save, config, log, and cache directories inside the app container.
- Shipping builds must exclude development audits, automated drivers, cheat behavior, and debug console paths.
- The game requires no network, microphone, camera, file picker, or user-selected-file entitlement.
- The only cooked playable map is the production `PiedmontWorld`; review maps and the arcade lab are development assets.
- Required front-end behavior includes a Quit command, focus-loss pause, fullscreen control, resolution control, and complete macOS icon sizes.
- Final submission cannot contain placeholder copy, gray-box presentation, TODO labels, or reachable test levels.

## Third-party content and license inventory

| Content | Source / creator | License or basis | Release status |
|---|---|---|---|
| Unreal Engine, Water, PCG, Control Rig, RigLogic, and Epic-provided assets | Epic Games | Unreal Engine/Epic content terms | Included; final cooked-asset review required |
| Geographic map geometry | OpenStreetMap contributors | ODbL; attribution included in `CREDITS.txt` | Ready |
| Bare-earth elevation | USGS 3DEP | U.S. government public-domain data | Ready |
| Character animation clips | Adobe Mixamo | Mixamo license; redistributed only as embedded game content | Source-download records still need archiving |
| Ultimate Modular Males | Quaternius | CC0 1.0 | Ready; license retained in source assets |
| Remington Model 870 | britdawgmasterfunk / Sketchfab | Download archive supplies CC BY 4.0; attribution included | Ready |
| “Stop! APD!” voice | OpenAI built-in cedar voice | AI-generated for this game | Record retained in `CREDITS.txt` |
| “Battle of the A-T-L” and “The Battle of A-T-L2” | Supplied by Elliott Inspace | Ownership/distribution confirmation required before submission | Pending documentation |
| Fab library environment/character additions | Individual Fab publishers | Per-item Fab Standard License or listing terms | Exact listing export and license receipts required before submission |
| Original ending artwork and game UI art | Generated for The Battle of ATL | Project-owned generated output and documented prompts | Ready |

## Current technical status

- The finished `THE BATTLE OF ATL` app icon contains all required 16, 32, 128, 256, 512, and 1024-pixel macOS representations. The current 2026-09-16 v2 artwork is installed in both Build 096 packages and remains recognizable at 16px and 32px.
- Pause menu includes Quit.
- Production bundle identity and sandbox-only, no-network entitlements are configured for Mac packages.
- Development playtests remain locally signed. The Build 096 App Store candidate is signed with the Web Experts Mac App Distribution identity and packaged with the matching Mac Installer Distribution identity.
- A clean Shipping package and runtime smoke test are required after each feature-complete candidate.
- 2026-09-19 [codex-maclaptop] Read App Store Connect live and corrected this record. Build 096 is **Approved**, not Waiting for Review, and expires on or about 2026-12-15. The `Public Beta` external group holds **0 testers, 0 invites, 0 installs, 0 sessions, 0 crashes and 0 feedback** — nobody has played the TestFlight build. The public invitation link is still active and was deliberately left on, pending Elliott's decision between keeping the beta public and moving to private per-person email invitations. The "macOS 1.0 — Prepare for Submission" row in the apps list is the App Store release version and is separate from the beta. Invitation paths verified on screen: per-email, existing testers, and CSV import; Apple's limits are 10,000 external testers per app and 100 internal, and macOS TestFlight requires macOS 13 or later.
- 2026-09-16 [codex-maclaptop] Build 096 packaged successfully in Shipping, launched under App Sandbox, and was staged at `/Volumes/Adam Assets/Unreal/Builds/AppStore/The Battle of ATL 0.96.0 Shipping/The Battle of ATL.app`. The staged candidate is ad-hoc signed until Xcode installs the Apple distribution identities.
- 2026-09-16 [codex-maclaptop] Accepted the current Apple Developer Program agreement, registered `com.webexperts.battleofatl`, and created The Battle of ATL macOS app record (Apple ID `6812874355`).
- 2026-09-16 [codex-maclaptop] Rebuilt the 1.4 GB Build 096 share ZIP with the v2 icon. `unzip -t` found no archive errors, and `codesign --verify --deep --strict` passed for both the share app and Shipping candidate.
- 2026-09-16 [codex-maclaptop] Created Mac App Distribution and Mac Installer Distribution certificates expiring 2027-09-16, installed the Apple WWDR G3 chain, and generated the `The Battle of ATL Mac App Store 2026` provisioning profile for `com.webexperts.battleofatl`.
- 2026-09-16 [codex-maclaptop] Prepared Build 097 with a wider, east-side Murder K plaza and a concrete-and-glass 725 Ponce-inspired landmark, moved hostile encounters away from the center riding line, and changed visible startup branding to `The Battle of ATL built by Web Experts`. The legacy `AuraPlayground` name remains only as the internal Unreal module and executable identifier.
- 2026-09-16 [codex-maclaptop] Signed and validated Build 096, created a signed 1.3 GB installer package, and uploaded it to App Store Connect. Apple processing completed successfully; only non-blocking missing-dSYM warnings were reported.
- 2026-09-16 [codex-maclaptop] Created `Web Experts Internal` and `Public Beta` TestFlight groups. Build 096 is `Ready to Submit`; its testing instructions are saved. Beta App Review requires the monitored contact phone number before the build can be added to the public group.
- 2026-09-16 [codex-maclaptop] Added Build 096 to the Public Beta group and submitted it for external TestFlight review. The build is Waiting for Review. Created the public invitation and linked it from `https://webexperts.com/battleofatl`; the invitation activates when Apple approves the build.
- 2026-09-16 [codex-maclaptop] Build 093 crash-camera and HUD changes compile in Shipping and retain every static store-readiness guardrail. Distribution signing and App Store Connect work remain deferred.
- 2026-09-16 [codex-maclaptop] Build 094 swimming motion and fast-stroke control compile in Shipping and retain every static store-readiness guardrail. Distribution signing and App Store Connect work remain deferred.
