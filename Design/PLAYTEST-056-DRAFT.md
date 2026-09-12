# The Battle of ATL — next Mac playtest (not yet packaged)

This draft describes unreleased work after installed build055. It is not confirmation that build056 exists.

- Press **P while riding** to switch between arcade and realistic handling. The current mode appears below the horn counter. Arcade remains the default.
- In realistic mode, ease off and brake before tight turns. Downhill coasting can carry you above the selected gear's assisted speed. Grass has less grip, and steering in the air does not rotate the bike. Space or S/down brakes; WASD and arrows still work.
- Traffic uses six paint colors on the existing sports-car body. Other everyday vehicle body types remain outstanding.
- The two outer DeKalb road ends have physical construction fences and “MORE ATL COMING SOON” signs. These are local closures, not a complete playable-world boundary.

Realistic handling uses the existing CharacterMovement collision system with a bicycle steering model, slope gravity and traction limits. It is not a fully simulated two-wheel rigid-body vehicle. Mode choice survives getting off/back on during the same run; persistent preferences across application restarts are not implemented.

Checks completed: native controls and mode switching, controlled slope/braking/road-versus-grass/air tests, HUD readability at1280×720, and a two-way main Krog route with both cars and autonomous pedestrians (481.56m, no wipeouts). A mapped park hillside also passes both directions (144.76m total,4.57m elevation span, no wipeouts). Do not infer whole-world acceptance from these selected routes. Subjective ride feel, additional world detail and many full-game requirements remain unfinished.

2026-09-12 [codex-maclaptop]
