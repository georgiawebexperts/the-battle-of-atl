# Build Prompt: Piedmont Park to Krog Street Tunnel E-Bike Game (Unreal Engine 5)

## Your role

You are the lead technical artist and gameplay engineer on an Unreal Engine 5 project. You are working inside the Aura/Codex toolchain and can create Blueprints, C++ classes, Landscape actors, materials, Niagara systems, AI controllers, and UMG widgets. Treat everything in this document as the design spec. Where the spec is silent, choose the option that makes the game look and feel more like real Atlanta, not the option that is easiest to build.

Version 1 of this project exists and it is not acceptable. Read the "What is wrong with version 1" section first so you understand what has to change.

## What is wrong with version 1

- The bike has no physics. It can ride into Lake Clara Meer and across the water. It does not lean, does not slow on grass, does not stop at walls or fences.
- There is no rider. The bike is an empty mesh.
- The world is blocky primitive geometry. Nobody from Atlanta would recognize it as Piedmont Park. There are no real landmarks, no real road layout, no trees, no skyline.
- There are no pedestrians, scooters, joggers, or other bikers. The world is empty.
- There is no game logic. No timer, no pickup, no finish line, no difficulty, no win or lose state.
- The gears and controls are not implemented as specified.

Version 2 must fix all six of these. Do not ship a build that still has any of them.

## The game in one paragraph

The player rides an e-bike through a scaled but geographically faithful recreation of Piedmont Park in Atlanta, Georgia. A collectible item is hidden at a random location somewhere in the park's network of paths. The player has to explore the park loops, find the item, then exit the park onto the Atlanta BeltLine Eastside Trail and ride south as fast as possible, past the Ponce Kroger and Ponce City Market, to the finish line at the mouth of the Krog Street Tunnel. The whole run has to be completed before a countdown timer expires. The trail is crowded with pedestrians, joggers, dog walkers, other cyclists, and e-scooters at a range of speeds. Collisions cost time and speed. Harder difficulties shorten the timer and add more and faster obstacles.

## Non-negotiable requirements

1. Real geography. The layout of paths, the lake, the ovals, the gates, and the BeltLine route must match the real places in the correct order and correct relative positions. Use the reference photos supplied with this prompt and OpenStreetMap data for the path network.
2. Recognizable landmarks. A person who has been to Piedmont Park and ridden the Eastside Trail should be able to name where they are at any point in the run.
3. A physical bike with a rider on it. Full physics, lean, gears, braking, no riding on water.
4. A living world. NPC traffic on every path, with behaviors, not static props.
5. A complete game loop. Start, timer, hidden pickup, checkpoints, finish, win, lose, difficulty select, restart.
6. Looks like a game people would want to play, not a graybox. Nanite geometry, Lumen lighting, Quixel Megascans foliage, real Atlanta sky and time of day.

## The world

### Scale

Build the world at roughly one third of real-world scale. The real course is 4 to 5 miles and the hard difficulty gives the player 5 minutes, so real scale would require highway speeds. Preserve the shape, the order of landmarks, and the relative proportions. Compress distances. The player's top speed should feel like a fast e-bike, about 28 to 32 mph on screen, and that speed should be enough to finish a clean run on hard with almost no margin.

### Source data for layout

- Pull the path and road network for Piedmont Park and the Eastside Trail from OpenStreetMap. Use the Landscaping plugin, Blender OSM export, or Cesium for Unreal as a reference layer. Extract the bike lane and multi-use path centerlines as splines and generate the paved surfaces from those splines.
- Use Google Photorealistic 3D Tiles through Cesium for Unreal as a reference overlay while blocking out. Do not ship the raw tiles as the final environment. Rebuild the key areas with proper assets so they read cleanly at bike speed.
- Use the supplied reference photos to match materials, tree species, fencing, benches, light poles, signage, and the look of each landmark.

### Piedmont Park, the start area

Recreate these areas and place them where they really are:

- Lake Clara Meer, the central lake, with the paved loop around it. This is the primary oval. The water must be a real Water Body Lake actor with collision that stops the bike. If the bike leaves the path and rolls into the lake, the rider dumps the bike, loses several seconds, and respawns on the nearest path.
- The stone boathouse and dock on the lake.
- The Piedmont Park Aquatic Center and Sharon Lester tennis courts on the lake's edge.
- The Active Oval, the large running track oval with sports fields inside it, on the west side of the park. This is the second major oval.
- The Meadow and Oak Hill, the big open lawn with the Midtown skyline visible above the trees to the west. The skyline must be there as background geometry: 1180 Peachtree, the Bank of America Plaza spire, the Promenade towers, and the rest of the Midtown line. This is the single most recognizable view in the park.
- The Promenade, the wide paved walkway with the double row of trees and the Legacy Fountain at the 10th Street and Piedmont Avenue corner.
- The 12th Street Gate and the 14th Street Gate on Piedmont Avenue with their stone entrances. The Charles Allen Gate on 10th Street.
- Magnolia Hall, the Greystone, and the Piedmont Park Conservancy Community Center near the lake.
- The Noguchi Playscape (the painted geometric playground near 12th Street). It is small but instantly recognizable, so include it.
- The dog park on the north side, the bocce courts, the community garden, and the Piedmont Park Conservancy greenhouse area near Monroe Drive.
- The Atlanta Botanical Garden boundary on the northwest side, with the canopy walk visible above the tree line.
- Piedmont Avenue on the west edge and Monroe Drive on the east edge with street traffic that the player can hear and glimpse but cannot ride into.

The park path network is not just the lake loop. It is an extensive web of paved paths and bike lanes connecting all of the areas above. Build the whole web so the hidden item can be placed anywhere along it and so there are multiple routes to the BeltLine exit.

### The BeltLine exit and the Eastside Trail

- The park connects to the BeltLine Eastside Trail on its southeast corner, near 10th Street and Monroe Drive. Build the real connector ramp and the trail entrance signage.
- The trail runs south. It is a wide concrete multi-use path with a painted centerline, art installations, murals on adjacent building walls, apartment blocks and restaurants on both sides, and low retaining walls and rail-corridor remnants. Match the reference photos for the surface, lighting poles, and wayfinding signs.
- Landmark order heading south from the park, in this sequence:
  1. The trail section behind the apartments south of the park, passing under the Virginia Avenue area and the Kanuga Street pedestrian bridge.
  2. The Ponce Kroger. This is the street-level Kroger in the 725 Ponce building, right on the trail, near the halfway point. Build the storefront, the trail-facing plaza, and the building above it. This is the midpoint checkpoint.
  3. Ponce City Market immediately after, the huge brick former Sears building with the trail running along its east side and the Ponce de Leon Avenue bridge.
  4. Historic Fourth Ward Park with the lake and amphitheater visible to the west.
  5. The Ralph McGill Boulevard crossing and the Freedom Parkway bridge.
  6. The Krog Street Market area with the brick market building and the trail ending at Irwin Street.
  7. The Krog Street Tunnel. The graffiti-covered concrete underpass beneath the rail tracks on Krog Street connecting Inman Park to Cabbagetown. The finish line is the mouth of the tunnel. When the player crosses it, the camera pulls back and the run ends inside the tunnel with the murals in view.

### Environment quality bar

- Landscape actor with a real heightmap, not a flat plane. Piedmont Park slopes down from Oak Hill and 14th Street toward the lake. The BeltLine has grade changes. Use the real elevation data.
- Trees: Quixel Megascans or equivalent. Willow oaks, magnolias, and hardwoods in the park. Use PCG to scatter them along the real tree lines shown in the photos. Grass should be Nanite foliage, not a flat green material.
- Lumen global illumination, Nanite on all static geometry, a sky atmosphere and volumetric clouds with a late afternoon Atlanta sun by default.
- Path surfaces: asphalt for park paths with painted lane markings where they exist, concrete for the BeltLine with the painted centerline and the trail wayfinding stencils.
- Props: benches, trash cans, light poles, bike racks, park signage, BeltLine art pieces. Populate them; empty paths look wrong.
- Audio: ambient park sound, city traffic near the streets, bike freewheel click, tire noise that changes on grass versus pavement, e-motor hum, brake squeal, NPC chatter and scooter whine.

## The bike and rider

### Pawn

- Build a custom Pawn. Root is a physics-enabled capsule or a two-wheel Chaos Vehicle setup with a skeletal bike mesh. A capsule root with a custom movement component is acceptable if the Chaos two-wheel setup proves unstable, but the result must feel weighty and must interact with slopes, curbs, and obstacles.
- The bike is an e-bike: a commuter style frame with a battery on the downtube and a hub motor. Add a small motor sound layer that rises with assist output.
- There must be a rider. Use a Metahuman or a rigged human character in casual cycling clothes, seated on the bike, with pedaling animation driven by wheel speed, a lean pose driven by turn input, and a braking pose. When the bike crashes the rider ragdolls or plays a dismount animation.

### Controls

- Up arrow: shift up one gear. Down arrow: shift down one gear. There are 7 gears. Each gear has a speed band. Low gears accelerate fast but cap low; high gears accelerate slowly from a stop but reach top speed. The e-assist scales with gear so that the player has to shift to get the best speed. Show the current gear on the HUD.
- Hold W to pedal. While pedaling, power is applied based on current gear and current speed. Releasing W coasts with rolling resistance and freewheel sound.
- Hold Space bar to brake. Braking is progressive. Braking hard while leaning can cause a slide. Braking on grass is less effective.
- Left and right arrows steer and lean. At low speed this turns the front wheel. At higher speed steering input leans the bike into the turn and the lean angle is visible on the bike mesh and the rider. Excessive lean at high speed on a tight turn causes a low-side crash.
- Shift or Tab toggles the camera between a chase camera and a first-person handlebar camera. Chase camera is default.
- Mouse look while riding is optional and should not fight the arrow steering.

Note: an earlier draft had Space as pedal. The current spec is Space brakes and W pedals. Do not implement Space as pedal.

### Physics rules

- The bike cannot enter water. The lake and the Fourth Ward Park pond have solid collision at the shoreline plus a trigger volume that causes a crash if the bike goes over the edge.
- Grass and dirt slow the bike by 40 percent and reduce traction. Stairs and curbs above a certain height stop the bike and cause a crash if hit at speed.
- Collisions with NPCs: hitting a pedestrian or scooter at speed causes a crash and a 3 to 5 second recovery. Brushing one at low speed causes a stumble and a speed penalty.
- Fences, walls, and buildings are solid.
- Downhill sections increase speed. Uphill sections slow the bike noticeably in high gears so shifting matters.

## Game logic

### Flow

1. Main menu: Play, Difficulty (Easy, Medium, Hard), Controls, Quit.
2. The run starts at the 12th Street Gate facing into the park. A 3 second countdown, then the timer starts.
3. Phase one, the park. The hidden item (a Piedmont Park Conservancy tote bag or a specific Atlanta item such as a Waffle House bag) spawns at a random point on the park path network, at least 400 scaled meters from the start and never on the direct route to the BeltLine exit. The HUD shows a warm/cold proximity indicator that only pulses when the player is within a short radius, so the player has to actually ride the loops. The item is visible on the path with a glow and a small marker once within line of sight.
4. When the player picks up the item, the HUD confirms it and a compass arrow points toward the BeltLine exit.
5. Phase two, the BeltLine. The player exits the park at the 10th and Monroe connector, rides south, and must pass through the checkpoint at the Ponce Kroger. Crossing the checkpoint gives a small time bonus on Easy and Medium and none on Hard.
6. The finish line is the mouth of the Krog Street Tunnel. Crossing it with the item ends the run as a win. Time remaining is the score.
7. If the timer hits zero before the finish, the run ends as a loss with a restart option.
8. Results screen: time used, time remaining, crashes, top speed, and a best time per difficulty saved locally.

### Difficulty

| Setting | Timer | NPC density | Scooter top speed | Extra hazards |
|---|---|---|---|---|
| Easy | 15 minutes | Low | Slow | None |
| Medium | 10 minutes | Medium | Mixed slow and fast | Aggressive scooter riders that swerve |
| Hard | 5 minutes | High | Fast, some wrong-way | The hostile character near the Ponce Kroger is active and pursues the player |

The hidden item search radius also changes: on Easy the proximity pulse triggers from farther away; on Hard it only pulses when very close.

### NPCs

Use NavMesh-driven AI controllers, or the Mass AI crowd system if the density gets high enough that individual controllers hurt performance. Every NPC needs a behavior, a walk or ride animation, and collision. Types:

- Walkers, in pairs and groups, who take up the path width and occasionally stop.
- Dog walkers with a leash that spans the path.
- Joggers who move in a steady line and do not yield.
- Other cyclists at normal bike speed who overtake and get overtaken.
- E-scooter riders in three classes: slow and wobbly, normal, and very fast. On Medium and Hard some ride against traffic and some swerve unpredictably.
- People loitering, sitting, or camped along the trail edges as slow or stationary obstacles that narrow the path, particularly in the stretch between the park and Ponce City Market.
- One hostile character on Hard only, spawned near the Ponce Kroger plaza as a nod to the old Murder Kroger nickname. He runs toward the player when they enter a radius. If he catches the bike the player crashes and loses 10 seconds. The player can outrun him by keeping speed through the checkpoint.

NPC density should be highest around the lake loop, the Promenade, the Ponce City Market stretch, and the Krog Street Market approach, which matches reality.

### HUD

- Countdown timer, large, top center.
- Current gear and speed, bottom left.
- Item status: "Find the item" with the proximity pulse, then "Item collected, get to Krog Street" with a compass arrow.
- Checkpoint notification at the Kroger.
- Minimap in the corner showing the real path network outline, the player, and after pickup the route to the finish. Do not show the item location on the minimap before pickup.

## Technical stack and structure

- Unreal Engine 5.4 or newer. Blueprints for gameplay and UI, C++ for the bike movement component and the NPC spawner if performance requires it.
- Plugins: Water, PCG, Landscaping or Cesium for Unreal for reference data, Metahuman, Chaos Vehicles if used.
- Folder structure: Content/PiedmontRide/Maps, Bike, Rider, NPC, Environment/Park, Environment/BeltLine, Landmarks, UI, Audio, Materials.
- One persistent level with the park and the BeltLine as streamed sublevels or World Partition cells so the full route loads without hitches.
- Version the project. Author credit: Web Experts, www.webexperts.com. Increment the build number in the project settings on every milestone.

## Build order

Work through these milestones in order and confirm each one before moving on. Do not skip ahead to polish while an earlier milestone is broken.

1. Bike pawn with rider, all controls, physics, lean, gears, braking, crash and respawn, tested on a flat test map with ramps, grass patches, and a water body. The bike must be fun to ride here before any world is built.
2. Landscape from real elevation data, path splines from OSM, lake with collision, park boundaries. Ride the entire path network end to end and confirm every path is connected and rideable.
3. Landmarks in the park, in their real positions, matched to the reference photos. Skyline. Trees and grass via PCG.
4. BeltLine trail from the park exit to the Krog Street Tunnel with all landmarks in order.
5. Game loop: menu, timer, random item spawn, pickup, checkpoint, finish, win, lose, results, save best time.
6. NPCs with behaviors and collision, then difficulty scaling.
7. Audio, lighting pass, post process, HUD polish, performance pass to hold 60 fps at 1080p on a mid-range GPU.

## Acceptance test

The build is done when all of the following are true:

- A rider is visible on the bike and animates with speed, lean, and braking.
- Riding into Lake Clara Meer is impossible; the bike crashes at the shoreline.
- Up and down arrows change gears and the gear visibly changes acceleration and top speed. W pedals, Space brakes, left and right lean.
- Someone who knows Atlanta can identify Lake Clara Meer, the Active Oval, the Meadow with the Midtown skyline, the Promenade, the 12th Street Gate, the Ponce Kroger, Ponce City Market, Historic Fourth Ward Park, Krog Street Market, and the Krog Street Tunnel without being told.
- The hidden item spawns somewhere different on each run and cannot be found by riding straight to the exit.
- Pedestrians, joggers, cyclists, and scooters populate every path and can be crashed into.
- Easy, Medium, and Hard use 15, 10, and 5 minute timers and visibly change NPC density and speed.
- A clean run on Hard is possible but requires correct gear use and near-perfect line choice.

## Reference material

Reference photos of the park, the gates, the lake, the Promenade, the BeltLine trail surface, the Ponce Kroger, Ponce City Market, and the Krog Street Tunnel are attached. Use them for materials, tree placement, signage, and the look of each landmark. When a photo and OpenStreetMap disagree on layout, trust OpenStreetMap for positions and the photo for appearance.

## 2026-09-10 world expansion — superseding requirements

Elliott added swimming with the bike left at its lake-entry point, separate rider/bike recovery and more realistic crashes, rare gun/knife encounters (bullet hits end the run; knife hits show blood), occasional 50 mph hyperbikes, rollerskaters, linked owner/dog/leash groups, hill exploration, matched trees and benches, a horn and automatic bike lights. The world continues through Krog Street Tunnel with provision for a future different game area. See WORLD-EXPANSION.md for the full requirements and revised implementation/acceptance sequence. Its water recovery and traversable tunnel rules supersede automatic water respawn and an endpoint-only tunnel above. These additions are recorded, not implemented in the current checkpoint.


## 2026-09-10 combat and compass clarification

The latest Combat and retrieval revision in WORLD-EXPANSION.md is authoritative: first knife hit dismounts and starts a chase; a second before remount is fatal. Voluntary dismount/remount and player gun use on foot are required. The countdown continues throughout movement and combat. Pre-pickup guidance is a coarse N/E/S/W compass, superseding the warm/cold pulse. Death clears the retrieved item and restarts the whole timed run with a newly randomized search. Bloody shooting effects and convincing hit/crash reactions are requested. All of these additions remain pending implementation.
