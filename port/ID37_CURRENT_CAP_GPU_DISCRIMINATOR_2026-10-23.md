# ID37 current-cap physical-GPU discriminator — 2026-10-23

## Bounded conclusion

At the frozen current-cap Linux binary, **ID37 causes the gross dark near-ground foreground** in a matched forest-1 frame. Hiding only XML ID37 removes the irregular black shrub silhouette and reveals the existing ground/trees beneath. The same run/state pair changes 90,951 RGB pixels, all in `[0,596,1280,720)` (half-open). The silhouette begins sparsely near y596 and occupies almost the whole width by y719; it is not a full-screen darkening or a sharp horizontal renderer rectangle. This is consistent with the original XML's alpha-0.95 black `shrubbery.png` foreground and its source-defined transparent-to-opaque vertical orientation. **No evidence in this bounded capture makes the gross lower obscuration a remaining Linux renderer-contract mismatch.** It does not prove exact Windows compositing/texture parity or visual acceptance.

The inspected latest current-cap clip's 1/5/9-second frames retain a dark, irregular bottom shrub layer as the player/camera move. Steam forest screenshot 14 also has a dark, irregular foreground above its bottom black matte, but differs in camera, player/orb state, viewport/presentation and effect phase. Its bridge, large trunk, haze and shrubs are useful qualitative landmarks only. The Linux image looks lower/darker in some regions; unmatched compressed store imagery cannot isolate ID37 as the reason. Any pale mid-ground/right-field composition issue is outside this discriminator.

## Reproducible capture / identity

- Working tree was already dirty; no build process was running. HEAD `22950f87a0c37207f234b859506759ec0c3f3ca3`. Before either game run, copied `port/build/deathleak-linux` to ignored `port/build/id37-current-cap-gpu-discriminator-2026-10-23/deathleak-linux.frozen`. Source and frozen binary SHA-256 before capture, and shared/frozen binary after, all `70739562d96395f4731674b0ad64a356e6d94f20d18d306bed9ff40269ebbacb` (8,985,328 bytes). This is HEAD plus pre-existing uncommitted work, not a clean-HEAD build; no rebuild occurred in this lane.
- Each variant ran sequentially in `deathleak-linux-build:bookworm` with repository mounted read-only, ignored evidence output mounted writable, `/dev/dri`, Xorg socket and LightDM `:0` authority. Physical display only: `DISPLAY=:0`, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, `SDL_AUDIODRIVER=dummy`. Both logs identify accelerated strict SDL OpenGL (`accelerated=1 software=0`), `nouveau / NVE7`, initialized GPU `Sepia.fx`, 71 original sprites and zero missing/unsupported assets. No other game/capture/build ran in this lane.
- Shared deterministic invocation: `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667 --capture /out/{variant}.bmp`. Baseline had `DEATHLEAK_RENDER_HIDE_XML_IDS` empty; hide variant set it to `37`. This filter is render-only. Both endpoint lines agree on all printed gameplay/state fields: player `(1368.99,3107.3)`, five orbs, health 25, focus 100, balance 5, grounded 1, vx 480, no shots/hits/kills. Only `elapsed` differs (`4.05773s` vs `3.99359s`), being wall-clock runtime, so complete endpoint-line byte equality is **not** claimed.
- Exact successful command pattern (run once with `variant=baseline; filter=` and once with `variant=hide-37; filter=37`; `out=port/build/id37-current-cap-gpu-discriminator-2026-10-23`):

```sh
docker run --rm --network host --hostname jerome-Linux-MacBookPro --device /dev/dri \
  --mount "type=bind,src=$PWD,dst=/src,readonly" \
  --mount "type=bind,src=$PWD/$out,dst=/out" \
  --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
  --mount type=bind,src=/var/run/lightdm/root/:0,dst=/xauthority,readonly \
  -e DISPLAY=:0 -e XAUTHORITY=/xauthority \
  -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
  -e DEATHLEAK_RENDER_HIDE_XML_IDS="$filter" \
  -w /src deathleak-linux-build:bookworm /out/deathleak-linux.frozen \
  --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 \
  --frames 180 --fixed-dt 0.0166667 --capture "/out/$variant.bmp" \
  > "$out/$variant.log" 2>&1
```

## Pixel evidence and source attribution

Pillow decoded the lossless BMPs as RGB 1280×720, formed `ImageChops.difference`, counted pixels with any nonzero RGB delta, and took the nonzero-mask bbox. Baseline BMP SHA-256 `ff755ee685accafbdfc5990b8f292c9ac61127fae1bd0e238c14ddbdfcf94df6`; hidden BMP `b124dd21802b79803a6c6d7ad4e4b8e581e63f2913aa102a4c4c48694a667405`. Evidence includes both BMPs, viewable PNGs and logs in the ignored directory. Changed-pixel row counts at y590/596/600/620/650/680/700/719 are `0/8/40/156/776/1076/1193/1274`; the natural top is ragged, then black coverage thickens downward. At `(640,700)`, baseline `(8,7,5)` versus hidden `(81,71,51)`; at `(100,700)`, `(7,6,4)` versus `(73,63,44)`. At `(640,590)`, both are `(87,75,54)`.

`Game/XmlFiles/levels/forest_exploration_1.xml:612-635` authors ID37 as `kMiddleForeground`, 2048×512 `shrubbery.png`, alpha `.95`, `followcamx=true`, `followcamy=false`, camera-X UV multiplier `.000690`, no flips. Original `Game/src/GameObjects/ParallaxLayer.cpp:26-100` supplies the followed-X transform and one-span shifted UV, and sets the unflipped Y texcoords; prior source/asset audits in `port/ID37_FIDELITY_DIAGNOSTIC.md` and `port/ID37_LOWER_BAND_SOURCE_DISCRIMINATOR_2026-10-22.md` establish opaque black upper texture rows, transparent lower edge, and source-over blending. Earlier Linux V/UV mapping errors were fixed; this fresh capture validates the present gross contribution, not exact shader parity. A prior-X/current-X follow timing residual may shift the silhouette horizontally during motion by a few pixels; it cannot explain its broad vertical black coverage.

## Next implementation gate

**Do not remove, recolor or attenuate ID37 based on this capture.** Treat the gross lower band as authored unless a synchronized original-Windows lossless forest-1 frame falsifies it. The next actionable ID37 gate is a held camera for at least two object updates, with original Windows and Linux using the same viewport/backbuffer, camera/view matrix, player/effect state, ID37 world transform, UV endpoints, alpha/blend and postprocess boundary. Compare isolated-ID37 alpha/top/edges and identical-background composite at matched state. Only a measured transform/sampler/blend residual should trigger a scoped renderer fix. Steam JPEG or wall-clock MP4 frames cannot satisfy that gate. No source, art, shared status/checkpoint, existing dirty file, commit or push was changed by this lane; this report is the sole non-ignored write.
