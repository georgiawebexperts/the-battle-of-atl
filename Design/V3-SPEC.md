# Build Prompt: BATTLE FOR THE A (Unreal Engine 5)

Version 3 spec. Author: Web Experts, www.webexperts.com. Increment the project build number on every milestone.

## Your role

You are the lead gameplay engineer and technical artist on an arcade action game in Unreal Engine 5, working through the Aura/Codex toolchain. You can create C++ classes, Blueprints, Landscape actors, materials, Niagara systems, AI controllers, animation blueprints, and UMG widgets. This document is the design spec. Where it is silent, pick the option that is more fun, more forgiving, and more recognizably Atlanta. This is an arcade game, not a simulator.

## What is wrong with the current build

- The bike is too hard to ride. It falls over on uneven terrain and the player spends more time getting back up than playing. It feels like a physics demo, not a game.
- There are no trees. There are no people. The park is empty and does not look like Piedmont Park.
- Shooting is weak or missing. There is no way to fight back.
- There is no title screen, no instructions, no level select, no radar, no game loop.

All of these must be fixed. Do not spend time polishing anything until the bike is fun to ride and the park is full of life.

## The game

BATTLE FOR THE A is an arcade action game set in a scaled, geographically faithful Piedmont Park and the Atlanta BeltLine Eastside Trail. The player starts on an e-bike at the 14th Street stone gate. Somewhere in the park an Artifact has been hidden. A radar on the HUD points toward it. The player rides the park loops, dodging crowds, finds the Artifact, then races down the BeltLine, past Murder Kroger and Ponce City Market, through the Krog Street Tunnel, to home in Cabbagetown before the clock runs out. Zombies, hostile shooters, rogue scooters, and hyped-up illegal e-bikes try to stop them. At any time the player can hop off the bike and the game becomes a first person shooter. Get back on the bike to keep moving.

Tone: loud, fast, funny, slightly unhinged. Think Crazy Taxi meets a zombie shooter, set in real Atlanta.

## Non-negotiable requirements

1. Forgiving arcade bike handling. The bike does not fall over from terrain. Ever.
2. Dismount into a first person shooter with real weapons and real enemies.
3. A packed park. Trees, crowds, music, frisbee, dogs, people chilling on the grass. Things are happening everywhere.
4. Real geography and recognizable landmarks, matched to the reference photos in the Reference folder.
5. A complete front end: title screen, Start, Instructions, Level Select, plus HUD with timer and radar.
6. Difficulty scales everything: time, crowd size, enemy count, enemy speed.

## The world

### Scale and layout

Build at roughly one third real scale so a Hard run fits in 5 minutes at arcade speeds. Keep the shape, the landmark order, and the relative positions accurate. Pull the path and road network from OpenStreetMap and generate the paved surfaces from splines. Use Cesium for Unreal with Google Photorealistic 3D Tiles only as a blockout reference, then rebuild with proper assets.

### Piedmont Park

The run starts at the 14th Street Gate on Piedmont Avenue, the stone entrance with the pillars and low stone walls. Match the reference photo. The player faces into the park with the path splitting ahead.

Build these areas where they really are, matched to the photos:

- Lake Clara Meer with the paved loop around it, the stone boathouse and dock, the Aquatic Center, and the Sharon Lester tennis courts. The lake is a Water Body actor. If the bike goes in, the rider splashes, the screen shakes, and they respawn on the nearest path two seconds later. Water is a time penalty, not a fail.
- The Midtown skyline across the lake. This view is the reference photo and it must read the same in the game: 1180 Peachtree, the Bank of America Plaza spire, the Promenade towers, reflections on the water.
- The Active Oval on the west side, the running track with fields inside it.
- The Meadow and Oak Hill, the big lawn with the skyline above the trees.
- The Promenade with the double row of trees and the Legacy Fountain at 10th and Piedmont.
- The 12th Street Gate and the Charles Allen Gate.
- Magnolia Hall, the Greystone, the Noguchi Playscape, the dog park, the bocce courts, the community garden, and the Botanical Garden boundary with the canopy walk visible.
- Piedmont Avenue and Monroe Drive on the edges with traffic sound and glimpses of cars.

The full path network, not just the lake loop. The Artifact can spawn anywhere on it.

### Trees and foliage

This is the biggest visual failure of the current build. Fill the park with trees. Willow oaks, magnolias, and hardwoods from Quixel Megascans, scattered with PCG along the real tree lines visible in the photos, with dense canopy on the Promenade, Oak Hill, and the lake shore. Nanite grass on every lawn. Shrubs and flower beds near the gates and the Conservancy buildings. Fallen leaves on the paths. If a screenshot of the park has fewer than 30 trees in frame, it is wrong.

### Life in the park

The park has to feel like a Saturday afternoon. Populate it with animated NPC groups placed where they really happen:

- People playing frisbee and throwing a football on the Meadow and Oak Hill, with the disc actually flying between them and occasionally landing on the path.
- People chilling on blankets on the grass, sitting up, lying down, on their phones, eating.
- A drum circle and a DJ with a portable speaker near the Promenade, with a crowd dancing. Music is audible and gets louder as you approach.
- A guitarist near the boathouse. A saxophone player at the 12th Street Gate.
- Dog owners with dogs on leashes, dogs off leash in the dog park, a dog that runs across the path.
- Kids on the Noguchi Playscape and the playground.
- Runners on the Active Oval. Tennis matches on the courts. Swimmers audible at the Aquatic Center.
- Roller skaters, some doing tricks, on the lake loop and the Promenade.
- Vendors: an ice cream cart, a hot dog stand, a guy selling water.
- Walkers, joggers, cyclists, and e-scooters on every path.

Every NPC has an animation, a sound, and collision. Nobody is a statue.

### The BeltLine Eastside Trail

Exit the park at the 10th and Monroe connector onto the wide concrete trail with the painted centerline, art installations, murals, apartments, and restaurants. Landmarks heading south, in order:

1. The stretch behind the apartments south of the park and the Kanuga Street pedestrian bridge.
2. Murder Kroger. The Kroger at 725 Ponce de Leon Avenue on the trail. Storefront, plaza, big building above, and an obvious Murder Kroger nod: graffiti on the wall, "MURDER" tagged above the sign, stickers on poles. Midpoint checkpoint. The most dangerous zone on the route.
3. Ponce City Market, the huge brick building with the trail on its east side and the Ponce de Leon Avenue bridge. Match the reference photo.
4. Historic Fourth Ward Park with the lake and amphitheater to the west.
5. The Ralph McGill crossing and the Freedom Parkway bridge.
6. Krog Street Market and the trail end at Irwin Street.
7. The Krog Street Tunnel, graffiti on every surface, echoing sound. Match the reference photo.
8. Home: a short stretch of Cabbagetown with shotgun houses, murals, and the Stacks lofts above the roofs. The finish is the front gate of a bungalow with the player's name on the mailbox.

The trail is crowded the whole way, densest at Ponce City Market and Krog Street Market.

## The bike

### Arcade handling, not simulation

Throw out the physics-driven bike. Build a custom Character Movement based pawn with a kinematic bike:

- The bike never tips over from terrain, curbs, grass, roots, or slopes. It stays upright always. Lean is cosmetic and driven by steering input.
- Uneven ground: the bike bobs and the camera shakes a little, speed drops slightly. That is all.
- Grass slows the bike by 25 percent. Stairs are a bump. Solid walls, fences, and buildings stop the bike with a bounce and a short speed loss.
- Crashes only happen from hitting an NPC, vehicle, or enemy at speed, or from an enemy attack. A crash is a 2 second wipeout with a funny animation, then the rider is back on and rolling. No ragdoll physics that leave the player stuck.
- Steering is snappy. Turning radius tightens as speed drops. At top speed a hard turn drifts the rear wheel with a skid mark and a sound, and the drift is controllable.
- Generous forgiveness: NPCs get a small push-away radius so glancing contact nudges rather than crashes. Only a direct hit at speed causes a wipeout.

### Rider

A rigged human on the bike at all times when riding, with pedaling animation, cosmetic lean, and a wipeout animation. Casual Atlanta clothes: Braves cap, Falcons jersey, or Hawks shirt. Let the player pick one on the title screen.

### E-bike gears and controls

- Up arrow shifts up, Down arrow shifts down. 5 gears. Low gears accelerate fast, high gears reach top speed. The HUD shows the gear.
- Hold W to pedal. Release to coast.
- Hold Space to brake. Tap Space while turning at speed for a power slide.
- Left and Right arrows steer and lean.
- E gets off the bike. E gets back on when next to it.
- Tab switches chase camera and handlebar camera.
- Esc pauses.
- A nitro boost meter fills from near misses with NPCs and enemy kills. Shift fires the boost for 3 seconds of top speed with a whoosh and motion blur.

## Getting off the bike: first person shooter mode

Pressing E while riding stops the bike, the rider dismounts, and the game switches to first person. The bike stays where it was with a marker on the HUD and the radar. Press E next to it to remount and the game returns to the bike view.

### FPS controls

- WASD move, Shift sprint, Space jump, mouse look, left mouse fire, right mouse aim, R reload, 1 through 4 select weapon, F melee.
- Movement is fast and floaty in the arcade sense. No stamina.

### Weapons

Weapons are picked up from glowing crates placed around the park and the trail, and dropped by enemies. The player starts with the pistol.

1. Pistol, infinite ammo, medium damage.
2. Shotgun, close range, big knockback. Great in the tunnel.
3. SMG, fast, sprays.
4. Frisbee launcher, the joke weapon, ricochets off walls and can hit multiple zombies. Ammo comes from the frisbee players on the Meadow.
5. Melee: a bike U-lock on a chain.

Shooting must feel good: hit reactions, muzzle flash, tracers, screen shake, hit markers, a satisfying crunch on a zombie headshot, ammo count on the HUD.

### Shooting from the bike

While riding, the player can fire the pistol one handed with the left mouse button in the direction the camera is facing. It is inaccurate at speed. Getting off the bike is the way to actually clear a zone.

### Health

A health bar. Enemies do damage. Health regenerates slowly when not taking damage. Health pickups are cans of Soda Pop, because Atlanta. Health hitting zero is a wipeout and a respawn at the last checkpoint with 10 seconds off the clock.

## Enemies

### Zombies

Zombies pop up out of bushes, from behind dumpsters, from under the Ponce bridge, and out of the tunnel walls. They are undead, twitchy, strung out, and clearly not human: green-gray skin, glowing eyes, ragged clothes. Play them for laughs. They are cartoon zombies, never a depiction of a real person.

- They say weird stuff when they spawn and when they chase, in a voice line pool of at least 30 lines: "You got a charger?", "Is this the BeltLine or the Beltway?", "I used to live in Buckhead", "Braves lost again", "The Marta is coming", "Brains... but make it vegan", "This used to be a Kroger", "I'm not even from here", "Ponce is a state of mind", "Do you know what time it is? I need to be at Chick-fil-A". Write the rest in the same voice.
- Most shamble. Some sprint. Sprinters shriek before they charge.
- They chase the player on the bike and can grab the rear wheel to cause a wipeout if they reach it. They are slower than a bike in top gear on flat ground, so a moving player is mostly safe. A stopped player is not.
- Headshots pop. Bodies dissolve after 5 seconds.
- Zombie density: light on Easy, moderate on Medium, heavy on Hard. On Hard there are zombie waves in the tunnel.

### Hostile shooters

Armed humans, gang style, who stand on rooftops, bridges, and behind cars and take shots at the player. They telegraph with a laser sight and a shout before firing. They are on Medium and Hard only. The Murder Kroger plaza always has at least one on Medium and three on Hard.

### Illegal e-bikers

Riders on hyped-up, throttle only, way-too-fast e-bikes with no helmets and loud speakers on the handlebars. They ride against traffic, weave, and ram the player. On Hard some of them are armed. Hitting one is a wipeout for both of you. They can be shot off their bikes.

### Rogue scooters

E-scooter riders in three speeds. Some swerve, some ride wrong way, some are drunk. On Hard they come in packs.

### The Murder Kroger knife guy

Stands at the Kroger plaza. Lunges on Medium. Chases on Hard. Can be shot.

## Difficulty

| | Easy | Medium | Hard |
|---|---|---|---|
| Timer | 15 minutes | 10 minutes | 5 minutes |
| Crowd density (walkers, joggers, cyclists, skaters, pets and owners, frisbee, chillers) | Light | Medium | Packed |
| Scooters | Slow, a few | Mixed, more | Fast, packs, wrong way |
| Illegal e-bikers | None | A few | Many, some armed |
| Zombies | Rare, slow | Regular, some sprinters | Constant, waves in the tunnel |
| Hostile shooters | None | Some, laser sight warning | Many, less warning |
| Murder Kroger knife guy | Absent | Lunges | Chases |
| Weapon crates | Many | Some | Few |
| Radar range | Long | Medium | Short |

Every one of these is a variable in a difficulty data table so it can be tuned without code changes.

## The Artifact and the radar

The Artifact is a themed Atlanta object hidden at a random path location in the park, never on the direct line to the exit. Per level it can be different: the player's lost phone, a Waffle House waffle, a Big Chicken figurine, a MARTA Breeze card. It glows and floats.

The radar is a circular minimap in the corner of the HUD showing the path network outline, the player as a triangle, the bike when dismounted, nearby enemies as red dots, and the Artifact as a pulsing gold blip. The blip only appears when inside radar range, and radar range depends on difficulty. Outside range, an arrow on the radar rim points the general direction and the pulse speeds up as the player gets closer. After pickup the radar shows the route home with a gold line and the arrow points to the next checkpoint.

## Game flow

1. Title screen. Logo: BATTLE FOR THE A in big Atlanta-style lettering (think Falcons red and black with a peach). Background: a slow camera pan across Lake Clara Meer to the skyline at golden hour with the drum circle audible. Menu: START, INSTRUCTIONS, LEVEL SELECT, OPTIONS, QUIT. Music: Atlanta style trap beat, original, looping.
2. INSTRUCTIONS: a screen with a keyboard diagram for bike mode and FPS mode, the objective in two sentences, the enemy roster with pictures, and the difficulty table. Reachable from the pause menu too.
3. LEVEL SELECT: Easy, Medium, Hard as three cards showing the timer and a one-line warning ("Just a bike ride", "They're waiting at Kroger", "Good luck"), plus the best time saved for each. Rider outfit choice here.
4. START drops the player at the 14th Street Gate on the bike. Three-two-one, go. On Easy the first 20 seconds show control prompts that clear as the player uses them.
5. Find the Artifact using the radar. Pick it up. Radar switches to route home.
6. Ride to the BeltLine exit, through Murder Kroger (checkpoint), past Ponce City Market, Fourth Ward Park, Krog Street Market (checkpoint), through the tunnel, to the gate.
7. Win screen: time remaining, kills, wipeouts, top speed, near misses, a letter grade, and a "BATTLE WON" stamp. Loss screen if the timer hits zero or if the player quits: "THE A WINS THIS TIME" with a retry button.
8. Best times saved per difficulty.

## HUD

- Timer, top center, big, turns red under one minute.
- Radar, bottom right.
- Gear and speed in bike mode; weapon, ammo, and crosshair in FPS mode.
- Health bar and nitro meter, bottom left.
- Objective text at the top: "Find the Artifact", "Get home", "Checkpoint: Murder Kroger".
- Zombie voice lines appear as subtitles so the jokes land.

## Audio

Park ambience, crowd chatter, the drum circle and DJ as 3D sound sources, saxophone at the gate, bike freewheel and e-motor, tire noise on different surfaces, skid, boost whoosh, weapon sounds with punch, zombie voice lines, shooter warning shouts, scooter whine, illegal e-bike speaker playing muffled music, tunnel echo, and a driving original soundtrack that gets more intense on the BeltLine and in the tunnel.

## Technical

- Unreal Engine 5.4 or newer. Blueprints for gameplay and UI, C++ for the bike movement component, the NPC spawner, and the difficulty table if needed.
- Plugins: Water, PCG, Landscaping or Cesium for reference, Metahuman.
- NavMesh AI controllers for enemies. Mass AI or instanced animated crowds for the background NPCs so hundreds of people can be on screen.
- One persistent level with the park, the BeltLine, and Cabbagetown as World Partition cells.
- Folder structure: Content/BattleForTheA/Maps, Bike, Rider, Weapons, Enemies, NPC, Environment/Park, Environment/BeltLine, Environment/Cabbagetown, Landmarks, UI, Audio, Materials, Data.
- Reference photos live in a Reference folder beside the project. Use them.

## Build order

Confirm each milestone with a playable test before moving on.

1. Arcade bike on a test map with hills, curbs, grass, stairs, a lake, and 50 wandering NPCs. Prove the bike cannot fall over from terrain and that wipeouts recover in 2 seconds. Add dismount, FPS movement, and the pistol on the same test map. This milestone alone should already be fun.
2. Park landscape from real elevation, path splines from OSM, lake, 14th Street Gate start, skyline. Trees and grass via PCG. This is the milestone where the park has to look like the photos.
3. Park life: every NPC group listed above, music, frisbee, dogs, chillers.
4. BeltLine, Murder Kroger, Ponce City Market, Fourth Ward Park, Krog Street Market, the tunnel, Cabbagetown.
5. Enemies: zombies with voice lines, shooters, illegal e-bikers, scooters, knife guy. Weapons and crates.
6. Game loop: title screen, instructions, level select, timer, Artifact spawn, radar, checkpoints, win and loss screens, saves.
7. Difficulty data table and tuning. Audio and soundtrack. Performance pass to hold 60 fps at 1080p on a mid-range GPU.
8. Package as a Windows shipping build named BattleForTheA.exe with a custom icon, and create a desktop shortcut. Double-clicking the icon must land on the title screen.

## Acceptance test

- Riding across grass, roots, curbs, and stairs at full speed never tips the bike.
- Pressing E gets off the bike into first person, shooting works and feels good, pressing E by the bike gets back on.
- Zombies pop up, say weird things with subtitles, chase, and can be shot.
- A screenshot from the 14th Street Gate looks like the reference photo. A screenshot across the lake looks like the skyline reference photo.
- The Meadow has frisbee, blankets, dogs, and music playing when the player rides past.
- The title screen has Start, Instructions, Level Select, Options, and Quit, and all of them work.
- The radar leads the player to the Artifact and then home.
- Easy, Medium, and Hard visibly change crowd size, enemy count, and timer.
- The game launches from a desktop icon into the title screen with no editor open.
- Someone who plays it once wants to play it again.