# Undead appearance — build 038

2026-09-11 [codex-maclaptop]

Two full rigged silhouettes replace the repeated Casual model: the creator's Farmer (hat, shirt, overalls and boots) and Punk (vest, tall hair and different clothing). Both use separate pale skin, weathered clothing and dark footwear materials. The old full-body green override is removed. Geometry comes from the CC0 Quaternius Ultimate Modular Men pack, obtained through its public mirror when the creator's Drive download quota prevented access; provenance is in SourceAssets/Rider/SOURCE.md.

Four modular mesh nodes are merged only after checking identity transforms and shared skin. Original glTF files stay intact. Each character uses its own matching glTF idle/walk/run clips; applying the old FBX clips distorted the different bind orientations and was rejected in native review. The shared animation sampler remains; pedestrians and police keep their previous clips. The old detached glowing-eye primitives are hidden in favor of the model's own eyes.

These are stylized character variants, not final GTA-level art. Clothing still needs actual torn/frayed geometry, more lived-in accessories and a stronger undead performance. Attack, fall and death animation remain unfinished; no final art or full animation acceptance is claimed. The old dissolve material is no longer used; corpses still expire on the existing timer.
