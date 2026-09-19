# The Battle of ATL — third-party art credits

Maintained by Web Experts. This file exists because a distributed build has to
carry the attribution its licences require, and because a required credit that
lives only in a design note is not a credit.

The game also shows a short credit line in the pause menu (Esc > Home). Keep the
two in step: if an asset in the **Required attribution** table below is in a
shipped build, its credit line must be visible in that build.

## Required attribution

| asset | where it is used | author / source | licence | requirement |
| --- | --- | --- | --- | --- |
| **Low Poly Bear** (`SourceAssets/Spirit/BearCandidate/Bear.fbx`) | the spectral black bear spirit, `ABattleSpirit`'s 3D body (`SM_SpectralBear`) | Mathilde_Lea, OpenGameArt, <https://opengameart.org/content/low-poly-bear> | **CC-BY 4.0** | **Attribution required.** Credit the author, the work, the licence and a link. Shown in the pause menu and here. |
| **Remington Model 870** (`SourceAssets/Weapons/Remington870`) | *candidate only — not equipped in any shipped build* | britdawgmasterfunk, Sketchfab, <https://sketchfab.com/3d-models/remington-model-870-cc0-d9a309a2f57b4a87ac38cc440a3deaa7> | **CC-BY 4.0** (the listing says CC0; the download archive's own licence file says CC-BY) | If this asset is ever shipped, add its credit line here and in the pause menu first. |

## Credited, no attribution required

These are CC0 or first-party. They are listed so the provenance of every shipped
asset is answerable from one file.

| asset | used for | source | licence |
| --- | --- | --- | --- |
| Quaternius **Ultimate Guns Pack** (`Pistol_1.fbx`) | the pistol | <https://quaternius.com/packs/ultimategun.html> | CC0 |
| Quaternius **Zombie Apocalypse Kit** (`Rifle.gltf`, `SMG.gltf`) | rifle and machine gun, riding and on foot | <https://quaternius.com/packs/zombieapocalypsekit.html> via the public CC0 mirror `agentkaerf/FreeModels` | CC0 |
| Quaternius **Ultimate Modular Men** (`Casual.fbx`, `Swat.fbx`, `Farmer.gltf`, `Punk.gltf`, `Animations.fbx`, `FPSArms.fbx`) | Ellison, the police, and the walking crowds | <https://quaternius.com/packs/ultimatemodularcharacters.html> | CC0 |
| Epic **CitySample** crowd content (`Content/CitySampleCrowd`) | city crowd meshes and atlases | Epic Games | Unreal Engine EULA |
| Unreal Engine **starter and basic shapes** (`/Engine/BasicShapes`, `/Engine/EngineMaterials`) | props, primitives, text | Epic Games | Unreal Engine EULA |

## First-party

Everything else in the game is original work for this project and needs no credit
line: the skatepark and its geometry, the Krog Street Tunnel shell and
cut-and-cover gallery, the Eastside Trail and BeltLine pavement, the memorial
flowers, the weapons' iron sights and the knife, the HUD, and all game code.

## Per-directory provenance

The detail — retrieval dates, public URLs, hashes, adaptations and the reasons an
asset was chosen over another — lives next to the assets, not here:

- `SourceAssets/Weapons/SOURCE.md` and `SourceAssets/Rider/SOURCE.md`
- `SourceAssets/Spirit/BearCandidate/SOURCE.json`
- `Design/FAB-ASSET-QUEUE.md` (the route used when the Mac is locked: the CC0
  GitHub mirror and OpenGameArt serve files over plain HTTP with no login, where
  Fab and Sketchfab need a signed-in browser session)
