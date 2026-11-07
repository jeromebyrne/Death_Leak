# Forest→Waterfall loading fade — physical GPU gate (2026-10-23)

## Verdict

**Pass: the committed fade is visible and completes on strict accelerated physical nouveau/NVE7.** No fade bug was found, so no source edit, commit, or push was made. Controlled fresh-process captures show active Forest immediately before the trigger, progressive darkening, an entirely opaque frame on both sides of scene replacement, then progressive reveal and fully visible Waterfall. This is Linux integration evidence only; it is neither a matched-Windows pixel comparison nor Jerome's visual acceptance.

## Ownership and provenance

Read the latest `PORT_WORK_STATUS.md` checkpoint first. The earlier fade implementation owner had completed `port/PRIVATE_FOREST_WATERFALL_LOADING_FADE_IMPLEMENTATION_20261023T2209Z.md` and explicitly released `port/linux/main.cpp` and GPU/display. Preflight showed no competing Death Leak game, FFmpeg recorder, Docker build/capture container, or other GPU owner; physical Xorg `:0` remained the standing desktop server. Work began at HEAD `50014a1d6e3914d532bac7394a1e4de85a330720`; `port/linux/main.cpp` was clean. Existing unrelated dirty files, notably `port/linux/SDLRenderer.cpp`, were preserved and included in the current-tree build. No shared checkpoint, source, test, index, or config was edited.

The established `deathleak-linux-build:bookworm` toolchain rebuilt this exact live tree serially into ignored `port/build/private-loading-fade-physical-gpu-20261023t2212z/`:

```sh
docker run --rm --network host --mount "type=bind,src=$PWD,dst=/src,readonly" --mount "type=bind,src=$PWD/port/build/private-loading-fade-physical-gpu-20261023t2212z,dst=/out" -w /src deathleak-linux-build:bookworm make -e -C port TARGET=/out/deathleak-linux -j2
```

Build exited 0 (`build.log`; only preexisting TinyXML fallthrough warnings). Binary SHA-256: `f3415ce65b476e176cc4e2de5521fedd0caad9d39eda6cfddae9b4a343de8219`. All captures used that binary, `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --fixed-dt 0.016666667`, and a fresh process for each zero-based frame F, with `--frames F+1 --capture /out/frameF.bmp`. Exact invocation template:

```sh
docker run --rm --network host --hostname jerome-Linux-MacBookPro --device /dev/dri --mount "type=bind,src=$PWD,dst=/src,readonly" --mount "type=bind,src=$PWD/port/build/private-loading-fade-physical-gpu-20261023t2212z,dst=/out" --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy -w /src deathleak-linux-build:bookworm /out/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 424 --fixed-dt 0.016666667 --capture /out/frame423.bmp
```

Each captured run exited 0. Every corresponding log reports `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, GPU Sepia and 71 original Forest sprites/zero missing or unsupported assets. Logs consistently record the Forest right-exit trigger at frame 398, player `(3120.99,3120.45)`, and `Entered .../waterfall_top.xml` by frame 411. Waterfall authored ambience initializes; frame 423 terminates in Waterfall with health 25 and seven orbs. No software/dummy-video allowance was enabled.

## Lossless frame sequence

All BMPs are 1280×720, 3,686,522 bytes and stored with same-ordinal logs in the ignored evidence directory. RGB means are across the full viewport; black means every RGB channel of every pixel is zero.

| Frame | State | RGB mean | SHA-256 of BMP |
|---:|---|---|---|
| 397 | active Forest, immediately pre-trigger/pre-fade | 56.39, 51.06, 41.42 | `1d1848281d51d35bda23c5d4d0b4acfa36e8e7f05aedba04fe6708b32db9676d` |
| 405 | Forest, partial fade-in | 23.43, 21.22, 17.21 | `9c729debe67ff2fe3fa235e7c2ce0e1a50fefde0183e21360c15a85be16e092a` |
| 410 | **fully opaque black**, before destination entry | 0, 0, 0 | `e26ef58a562d45a8f8bf40aec887e913d93ac5b82bb5bfc3223ad44672d38998` |
| 411 | **fully opaque black**, destination entered | 0, 0, 0 | same as frame 410 |
| 412 | **fully opaque black**, minimum hold | 0, 0, 0 | same as frame 410 |
| 415 | Waterfall, partial fade-out | 26.97, 24.05, 18.76 | `f6b8786aa9c148946866273442ab2bc37c74b0f4a5ed4ccf6930e6c2bba51634` |
| 423 | first fully revealed Waterfall after fade by the committed fixed-dt state machine | 80.92, 72.15, 56.27 | `e28fa0b260a1b3cadedadecce62a8639a20f60b2549cfdd6acda5d1b7415476b` |
| 424 | continued Waterfall gameplay | 76.18, 68.05, 53.37 | `60a4bdc5da9e54aa10efe14a5b73e73c7089a4c2194f3ff96f10f5e62f59393a` |

Frames 410–412 are byte-identical black BMPs, including the frame in which Waterfall is installed: no old/new scene exposure at the opaque boundary. Frames 397 and 405 differ in 831,447 pixels and 405→410 in 827,069 pixels; frame 415 is nonblack after entry. The changing later Waterfall means reflect continuing animated gameplay and are not a pixel A/B against original Windows. Additional frames 426/429 and logs remain in the evidence directory. Lossless PNG review copies of 397/405/410/411/429 were also made; BMPs are the primary evidence.

## Scope and release

No real fade defect was observed; `port/linux/main.cpp` remains the committed HEAD version. Final Git check found it clean and HEAD unchanged. Final process/container check found no Death Leak game, FFmpeg capture, build, or capture container. **Exclusive physical GPU/display/build lane released.** Matched original-Windows parity, attachment readback of previously delivered gameplay clips, and Jerome's corrected-gameplay visual acceptance remain open.
