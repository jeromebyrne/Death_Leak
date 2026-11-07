# Gameplay HUD film first-active-frame phase — private CPU implementation (2026-10-23)

## Change

`port/linux/main.cpp` now initializes the ordinary `game_hud` film selector's last-switch timestamp to zero, like `UIAnimatedSprite`, and updates it against integer milliseconds since `steady_clock` epoch. This is the Linux monotonic-uptime analogue of Windows `timeGetTime()` used by the original UI update. The strict `1000/11 == 90` millisecond comparison, single-step advance, four-frame wrap, and last-switch assignment on an advance are preserved. No fixed first-frame-1 shortcut was used: a first active update at 90 ms still selects frame 0, while one at 91 ms selects frame 1.

The four existing film assets, viewport destination, byte alpha 51, and postprocess → film → bars → meter draw order are unchanged. The `sea_2` and menu selectors are untouched. This is source-command/phase alignment only; no claim of Windows pixel parity or matched-frame visual acceptance.

## CPU verification

Ran `python3 port/tests/gameplay_film_first_active_frame_private_cpu_test.py`: **PASS**. The private fixture compiles the production selector extracted from `main.cpp` with `g++ -std=c++17 -Wall -Wextra -Werror`, then checks first-update 90/91 ms, exactly one advance for a much later first timestamp, strict next 90/91-ms threshold, four-frame wrap, selected second asset path, and static gameplay draw ordering/assets/viewport/alpha. Ran `git diff --check -- port/linux/main.cpp`: **PASS**.

No shared build, GPU, physical Xorg, or capture run. No commit or push. Unrelated dirty files were present before work and were not edited. This owner's writes were limited to `port/linux/main.cpp`, the uniquely named private CPU fixture, and this report.

**Ownership of these three files and this bounded CPU lane is explicitly released.**
