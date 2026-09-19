# Free asset integration queue

2026-09-11 [codex-maclaptop]: Elliott explicitly authorized downloading any free assets that help the game. Curate for the requested realistic Atlanta setting and Mac compatibility. No paid purchase authorized. Fab signed-in library inspected at https://www.fab.com/library.

Downloaded to external staging project /Volumes/Adam Assets/Unreal/GameAnimationSample/Content:
- M1911 (FREE), Xeradev: M1911/ (30 uassets, 36,200,858 bytes). Rigged and static pistol, magazine/ammo, PBR materials. https://www.fab.com/listings/77247d26-33a3-4dba-a840-7a7d8c92bb9c
- Qi Gong animations, Motion Cast: Qi_Gong_animations_Pack/ (49 uassets, 87,969,587 bytes). 22 exercise animations; seller lists 18 loops. Fit for background park exercise, requiring retargeting. https://www.fab.com/listings/95351f6b-3c56-4405-a35a-1e170a1913c2
Neither is migrated to the actual game or gameplay-verified yet. Launcher downloads completed and files exist.

Other newly observed library additions, not downloaded by this agent:
- Fire Effect VFX; Dreamcore Music 3; Material system with physics.
- Garden Foliage, Stinging Nettle, Wood Anemone, Dead Pine, Ginkgo; FREE Stylized Foliage Pack. Prefer realistic species and inspect performance. Garden Foliage listing describes tropical/stylized plants, lower priority for the main park.
- Low Poly Weapons Set, EasyLocomotionSystem, LCAT: Locodrome Character Animation Toolkit, Advanced Networked Shooter System (Core Plugin) V1.0.
LCAT is an editor Control Rig animation tool, potentially useful for cinematics, not a runtime locomotion pack. Shooter core lists C++/HUD/recoil/ammo systems, but weapon models, combat animations, audio and effects are external demo content; verify Mac support before use and avoid replacing working gameplay without a scoped integration test.

Sources: https://www.fab.com/listings/343098bf-bcfa-4d3a-8260-96e9c77b8635 ; https://www.fab.com/listings/0a897de8-58de-48f6-bc8a-cc343fdb5483 ; https://www.fab.com/listings/6fbe6321-4915-49c5-8350-fc7e04709376 .


2026-09-11 [codex-maclaptop]: Added Epic City Sample Vehicles (free) to the Fab library; product page now shows View in My Library and View in Launcher. Listing includes 13 vehicles and Mac as a target platform, package versions UE 5.0–5.3. Actual UE 5.8/Mac integration remains unverified. https://www.fab.com/listings/2909157b-ddfa-4cef-a925-69dc2467021f . Download not started: native UI unavailable while Mac locked. Do not count library acquisition as downloaded content.


2026-09-11 [codex-maclaptop]: First verified Mixamo motion acquired: Sleeping Idle (Sleeping Deeply), Default Character, FBX Binary with skin,30fps, no keyframe reduction,207 displayed frames. SourceAssets/Mixamo/SleepingIdle_WithSkin.fbx (2,323,904 bytes, provenance JSON beside it). Imported reference mesh/skeleton/animation at /Game/BattleRetarget/Mixamo/SleepingReference using Scripts/import_mixamo_sleep_reference.py; not retargeted or wired to gameplay. Native Launcher remains locked; Mixamo browser session works.

2026-09-13 [codex-maclaptop]: M1911 migrated into actual game Content/M1911 (28 dependency packages, inspect/copy reports). Rigged pistol and magazine motion now in opt-in detailed player preview; native control/ammo/magazine checks pass and final reload render inspected. Not installed/default yet; continuous hand contact, muzzle alignment, interruptions/performance remain pending. Qi Gong remains staged only.

2026-09-19 [codex-maclaptop]: Elliott asked "cant you find assets like you did last time?"
after the machine gun still had no mesh of its own. Second successful route, and the
one to prefer while the Mac is locked: the CC0 mirror repo `agentkaerf/FreeModels`
holds Quaternius packs as plain glTF over raw.githubusercontent.com, with no login,
no Launcher and no browser session. Downloaded **SMG.gltf** from the same Zombie
Apocalypse Kit the rifle came from (174 KB, self-contained, sha256 in
SourceAssets/Weapons/SMG-SOURCE.txt) and wired it to slot 2 on the bike and on foot.

The browser routes remain blocked, not refused: the Fab library and Sketchfab
downloads need Elliott's signed-in session, and the Codex browser bridge reports no
auth token while the Mac is locked. Ask again with the Mac unlocked and the same
free-asset sweep can continue from the queue above.

What that mirror does **not** have: a bear. Its animal models are wolf, fox, husky,
deer, stag, cow, bull, donkey, alpaca, horse and shiba inu (Ultimate Animated
Animals), plus cat, dog, horse, pig, raccoon, sheep, wolf, chicken and chick (Cube
World). The spectral black bear therefore still needs the Sketchfab lead already
recorded in Design/BLACK-BEAR-SPIRIT.md, a Fab listing, or a paid purchase - all of
which need the browser session. Do not substitute a wolf or a low-poly stand-in:
that note explicitly forbids it.
