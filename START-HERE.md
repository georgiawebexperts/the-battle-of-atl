# Aura playground

Created 2026-09-10 for Elliott's short Aura trial. Use fictional or public content.

First ask Aura: "Tell me about this project and confirm you can inspect the open level. Do not change anything yet."

Small first experiment: "Build a simple showroom using basic shapes: a floor, three product pedestals, lighting, and a camera view. Put a cube on the center pedestal. Keep it small and save the level as ShowroomTest. Take a screenshot of the result."

Then: "Make the center cube change between red, blue, and green when I press Space in Play mode. Explain briefly how to test it."

The purpose is to see whether Aura can produce something interactive that could be useful for a client. Start with basic shapes before adding purchased assets or complicated models.

Review date: September 21, 2026. Nothing should be deleted automatically.

Trial locations:
- Engine: /Volumes/Adam Assets/Unreal/UE_5.8
- Test project: /Volumes/Adam Assets/Unreal/Projects/AuraPlayground
- Epic asset cache: /Volumes/Adam Assets/Unreal/VaultCache
- Aura application: /Applications/Aura.app
- Xcode application: /Applications/Xcode.app

Review other projects and tool dependencies before uninstalling Unreal or Xcode. Save any work worth keeping before deleting the playground.


## BeltLine Glide — current prototype, September 10, 2026

The trial now contains a Lake Clara Meer/Piedmont Park blockout with an adjoining mapped trail. It does not yet reproduce the entire BeltLine. The lake comes from OpenStreetMap; the loop is an approximation derived from the shoreline. Terrain is flat and scenery uses simple shapes.

Open AuraPlayground.uproject and the BeltLineGlide map. The native game controller is compiled and the map is saved using /Script/AuraPlayground.GlideGameMode. Press Play in Unreal, then click the game viewport.

- W / Up arrow: forward
- S / Down arrow: reverse
- A / Left arrow and D / Right arrow: steer
- Space: brake
- R: reset bike
- Escape: stop Play In Editor

The native controller includes a chase camera, controls/speed/score display, moving pedestrians and scooter riders, and a collision penalty/reset. **Build succeeded, but this native version still needs the final hands-on play test. The Mac locked before that test could run.**

Implementation: Source/AuraPlayground/GlideGame.cpp and GlideGame.h. Earlier Aura-generated Blueprints remain preserved. The original placed Blueprint bike and its presentation actors are hidden during gameplay; the game mode spawns the native bike. Input is configured in Config/DefaultInput.ini. Startup/default map settings are in Config/DefaultEngine.ini. One-time migration in Content/Python/init_unreal.py saved the native game mode; its result is Scripts/native-setup-done.json.

Maps and attribution: References/MAP-SOURCES.md. Preserve OpenStreetMap attribution when sharing.

All engine, project, source, and build products are on Adam Assets. Xcode and Aura.app are internal; Xcode's Metal toolchain is system managed. Trial review reminder is September 21. Nothing will be deleted automatically.


### 2026-09-10 [codex-maclaptop] — Native controller play test
- Launched Play in Editor successfully after unlock using the external project Python test script. Native GlideBike and GlideHUD active.
- CUA key taps verified Up/W translation, Right/A steering, and R reset (visually returned to initial view). Telemetry in Scripts/live-test.json confirms coordinate and yaw changes. Short automated taps produce centimetres of movement; sustained human-key riding and full-lap/collision/brake tests remain unverified.
- Nine traffic actors (pedestrians and scooters) are running at ground height, with changing positions in telemetry. Prototype remains a crude, flat Lake Clara Meer slice with nearby trail, not the whole BeltLine.
- Left game running in PIE for Elliott. Click viewport, hold WASD or arrows; Space brake, R reset, Escape exits play.
