# Forest-1 ID22 x320 overdraw: same-frame physical-GPU discriminator — 2026-10-23

## Verdict

At the frozen current-tree `R:180` Forest-1 frame, authored XML Sprite ID22 (`tree_strip.png`) **does visibly cover/change the x320 middle band**. Hiding ID22 changes 158 of 180 pixels in the x320/y380–559 column, including the proposed y440–530 strip-onset band; the same hide changes **zero** pixels at x640 and x960 in y380–559. This validates the source follow-up's column-specific ID22 contribution, not a Linux-only layering defect. Crucially, ID22 is **not the sole occluder of ID19** at x320: even after hiding ID22, additionally hiding ID19 changes zero pixels there in y380–559. In contrast, hiding ID17 after ID22 exposes ID17's x320 contribution starting at y519. Thus the narrower causal conclusion is ID22 overdraw and partial ID17 exposure, while the x320 ID19 absence still has another later/compositing explanation. No XML/art change follows from this capture.

## Frozen capture and provenance

- Repository HEAD `c1756b2f5b8b2c95b0566ffa7126b2b70ab4891b`; working-tree state saved in ignored `port/build/forest-id22-overdraw-gpu-discriminator-2026-10-23/tree-status.txt`. The integrated ID37-alpha fixed binary from `port/build/id37-alpha-integrated-gpu-2026-10-23/deathleak-linux.fixed` was copied, then used unchanged for all five variants. Its before/after SHA-256 equals `b60b1e42a91c232d24470242d2eacd8edecc6e8d61c2f6b49cf949ef7ed2bf4b`. The shared older `port/build/deathleak-linux` was not used or overwritten. No source, XML, art, test, or shared binary was edited.
- One exclusive Docker container on physical Xorg `:0` with `/dev/dri` and LightDM Xauthority ran five sequential captures using `deathleak-video-capture:bookworm`. Every run: `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667 --capture <mode>.bmp`, `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy`. Render-only `DEATHLEAK_RENDER_HIDE_XML_IDS` was unset for baseline and set respectively to `22`, `22,17`, `22,19`, `22,17,19`; the diagnostic parser accepts comma-separated IDs (`port/game/LevelScene.cpp:62–85`). All five lossless 1280×720 BMPs and logs are in the ignored evidence directory above.
- Each log explicitly reports `video_driver=x11 name=opengl`, `accelerated=1 software=0 mode=strict-accelerated`, and OpenGL `nouveau/NVE7`, with GPU Sepia initialized. The printed simulation state agrees across all five: player `(1368.99,3107.3)`, five orbs, health 25, grounded, `vx=480`, and all other gameplay fields. Endpoint lines are not byte-identical because `elapsed=` is wall time (3.63–4.04 seconds). Baseline BMP SHA-256 `8d85a22bd50aab1ef8adf44702f01567fb432434ab6263e6c249045e088d7bd5` exactly equals the fixed-full image from the ID37 integrated GPU gate, providing same-frame cross-gate registration. The prior read-only GDB gate measured this route's final camera `(1248.98621,3400)` with a byte-identical baseline for that prior binary; this capture did not remeasure the current binary's camera, so identical gameplay state and baseline-image hash, rather than a new camera trace, establish registration to the current ID37 gate.

## Measured lossless pixel deltas

Any nonzero RGB-channel difference after BMP decoding counts as a changed pixel; bboxes are half-open `(left,top,right,bottom)`. Coordinates are top-down image pixels.

| Comparison | Changed RGB pixels, full frame | Bbox | x320/y380–559 | x640/y380–559 | x960/y380–559 |
| --- | ---: | --- | ---: | ---: | ---: |
| baseline → hide22 | 308,225 | `(0,0,1273,623)` | 158 | 0 | 0 |
| hide22 → hide22+17 | 71,270 | `(0,505,1280,632)` | 40; first y519 | 48; first y506 | 33; first y527 |
| hide22 → hide22+19 | 61,292 | `(19,440,1280,567)` | **0** | 51; first y460 | 73; first y455 |
| hide22 → hide22+17+19 | 128,435 | `(0,440,1280,632)` | 41; first y519 | 96; first y460 | 105; first y455 |

At x320, baseline→hide22 differs in runs y380–416, 436–502, y504, 506–529, and 531–559. Concrete RGB samples (baseline → hide22 → hide22+17 → hide22+19): y450 `(41,35,25) → (75,65,45) → (75,65,45) → (75,65,45)`; y520 `(98,85,60) → (83,71,50) → (92,79,56) → (83,71,50)`. At y451, hide22 similarly changes `(42,36,25)` to `(76,65,46)`, but hide22+19 remains `(76,65,46)`: an apparent onset in a baseline image does not by itself identify ID19 as the current composited writer. At y520, the hide22+17 delta independently establishes an exposed ID17 contribution.

## Interpretation and boundary

The source follow-up identified XML ID22 after ID17 at equal near-background depth, alpha-one and a horizontally flipped repeat-X tree strip; a point PNG probe predicted opaque ID22 at x320/y450–530 but transparent at x640/x960. The observed hide22 delta has exactly that column-specific middle-band pattern. The earlier current-frame GPU gate found ID17/19 composited hide deltas at x640/x960 but none at x320 when each was hidden alone. The new second-order hide comparison shows why the proposed simple explanation was incomplete: removing ID22 reveals ID17 at x320/y519 onward, but not ID19 in the tested band. Other later geometry, alpha, or post-composite interactions may still mask ID19; this experiment does not identify that remaining cause.

This is **Linux current-frame attribution only**. Original Windows frame/camera/effect-state RGB is unavailable; stable authored ordering in both implementations means ID22 overdraw is not, by itself, a port regression. No matched-Windows parity, visual acceptance, or release claim is made. The physical-GPU container exited and the lane is released.
