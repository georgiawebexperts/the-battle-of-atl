
## 2026-09-11 [codex-maclaptop] — Source hierarchy correction and free assets

Native walking review reproduced backward shoes (idle toe dot L -0.8305, R -0.8851). Autodesk FBX SDK inspection confirms Animations.fbx parents Foot.L/R to Root, while Casual.fbx parents them to LowerLeg.L/R. Sharing a skeleton does not convert source-local animation tracks to the target parent frame. The runtime now converts those legacy foot rotations through the authored Root into the target lower-leg frame, preserving ankle motion. Native glTF clips skip this correction.

Built/cooked successfully; native walking test now passes (idle L .5181, R .9933, support gap 1.823 cm). Captures in work/build044-walking-review show forward shoes and soles down for idle/walk/run. The idle pose is still awkward and the overall style is not final. The test now rejects reversed idle feet instead of relying solely on ground height, knee motion and travel. Desktop installation remains build 043.

Game Animation Sample UE 5.8 downloaded and installed at /Volumes/Adam Assets/Unreal/GameAnimationSample (about 7 GiB before other packs). City Sample Crowds 5.3 package downloaded into that separate project; compatibility with the game is untested. European Hornbeam 5.6 package download initiated into the same project. No paid acquisition. Mixamo exports have not yielded a verified local FBX; curated movement/combat/recovery/zombie/ambient/dance selection is authorized, not the full catalog.
