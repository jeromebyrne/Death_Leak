# Health meter length implementation — 2026-10-23

**Completed bounded HUD slice.** On the `feature/linux-renderer-foundation` branch, the Linux HUD now applies the original health meter's `3 × maxHealth` length in 1920-wide UI units, projected by 2/3 into the 1280-wide viewport. The health overlay span, flipped right cap, and bar fill derive from that live length. The existing health-left anchor, health textures/tints, focus meter, and orb commands remain unchanged.

Source contract: `Game/src/UI/UIGameHudScreen.cpp` schedules a changed cap for the following update; `DoHealthMeterUpgrade` immediately shrinks or grows by 0.20 original UI units per update; `Game/src/UI/UIMeter.cpp` uses `(length - 0.8 × edge width) × progress` for fill. The 4-pixel edge texture was confirmed from `Game/Media/UI/meter_edge_2.png`. Health cap flipping now uses the actual Sprite `flipX` field (the former positional initializer put `true` in rotation and zero in opacity).

## Evidence and verification

- Fresh 25-cap: first draw retains XML width 300; second draw settles at 75 source units / 50 logical pixels. Full-health bar fill is 47.8667 logical pixels; at 50% it is 23.9333, while outer length/cap stay fixed.
- Fresh imported 65-cap: first draw 300; second draw settles at 195 source units / 130 logical pixels. Upgrading an already settled 25-cap schedules on the first changed-cap draw and reaches 75.2 source units on the next draw.
- CPU renderer-spy fixture: `c++ -std=c++17 -Wall -Wextra -Iport port/tests/health_meter_length_cpu_spy_test.cpp port/game/Hud.cpp -o /tmp/health_meter_length_cpu_spy_test && /tmp/health_meter_length_cpu_spy_test` — **PASS**, including unchanged focus/orb sprite and orb glyph commands.
- Docker build: `docker run --rm -v "$PWD":/src:ro -w /src deathleak-linux-build:bookworm make -C port TARGET=/tmp/deathleak-hud-build -j2` — **PASS**, exit 0. Existing TinyXML warnings only; no HUD warning. Source bind was read-only and output stayed inside the disposable container.
- `git diff --check -- port/game/Hud.cpp port/game/Hud.h` — **PASS**.
- No GPU/display or Windows matched-frame claim was made.

## Hashes and scope

Base HEAD before this slice: `2e1624e9eb2addc4f7b804684a1cfb316c74a3f0`.

| Owned file | SHA-256 |
| --- | --- |
| `port/game/Hud.cpp` | `bc6ef2a38e919f1b771fbb832614de41504dcf3f3e2f9cc59b8a0c18eb40c05f` |
| `port/game/Hud.h` | `a07390e5a0935d5e0a9e46ecec96ac8f9c2fabfbf8c080f17ccd499565d18051` |
| `port/tests/health_meter_length_cpu_spy_test.cpp` | `8bf4b4d9bce79461970831b965253aa635fd11a142c18bd863e65c64544d721c` |

Only these three files and this handoff were staged for the slice; unrelated dirty LevelScene/main/integration/status files were left untouched. **Implementation and verification complete.**
