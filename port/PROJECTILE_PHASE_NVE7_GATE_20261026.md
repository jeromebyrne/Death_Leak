# Active projectile phase and physical NVE7 gate — 2026-10-26

Base: exact remote `c70bb80b82d53a22aad2c5fe9a726656f4074691`; clean detached `/tmp/deathleak-projectile-phase-20261026`. The primary checkout's unrelated dirty files were not built or staged.

## Access recovery

Host Xorg `:0` is launched with `-auth /var/run/lightdm/root/:0`. The old `/home/openclaw/.Xauthority` is rejected (`Invalid MIT-MAGIC-COOKIE-1 key`); direct unprivileged access to the LightDM directory is denied. A root-owned, ephemeral Docker runner bind-mounted the live LightDM directory read-only, `/tmp/.X11-unix` read-only and `/dev/dri`. No cookie bytes were printed or copied to the repo. `xdpyinfo` authenticated (1440×810 display); the game itself selected strict SDL x11/OpenGL `accelerated=1 software=0 vsync=1`, `vendor=nouveau renderer=NVE7`, 1280×720 output.

## Executable slice

`MixedCommandInterpolationGate.h` now admits bounded projectile position, isotropic authored fade-in scale/opacity and shortest-path heading between the *same stable birth ID*, while holding texture, source, RGB, flip, blend, UV and shader state. Bounds reject discontinuous travel, rotation, dimensions and alpha jumps. Scene epoch/camera cut guards remain. New full emitted-packet topology check rejects projectile phase sampling across command births/deaths, painter reorder or batch cardinality change. Other commands retain the conservative existing policy. `Makefile` now tracks the gate header as a production dependency. No default pixels, simulation or visible interpolation switch changed.

Focused C++ mixed-command fixture passed projectile phase/alpha=1, flip, scene cut, and effect-birth rejection; isolated production Docker build passed (SHA-256 `8a3341aa2dd1a675d10b668230c043c70e3eb4e405f306066fa433cdd3dcf30a`).

## Physical actual-packet proof and decision

Final binary, fixed-1/60 Forest `B:125`, active frame 120: 93 emitted commands, three stable projectile sprites **all sampled** at fractional alpha; 45 commands sampled/48 held. Frame 121: 3/3 sampled; frames 122–124: 0/3 held by topology/discontinuity policy. Offscreen replay on physical NVE7 through the complete SDL scene/postprocess path: current and alpha=1 BMP SHA-256 both `e309144c232d44de3ea89cc7e30fe7959203b908a94bac675857ff95d866950c`, **0/921,600 RGB pixels changed**; alpha=.5 SHA-256 `ecd32f56f06122f9486600f2817cabef199be24413a7ca551613f2888dd46cd0`, 533,977 pixels changed (whole-frame, not projectile-isolated). Forest→Waterfall-top cut frame 415: scene epoch 2, 40 commands, 0 sampled; all three captures byte-identical SHA-256 `5369f94fb1facd4c89b2e0a9d96009effccc72e0735db13d0d63cf47e3299e3d`. Evidence is under ignored `port/build/projectile-phase-evidence/` in the isolated checkout.

**Visible B and serial active A/B/A are rejected.** Projectile motion is now fractionally sampleable within stable paired emitted packets, and NVE7 access is restored, but frame 120 still holds 48 commands including changed actor/effect/film states while scenery and projectiles move. The physical fractional image proves changed output, not coherent full active composition. Next distinct gate: own and sample or globally hold the remaining active actor/effect phases together, with explicit film/animation discontinuity policy, then repeat moving/cut exact-frame physical NVE7 checks before a visible switch and serial A/B/A. No 8K, high-Hz, 60-FPS, sustained/thermal/VRAM or release acceptance.
