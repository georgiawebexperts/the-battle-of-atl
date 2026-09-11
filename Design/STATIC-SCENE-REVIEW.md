# Static scene rendering on the locked Mac

2026-09-11 [codex-maclaptop]

Scripts/render_park_review.sh produces two 1280×720 PNGs and a manifest in
work/scene-review using the real Metal renderer and editor SceneCapture2D. The
Mac desktop can remain locked. The wrapper uses an absolute project path,
AllowCommandletRendering, NoTextureStreaming and RCWebControlDisable. It never
saves the level or any material/actor edits. Native app build024 is unchanged.

NoTextureStreaming is essential for this review: without normal game-view
streaming updates, the capture used coarse landscape heightmap data and appeared
to bury large portions of the paths. With full textures loaded, nearby paths
render continuously. Terrain sign/orientation, global/component LOD0 overrides
and disabling Nanite did not resolve the artifact. Do not rebuild or raise roads
based on the initial streamed captures. A separate terrain-only trace of 2,926
pavement triangle centroids found no buried sample; minimum clearance was about
3.00015 cm, median 3.00507 cm. This is sampled evidence, not every triangle.

The wrapper also disables the Remote Control listener. The first render exported
images successfully but exited1 because an external Aura client repeatedly
requested a missing AuraSandboxStatics object. RCWebControlDisable stopped those
calls for this commandlet, and the final wrapper exited0. No Aura credentials or
persistent plugin settings changed.

Capture sunlight is temporarily raised from the saved intensity4 to40 for
readability. This does not establish how exposure looks in the actual game.
The scene is loaded as an editor world: GameMode does not spawn the bike, traffic,
enemies or HUD. These renders prove static geometry can be reviewed; they do not
prove physical input, gameplay appearance, audio, feel or FPS.

The lake water surface is not visible in the current static capture. Its cause
still needs investigation, including Water subsystem/view initialization before
assuming a missing runtime asset. Foliage, landmark architecture and final world
materials remain visibly unfinished. The diagnostic files in work/render-probe
preserve exploratory attempts, all without map saves. Final image hashes and
scope are recorded in Tests/Results/2026-09-11-static-render-review.json.
