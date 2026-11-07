# Rotated repeat-X resized/letterboxed physical-GPU gate — 2026-10-24 00:07Z

## Verdict

**PASS for the bounded Linux branch gate.** The prior frozen executable's ID17 rotated repeat-X path ran on physical Xorg `:0`/Mesa nouveau NVE7 in a 1000×700 window, with SDL's 1280×720 logical output letterboxed to GL viewport `(0,69,1000,562)`. A target-coordinate GL spy measured identical viewport, scissor box, and scissor enable state immediately before the private draw's attribute push and after its restoration. The same executable and same scripted state, with only `DEATHLEAK_TEST_DISABLE_ROTATED_REPEATX=1` changed, produced a localized pixel delta and unchanged later-composited controls. This does **not** assert Windows parity or visual acceptance.

## Source-backed method and exact commands

- Source `port/linux/SDLRenderer.cpp:137` calls `SDL_RenderSetLogicalSize(renderer_,1280,720)`. The private branch at lines 247–256 selects `DrawRotatedRepeatX`; lines 300–356 flush SDL, push GL attributes/matrices, draw the quad, restore matrices/program/binding/attributes/active texture. `port/linux/main.cpp` records a final `--capture` after painter, postprocess and HUD. The atomic packet writer assumes a 1280×720 BMP, so this resized test intentionally used non-atomic `--capture`; no schema/matched-Windows claim is made.
- Existing frozen executable: `port/build/private-rotated-repeatx-20261024t0000z/deathleak-linux`, SHA-256 `025576656d9f560c3aa687490b9f9e3b1a3076f8f3dbdf4fce899d88d9e27c73`. A temporary, ignored `LD_PRELOAD` shim in `port/build/private-rotated-repeatx-resize-gate-20261024t0007z/resize.c` changes only the `SDL_CreateWindow` request from 1280×720 to 1000×700. `resize_state.c` adds a GL spy triggered by ID17's logged U `3.238435`; both were compiled `gcc -shared -fPIC -o <name>.so <name>.c -ldl`. No shared source file or product binary was changed.
- Preflight: `DISPLAY=:0 XAUTHORITY=/tmp/deathleak-current-xauth xrandr --current` reported physical screen 1440×900; `ps`/`docker ps` had no Death Leak capture process/container. Captures ran serially from repository root, each exiting 0:

```sh
DISPLAY=:0 XAUTHORITY=/tmp/deathleak-current-xauth LD_PRELOAD="$PWD/port/build/private-rotated-repeatx-resize-gate-20261024t0007z/resize.so" DEATHLEAK_TRACE_ROTATED_REPEATX=1 SDL_AUDIODRIVER=dummy port/build/private-rotated-repeatx-20261024t0000z/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --fixed-dt 0.016666667 --frames 425 --capture port/build/private-rotated-repeatx-resize-gate-20261024t0007z/enabled.bmp > port/build/private-rotated-repeatx-resize-gate-20261024t0007z/enabled.log 2>&1
DISPLAY=:0 XAUTHORITY=/tmp/deathleak-current-xauth LD_PRELOAD="$PWD/port/build/private-rotated-repeatx-resize-gate-20261024t0007z/resize.so" DEATHLEAK_TEST_DISABLE_ROTATED_REPEATX=1 DEATHLEAK_TRACE_ROTATED_REPEATX=1 SDL_AUDIODRIVER=dummy port/build/private-rotated-repeatx-20261024t0000z/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --fixed-dt 0.016666667 --frames 425 --capture port/build/private-rotated-repeatx-resize-gate-20261024t0007z/disabled.bmp > port/build/private-rotated-repeatx-resize-gate-20261024t0007z/disabled.log 2>&1
```

- The third run used the same executable and script with the state spy:

```sh
DISPLAY=:0 XAUTHORITY=/tmp/deathleak-current-xauth LD_PRELOAD="$PWD/port/build/private-rotated-repeatx-resize-gate-20261024t0007z/resize_state.so" DEATHLEAK_TRACE_ROTATED_REPEATX=1 SDL_AUDIODRIVER=dummy port/build/private-rotated-repeatx-20261024t0000z/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --fixed-dt 0.016666667 --frames 425 --capture port/build/private-rotated-repeatx-resize-gate-20261024t0007z/targetstate.bmp > port/build/private-rotated-repeatx-resize-gate-20261024t0007z/targetstate.log 2>&1
```

Its isolated spy wraps `glPushAttrib`, `glTexCoord2f`, `glGetError`, and `glActiveTexture`, triggering on U 3.238435. `targetstate.bmp` was pixel-identical to `enabled.bmp`; the spy did not perturb the image.

## Measured evidence

- Both runs logged `video_driver=x11 name=opengl accelerated=1 software=0`, `vendor=Mesa renderer=NVE7`, the Forest→Waterfall trigger at frame 398, and the same final simulation state `player=(-2139.01,3114.7), orbs=7, health=25, focus=100, fired=25`. Enabled logged `Rotated repeatX texture=244 source=734x1024 u=3.23843->0 angle=5.15393 backend=private-gl drawError=0`; disabled logged no rotated-repeat branch.
- Target spy: `before viewport=0,69,1000,562 scissor=0,0,1000,700 enabled=0`; `after viewport=0,69,1000,562 scissor=0,0,1000,700 enabled=0 match=1`. The source has no viewport/scissor mutation inside this branch; the captured state proves the target draw preserved SDL's letterboxed viewport and the existing scissor configuration. Scissor *enabled* restoration was not exercised because SDL had it disabled in this state.
- PIL `ImageChops.difference` over RGB 1000×700 BMPs: **8,985 changed pixels**, inclusive bounding box x=0..993, y=233..361. At `(947,328)`, enabled RGB `(79,69,50)` versus disabled `(32,27,19)`. Every pixel in y=0..232 and y=362..699 matches, including upper/lower matte and later-painter/HUD sample `(500,500)=(15,15,15)` in both. The enabled branch has no observed corruption outside its visible composite footprint. Prior nominal-size pre-HUD attribution remains in the implementation report; this resized capture is final-composite only and cannot assign every changed pixel to ID17 because the opt-in switch also affects any other eligible rotated repeat-X sprite.
- SHA-256 BMP: enabled and target-state `c04a8053c460419232fdb05a59a73b874fceb7c2ca3da8349fe48b531a8ef6bf`; disabled `bc5b4d20ab4d4110e58747fd192dce9abe4aeb0715258b92a4713b00faa6fa09`.

## Scope/release

No defect was found in this bounded gate; no product software edit/build, commit, push, shared status/checkpoint, or unrelated source change was made. Only this report and ignored local harness/capture files were written. Postflight `ps`/`docker ps` showed no game, FFmpeg or capture process/container. **Exclusive physical Xorg :0 GPU/build lane released.**
