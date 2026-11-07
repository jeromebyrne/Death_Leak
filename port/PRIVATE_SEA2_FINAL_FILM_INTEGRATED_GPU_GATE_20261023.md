# Private sea_2 final-film integration and physical GPU gate

UTC 2026-10-23. Scope: `port/linux/main.cpp`, isolated Docker build output and physical Xorg `:0`/NVE7 capture. Visual parity remains unaccepted.

## Source change

Read `PRIVATE_SEA2_FINAL_FILM_INTEGRATION_PATCH_20261023T1757Z.md`, its clock source audit and CPU preflight, and the pre-edit `git diff -- port/linux/main.cpp`. The pre-existing dirty diff in `main.cpp` implemented the separate ordinary `game_hud` film helper/clock. Preserving all of that diff, added retained `(finalFilmIndex, finalFilmLastSwitchMilliseconds)=(0,0)` state beside the ordinary state and replaced only the sea_2 `int(finalSceneTime*11)%4` selector with a monotonic integral-ms call to that helper immediately before the final film draw. The original sea_2 four assets, alpha 64, 1280x720 quad, postprocess, 100-pixel bars/lower flip, title order, and `finalSceneTime` strict 15/35-second gates were unchanged. The final selector does not share mutable state with the ordinary selector and is not reset on scene entry. This models the retained original `final_scene_hud` widget and one-step-per-active-update clock; it is not an assertion of Windows absolute phase or 32-bit clock-wrap parity.

## Focused CPU and build evidence

An ephemeral C++ fixture compiled the **actual extracted helper body from current `main.cpp`** under `g++ -std=c++17 -Wall -Wextra -Werror`, exercised 90/91, 181/182, 273, 364 ms `(index,last)` vectors, 3-to-0 wrap, a late 10000-ms one-step update, same-time no-repeat, ordinary-selector nonmutation and scene-clock independence. Source assertions checked the integrated sea_2 call-before-draw, postprocess-to-film-to-bars-to-title order, alpha/viewport/lower flip, absence of the old selector, and unchanged strict 15/35-second comparisons. PASS. This is a selector/branch static gate, not a recording renderer spy of live draw commands. `git diff --check -- port/linux/main.cpp` PASS.

Current dirty-tree Docker build PASS: `deathleak-linux-build:bookworm make -e -C port TARGET=/out/deathleak-linux -j2`, read-only `/src`, unique ignored output `port/build/sea2-final-film-20261023/`. Binary SHA-256 `fff1afc286879ba8841fa8822c3b9e57eadb8a0d75c55bfc50746303c99fb447`. Only pre-existing TinyXML warnings appeared. Build includes other dirty source, so it is not a clean scoped-commit proof.

## Strict physical GPU/lossless active-HUD gate

Two serial runs used that exact binary with `--play Game/XmlFiles/levels/sea_2.xml --fixed-dt 0.0166667`, `--frames 120` and `920`, lossless BMP `--capture` into the evidence directory, Docker `--network host --hostname jerome-Linux-MacBookPro --device /dev/dri`, host Xorg `DISPLAY=:0`, LightDM Xauthority, X11 SDL OpenGL and dummy audio. Both exited 0. Both logs report `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, GPU Sepia initialization, 19 original sea_2 sprites, zero missing assets and authored sea_2 ambience. At frame 900 / scene time 15.0166 the title log fires, consistent with the retained strict scene-time gate. Both BMPs are 1280x720 / 3,686,522 bytes. In inspected pixels, rows y=50 and y=670 are solid black at every x (top/lower bars); middle scene pixels are nonblack, and title-region red pixels occur in the frame-920 BMP. SHA-256: frame 120 `1dd0241d489d0a94880b52c307df9959fc76abbdbcfd49d57d51deefed0e1ff4`; frame 920 `ebe3a55b1e34a7294207f426331eca177479c6007dae785af278631a1b0378bb`.

This proves active sea_2 final-HUD rendering and title/bar coverage on strict physical GPU. It does **not** isolate the film index in the pixels: no controlled-index same-scene A/B, command spy, or matched original-Windows frame was obtained. Do not claim original pixel parity or Jerome acceptance. A concurrent separate forest-review worker also ran a build while this lane operated; the unique output/capture paths avoided artifact overwrite, but exclusive Docker build reservation was not enforceable from this child.

## Commit disposition and release

No commit or push. The inherited dirty `main.cpp` helper/ordinary-HUD implementation is a direct compile dependency of the new sea_2 hunk; staging the full file would scoop unrelated ownership, whereas staging only the sea_2 hunk would create a noncompiling commit against HEAD. The existing unrelated dirty source/status/reports were not staged or changed by this task. A scoped commit can follow after the ordinary helper's owner commits or explicitly hands off that hunk; do not misrepresent this working-tree verification as committed software.

No Death Leak build/game/GPU process from this task remains. **`port/linux/main.cpp`, isolated Docker build, physical Xorg `:0`/NVE7 GPU and capture lane are explicitly released.**
