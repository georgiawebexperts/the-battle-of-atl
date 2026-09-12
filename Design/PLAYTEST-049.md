# The Battle of ATL — Mac playtest 049

2026-09-12 [codex-maclaptop]

Installed on desktop as0.49.0. Previous build048 is preserved on the external drive. This is a development playtest, not a final release.

Changes: non-water riding collisions use physical rider and bike falls; recovery returns Ellison to unarmed on-foot movement. Walk back to the fallen bike and press E to remount. Off-bike countdown runs at the existing1.25 multiplier. Death interrupts and cleans up the crash. Get-up alignment compares whole-body landmarks. HUD explains knockdown and getting back up. Park canopy includes detailed hornbeam trees and tested trunk collision.

Native editor-game evidence: keyboard collision/remount, two consecutive crash cycles, mid-fall death, post-get-up death. These tests do not accept arbitrary terrain, blocked recovery, animation/video quality, every hazard, or full-game performance.

Packaging/cooking passed. Packaged two-cycle crash/remount, post-get-up death, health/checkpoint tests passed. Captured and inspected packaged crash/remount and canopy views; strict local signature and desktop target/version verified. Three fixed1920x1080 canopy views measured median12.117/15.722/14.296ms, not sustained gameplay performance acceptance.
