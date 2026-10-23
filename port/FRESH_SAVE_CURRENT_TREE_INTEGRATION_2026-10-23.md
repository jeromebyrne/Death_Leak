# Fresh-save current-tree integration — 2026-10-23

## Scope and result

Integrated and verified the released directed-blood impact-position repair (`Combat.cpp`), intro base-physics handoff (`main.cpp`), and intro arm/cadence handoff (`LevelScene.cpp` plus animation fixture) against the shared current tree. The prior dirty changes in `SDLRenderer.cpp`, `integration.sh`, `strafe_body_cpu_test.cpp`, status files and unrelated reports were left untouched. The fresh route was **opt-in** `--fresh-save-intro --play Game/XmlFiles/levels/forest_exploration_1.xml`, no load-save, scripted held Interact `I:220`, fixed 0.0166667 s; it is active rendered play, not a static preview.

## CPU and build gates

Four separately compiled `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat` fixtures, linking the relevant current-tree game translation units and TinyXML, all exited 0:

- `blood_directed_impact_position_cpu_test`: directed extended-ray contact preserves full advanced center and spray ray-end; ordinary zero-direction guard PASS.
- `player_blood_impact_cpu_test`: ordinary splat/material/no-material/inactive/rebound PASS.
- `fresh_save_intro_physics_cpu_test`: descent/contact at tick 12 from Y=3385 to Y=3206.9, grounded hold, threshold predicate and no-shot continuation PASS.
- `fresh_save_intro_animation_test`: authored 15/40-fps intro body command clock, empty arm in Looped/Pull and no-shot ordinary, later shot-positive arm PASS.

`git diff --check` on scoped tracked source/fixture files passed. Current-tree Docker `make -C port -j2` in `deathleak-linux-build:bookworm` exited 0. Built binary SHA-256: `db8e4caeb365948193b61050cd8cd20822b4ef8d33f225060576af952c1157fb`; build log and captures are in ignored `port/build/fresh-save-current-tree-integration-2026-10-23/`.

## Strict physical display/GPU evidence

Nine new-session captures at frames 1, 8, 12, 160, 166, 180, 195, 196 and 200 used host physical Xorg `DISPLAY=:0`, LightDM authority, `/dev/dri`, Docker X11, SDL OpenGL strict accelerated mode and dummy audio. **Every** log reports `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated` and `OpenGL device vendor=nouveau renderer=NVE7`. Each is a lossless 1280×720 BMP with its own log; `contact-sheet.png` and `player-crops.png` are review aids.

- Frame 1 reports player Y=3384.32, grounded=0, vy=-81.8182; frame 8 Y=3286.82, grounded=0, vy=-1309.09; frame 12 Y=3206.9, grounded=1, vy=0. The inspected crops show the body descending; the frame-12 player is at/below the crop edge, so ground contact is established by runtime state and the CPU fixture, not by a claimed visible foot-line measurement. Frames 160–200 retain Y=3206.9, grounded=1, vy=0.
- Frame 160 has no pull transition. Frame 166 log records the sole pull at frame 165; frame 180 crop shows red burst particles near the grounded player and a changed Pull pose. Frame 195 has no completion entry; frame 196 records completion at frame 195. Frame 196/200 logs show `fired=0 shots=0`, grounded=1. This is a full main-loop no-shot handoff; the separate draw-command fixture proves no stray intro/ordinary arm command, while these pixels alone do not isolate an occluded arm.
- The current frame 180 shows a red cluster and the prior blood-projectile report had a localized on/off difference at 180/200, but this new run has **no same-binary effect-off control**. It cannot separate the 70-particle sword burst from the one directed blood projectile, and does not establish an authored-forest line impact. Directed impact position remains CPU/source-verified only. No per-flight position/collision telemetry or matched Windows frame was captured.

The source contracts behind the changed code are: original `Player::Update` executes base `Character::Update` before the unpulled early return; intro arm sequences are empty and Pull body rate is authored 40 fps; original directed projectile advances its center before collision and does not rewind to ray contact. The port's physics/collision and private RNG remain approximations. Previous reports provide Linux causal A/B for individual effects, but their binaries predate this combined tree; the new captures should not be treated as a pixel-for-pixel prior comparison.

## Commit-time concurrent hunk caveat

Scoped commit `33e5017d1a48fe04cf144f3b06187746e565a66f` was pushed and remote SHA verified. Immediately after push, `git show` revealed that `port/linux/main.cpp` had gained an additional pre-threshold `cameraShake.Start(...)` hunk **between the initial diff review and staging**. That hunk was not in the reviewed 4-line main diff; it was inadvertently included in this commit and should be attributed to its separate owner, not this integration repair. The recorded binary hash and GPU captures cannot be asserted to correspond to the pushed commit, because the concurrent edit may have landed after the build. The four focused CPU fixtures do not cover camera shake. This report records the provenance error explicitly; it does not undo another owner's change or claim that the pushed tree passed a post-commit GPU gate. A fresh post-commit build and strict NVE7 spot captures were subsequently run: Docker `make -C port -j2` exited 0, binary SHA-256 `3c22fa9ed8f5f3c90c18391536aa49a0ba2432b3499df616f6f08876df56c7e7`, and frames 180/196 again reported `x11/opengl accelerated=1 software=0`, `nouveau/NVE7`, grounded Y=3206.9, pull at 165 and completion at 195, with `fired=0 shots=0` at 196. Logs/BMPs are `postcommit-*` in the same ignored evidence directory. This validates a narrow pushed-tree smoke path; it does not validate the shake hunk against its source contract or provide matched visual acceptance. Shake-owner provenance/sign-off remains outstanding.

## Boundaries and release

No matched Windows/Jerome acceptance, human controls, audio quality, exact original camera timing, or exact impact pixel parity is claimed. `PORT_WORK_STATUS.md` and `CHECKPOINT_STATUS.md` were not edited. All captured Docker processes exited. **Exclusive source ownership of `port/game/Combat.cpp`, `port/linux/main.cpp`, `port/game/LevelScene.cpp` and the relevant fixtures, shared `port/build`, and physical Xorg `:0` GPU lane are explicitly released.**
