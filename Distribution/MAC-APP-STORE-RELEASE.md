# Mac App Store release record

Last updated 2026-09-16 [codex-maclaptop]

## Release identity

- Product: The Battle of ATL
- Platform: macOS App Store only
- Bundle identifier: `com.webexperts.battleofatl`
- Apple Team ID reserved for later distribution setup: `8HAG5A4GS7`
- Distribution certificates, provisioning profiles, App Store Connect records, and upload remain deferred until feature completion.

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

- The finished `THE BATTLE OF ATL` app icon contains 16, 32, 128, 256, 512, and 1024-pixel macOS representations.
- Pause menu includes Quit.
- Production bundle identity and sandbox-only, no-network entitlements are configured for Mac packages.
- Development playtests remain locally signed. Distribution signing is intentionally not configured.
- A clean Shipping package and runtime smoke test are required after each feature-complete candidate.
- 2026-09-16 [codex-maclaptop] Build 093 crash-camera and HUD changes compile in Shipping and retain every static store-readiness guardrail. Distribution signing and App Store Connect work remain deferred.
- 2026-09-16 [codex-maclaptop] Build 094 swimming motion and fast-stroke control compile in Shipping and retain every static store-readiness guardrail. Distribution signing and App Store Connect work remain deferred.
