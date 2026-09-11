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

## Lake render correction, build025

2026-09-11 [codex-maclaptop]

The reflected WaterBodyLake scale (1,-1,1) left the world-space shoreline
clockwise. The normal material was invisible; a plain material on a temporary
copy of the same mesh rendered the full lake. Rebuilding water alone, warming
engine frames, and enabling persistent capture state did not resolve it.
Normalizing the actor to positive scale and reversing its spline order restored
water rendering with the original Water plugin materials. All 131 world-space
vertices are identical (maximum error 0 cm). The terrain, island, hazard polygon,
paths and bridges were not moved. Scripts/normalize_lake_transform.py is
repeatable and saves only after checking the vertex positions.

Static reviews now warm 12 engine frames per view, explicitly ticking the water
subsystem, and preserve the capture view state. TickSceneReview is commandlet-only
and editor-only. These captures still do not establish actual gameplay input,
audio or frame rate. The map remains visibly unfinished in foliage and landmarks.

The first normalized save still rendered only part of the lake after reopening.
A fresh water-body rebuild restored coverage; rebuilding the water zone alone did
not. WaterMeshComponent.cpp reuses the lake physics hull vertices for tile
polygons, assuming bottom vertices then top vertices. Disabling the redundant
WaterBody collision makes it use the shoreline spline instead. The separate
PiedmontWaterHazard and hidden solid shore still implement V3 recovery. Underwater
Water plugin post-processing/buoyancy overlaps are no longer supplied by this
lake; V3 uses timed bike recovery, not an underwater swimming mode.

Final fresh-process review after saving the spline-tile setup shows full lake
coverage, the exposed island, and the crossing bridges. The verified image is
work/water-review/final-spline-aerial.png and the manifest/hash is recorded in
Tests/Results/2026-09-11-build025-water-render.json. Use unique image filenames
when reviewing successive revisions so the image viewer cannot reuse a prior
path's preview.
