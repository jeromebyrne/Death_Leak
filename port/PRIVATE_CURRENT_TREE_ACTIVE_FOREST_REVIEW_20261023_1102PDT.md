# Private current-tree active Forest gameplay GPU review — 2026-10-23

## Result and provenance

**Deliverable:** `port/build/current-tree-active-forest-review-verified-20261023/forest-current-tree-active-10s.mp4` (SHA-256 `1e93bc0e684d50b8cd4d9bebde3678695788723dd039ce1571d2a82ce17b448c`). This is a fresh, direct game-window, 10-second Forest-1 active-play review clip. It begins after the recorder sees both the game window and the authored Forest-1 audio-level entry log, then waits 0.5 s. No menu/startup segment is present in the inspected frames. The capture uses scripted normal input `B:300,F:99700`, fixed simulation dt `0.016666667`, and dummy audio; the video is intentionally silent.

HEAD at the verified build was `515fe235b1b8768142af8ce83536b15f0eff83d1`, plus pre-existing dirty source. Forced, isolated Docker `make -C port -B -j2 TARGET=build/current-tree-active-forest-review-verified-20261023/deathleak-linux all` passed (existing TinyXML warnings only). Frozen binary SHA-256 `fff1afc286879ba8841fa8822c3b9e57eadb8a0d75c55bfc50746303c99fb447`. Source SHA-256 manifests for `main.cpp`, `SDLRenderer.cpp`, `LevelScene.cpp`, and `ParticleSpray.h` are identical before this build, after the build, and after capture (`source-before.txt`, `source-after-build.txt`, `source-after-capture.txt`). No shared production binary or source/art/XML was written by this lane.

An initial candidate in `port/build/current-tree-active-forest-review-20261023t1810z/` was **superseded**, not used for this verdict: another owner's `main.cpp` edit landed between its pre-build manifest and completion, making that binary's source snapshot ambiguous. I rebuilt after the edit and verified stable hashes through the final capture. The superseded candidate must not be presented as current-tree evidence.

## Reproduction command and checks

From repository root, the verified build command was:

```sh
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src deathleak-linux-build:bookworm make -C port -B -j2 TARGET=build/current-tree-active-forest-review-verified-20261023/deathleak-linux all
```

The exact recorder is retained at `port/build/current-tree-active-forest-review-verified-20261023/record.sh`; its host invocation was:

```sh
docker run --rm --name deathleak-current-tree-forest-review-verified --network host --hostname jerome-Linux-MacBookPro --device /dev/dri --mount "type=bind,src=$PWD,dst=/src,readonly" --mount "type=bind,src=$PWD/port/build/current-tree-active-forest-review-verified-20261023,dst=/out" --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority -w /src deathleak-capture:post-foliage sh /out/record.sh
```

`game.log` reports `video_driver=x11 name=opengl accelerated=1 software=0 vsync=1 mode=strict-accelerated`, OpenGL `vendor=nouveau renderer=NVE7`, GPU Sepia.fx scene-before-HUD pass, 71 original Forest sprites and zero missing/unsupported assets. `ffprobe.txt` records H.264/yuv420p, 1280×720, 30 fps, **300 decoded frames / 10.000000 s**, 5,081,707 bytes. Full `ffmpeg -xerror` decode exited 0; `decode.log` is empty. `still-1s.png`, `still-5s.png`, `still-9s.png` are representative frames; consecutive sampled pairs differ by 644,032 and 648,334 pixels above 10 grayscale codes. The orb display moves 2→5; player/camera progress across bridge to ground corridor. This is active gameplay, not a frozen window.

## Visual inspection against saved references

The verified 1/5/9-second stills show coherent sepia trees, bridge, orbs, projectile/aim line, player, layered foliage and readable HUD. They have no blank/menu frame, conspicuous missing layer or newly obvious composition break. Compared qualitatively with the prior corrected same-route clip `port/build/forest-current-cap-delivery-capture-2026-10-23/forest-current-tree-10s.mp4`, the same bridge and ground corridor recur. Wall-clock capture starts/camera positions differ by small amounts, so the stills are **not pixel-registered** and no before/after delta is inferred. The older 11-second `current-tree-post-fog` clip diverges after its longer rightward input and cannot be treated as a frame-matched comparator.

Saved Steam `../store_media/screenshots/14.jpg` (SHA-256 `d7326063f5f9b163e9c6513ad2b68a27dd92f833e379a9049d9bc4b2b08dc3ab`) contains the recognizable raft/bridge, right branch, sword and orb landmark. Its lower ground around the player and right branch reads as a broader/brighter diffuse veil than this clip's 1-second bridge frame; the current 9-second frame also has poor player contrast against the dark central trunk/foreground. These are review concerns, **not proven Linux renderer defects**: Steam's HUD shows 13 orbs versus 2 here, and camera, elapsed fog time, player pose, weather/effect phase and encoding are unmatched. Source-authored Forest fog IDs 119/122 have alpha .5/.6 and a six-second fade (`forest_exploration_1.xml:1629-1665`; original `ScrollingSprite.cpp:22-55`; Linux `LevelScene.cpp:884-909`). Prior physical same-frame hide-ID evidence shows those fog layers are present. The near-black lower silhouette has separately been attributed to authored ID37; removing/brightening it from this unmatched image would not be source-backed.

**Next highest source-backed visual gate:** obtain one lossless original-Windows raft/right-branch frame and sidecar with camera/player position, elapsed level/effect time, fog IDs 119/122 alpha and orb state; register static landmarks against a Linux fixed-state frame and compare the lower-ground ROI, using the existing same-frame hide-119/122 control. Only if a difference remains after matched fade/state should the first differing source update/draw parameter be corrected. No speculative fog/ID37/art change is justified from the saved Steam JPEG alone.

## Ownership/release

No source or authored-art edits, commit, push, upload or message delivery were performed. All candidate/final captures and build outputs are ignored private artifacts; this uniquely named private report is the only non-ignored file written by this lane. Shared checkpoint files and unrelated dirty work, especially `main.cpp`, were preserved. The named Docker capture container exited 0; final `ps`/`docker ps` showed no Death Leak game, build, FFmpeg or capture container. **Shared Docker build and physical Xorg :0 / NVE7 GPU lane explicitly released.** This does not establish Windows parity, full-game acceptance or Jerome's visual acceptance.
