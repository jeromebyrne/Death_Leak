# Private sea_2 distinct visual source gate — authored AmbientBird IDs 15/16

Read-only audit, 2026-10-24 01:01 UTC. This report is the only write. No GPU, Docker, compile, game run, source/status edit, or parity claim. Existing sea_2 film/bar reports and village endpoint reports cover separate issues; a bounded search of `port/*.md` found no AmbientBird or sea_2 bird report.

## Source-proven disparity

`Game/XmlFiles/levels/sea_2.xml:286-314` authors `AmbientBird id="15"` at (-8137.453125,-704.309814), alpha .4, kFarBackground, animated via `XmlFiles\bird_anim.xml`, with `travel_offset=5000`; `:315-343` authors ID16 at (-4654.958496,-1021.309814) with the same visual/animation/travel contract. The initial player is at (-4208.853516,-1028.309814) (`:57-59`), placing ID16 about 446 world units left of the player before camera/crop effects. `Game/XmlFiles/bird_anim.xml:5-15` distinguishes one-frame Still (`ambient/2.png`) from three-frame Running (`ambient/1-3.png`).

On original Windows, `Game/src/GameObjects/AmbientBird.cpp:34-50` initializes travel direction and start position; `:101-110` calls NPC update and accelerates X each active update; `:112-163` conditionally resets after authored travel only when off-view/far from player; `:165-187` alternates flap/glide state, and `:52-83` selects Running at 9 FPS versus Still at 1 FPS. This is not a stationary decorative sprite.

Linux `port/game/LevelScene.cpp:373-408` admits an animated object with a texture and creates a generic `SceneSprite`; its only special animation-sequence load branches at `:409-489` are Player and CurrencyOrb. It never reads `travel_offset` (no occurrence in `port/game/LevelScene.cpp`), and the update loop at `:882-960` handles parallax, scrolling, occlusion, and already-loaded animation frames, with no AmbientBird travel/flap state. Draw at `:1383-1392` therefore uses the generic static position/first animation texture for IDs 15/16. This is a definite source-command omission for an authored sea_2 object, independent of the final-HUD film selector and bars. Source alone does not prove ID16's bird pixels are exposed in every final-scene frame: kFarBackground occlusion, camera state, and animation phase matter.

## Next isolated gate

After the Forest ID47 owner releases source/build/GPU, first use a CPU recording scene test or read-only loader probe to assert sea_2 IDs 15/16 are admitted with authored location, alpha, depth, animation path and travel offset, then show that a controlled positive update produces original-directed X motion and Running/Still texture transitions while an unchanged-camera Linux baseline does not. Patch only an AmbientBird-specific scene path, preserving other animated entities and authored ordering; model source reset and flap timing before capture. For visual priority, use a state-matched original-Windows sea_2 frame/trace and lossless Linux frame around visible ID16, with camera and postprocess/HUD stage recorded. Do not infer original pixel parity or acceptance from source inspection.

Read-only ownership released; no implementation or GPU lane reserved.
