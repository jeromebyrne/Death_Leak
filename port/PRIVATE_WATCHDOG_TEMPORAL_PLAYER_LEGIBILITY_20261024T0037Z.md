# Temporal player legibility: corrected 13-second gameplay clip

## Finding

**Player readability changes with movement and foreground overlap; the sampled clip does not prove a port defect.** Reviewed existing decoded stills from the privately delivered corrected-fade candidate `port/build/private-corrected-fade-active-gameplay-20261023t2231z/corrected-active-13s.mp4` (SHA-256 `93f4ff3fec07002db491951b3bda2cffdfa0871168040759d73776dd9ab8e4d2`, per its capture/delivery reports). No recapture.

- Forest 1 s: the running player is distinct on the brighter bridge, approximately x640–720/y408–513. At 5 s his upper body remains visible near x660–720/y483–563 while legs merge with the dark floor/foreground. At 7 s near x720–800/y490–565, upper-body separation is weak against the dark central trunk/veil and lower-body tracking is harder. By 8 s near x930–1010/y482–565, he is more separable against a lighter right-side patch. These are approximate visual extents, not segmented masks; camera, player pose, and cover move between samples. At 9 s he reaches the right image edge, adding framing loss.
- Waterfall-top 10.1 s: a large foreground tree/blur dominates the center and the player is not identifiable in that transition-adjacent still. At 10.5 s he is clear on the left (roughly x80–165/y225–324), before the center trunk. At 11.5 s he remains visible around x230–312/y248–337 and approaches its left edge. At 12.8 s he is not identifiable while the center trunk still occupies roughly x300–550. Rightward traversal into cover is plausible, but the MP4 cannot establish the exact player position, covering ID, or whether ID2 was emitted at 12.8 s. Persistent HUD and changed composition do not suggest a frozen scene.
- The 9.7–9.9 s black interval is the documented corrected loading-fade opaque hold, not a player-legibility failure. Video seconds are wall-clock samples, not simulation/render ordinals.

## Source-backed contract and limitations

Forest `Game/XmlFiles/levels/forest_exploration_1.xml:33-46,176-190,612-635` authors player ID2 at `kPlayer`, alpha 1; ID10 tree blur at `kMiddleForeground`, alpha .7, fade disabled; ID37 lower shrubbery at `kMiddleForeground`, alpha .95, fade disabled. The prior same-state ID10/ID37 gates concern **other** Linux frames: they support plausible authored attenuation, not exact attribution for this video. The Forest 7→8 s recovery may reflect changing overlap/background contrast, not an animation/fade fix.

Waterfall `Game/XmlFiles/levels/waterfall_top.xml:33-47,441-467,506-558` authors player ID2 at `kPlayer`, alpha 1; large ID23/24 `tree2.png` at `kGroundFront`, alpha .95/1 with occlusion fade enabled (target .5); and ID27–30 `tree2_blur.png` foreground trees at alpha .95/.95/.95/.7 with fade **disabled**. Foreground player hiding is an expected composition possibility. The saved still does not identify which tree, if any, covers the player at 12.8 s. A fade-disabled overlay should not be called a missing fade from appearance alone.

Original and port source paths sort descending depth, drawing player before foreground; conditional fade is gated by the authored XML flag, per `PRIVATE_WATCHDOG_FOREST_PLAYER_BODY_OCCLUSION_SOURCE_GATE_20261024T0025Z.md`. This lane did not replay commands or quantify pixel attenuation. H.264, sepia/film phase, motion, and absent player-only masks make cross-time RGB differences non-causal. No same-state original-Windows frame or visual acceptance is available; no parity claim follows.

## Next discriminating gate

At the 12.8 s-type Waterfall pose, obtain **one same-render-ordinal lossless Linux draw packet**: player world/screen bounds, body/arm frame and ID2 emission, camera, ordered foreground IDs/UV/alpha/fade state, and pre/post-tree and final pixels. This separates player-not-drawn/off-screen from authored tree occlusion. If a first divergent command/pixel stage emerges, compare a qualified same-state original D3D10 packet before correcting code. A separately registered Forest 7 s overlap packet is needed for upper-silhouette parity. No opacity, animation, or order edit is justified by these wall-clock frames.

Read-only visual/source lane. Only this unique report was written; shared status, source/tests, build/GPU, index, and unrelated dirty work were untouched.
