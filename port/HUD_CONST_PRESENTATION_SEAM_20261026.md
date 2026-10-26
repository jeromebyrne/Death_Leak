# Bounded HUD const-presentation seam — 2026-10-26

Base: clean detached fetched remote `2bbfc7fa63051435f5ffd2324ed0dac4546b6167`; primary seven unrelated dirty tracked paths and all untracked files preserved.

## Implementation

`Hud::Prepare(health,maxHealth,focus,preRegistrationProcessTotal)` advances the two meter transitions, backing decay, upgrade width, and target registration once at the original HUD call site. `Hud::Draw(renderer,orbBalance) const` submits only the prepared orb/text/sprite commands. Original update-before-registration ordering is retained. The live caller splits the former combined call immediately before its HUD submission, so the 60-Hz fixed-dt route preserves call cadence/order. No extra display-frame loop or full-scene read-only painter has been added. This bounded HUD seam alone does not solve camera, shader, film, lazy resource, or packet side effects.

## Verification

- Focused `hud_const_presentation_cpu_test.cpp`, C++17 `-Wall -Wextra -Werror -UNDEBUG`: passes. Two draws at the same prepared clock through a const HUD emit identical sprite/rect commands; the next Prepare, not Draw, decays the health backing by exactly `.005` of the authored bar width.
- `hud_fixed60_parity_trace.cpp` compiled against both untouched remote-base Hud.cpp/Hud.h (`BASELINE_HUD`) and candidate. 240 fixed-60-Hz frames with health/focus/max-health/orb transitions produce the identical per-frame sprite/rect command digest `bd7fd75484350579` in both binaries. Digest includes geometry, tint, texture identity, opacity, rotation, flip/repeat and frame boundaries; it is a CPU command parity proof, not a pixel proof.
- Isolated production Docker `deathleak-linux-build:bookworm make -C port -j2`: passes (existing main/TinyXML warnings). `git diff --check`: passes.
- Physical Xorg `:0` is not authorized to this sandbox: `DISPLAY=:0 XAUTHORITY=/home/openclaw/.Xauthority xdpyinfo` exits 1 with `Invalid MIT-MAGIC-COOKIE-1 key`. Therefore synchronized physical NVE7 60-Hz full-frame pixels were **not** captured. No GPU FPS, >60-Hz display output, 8K, sustained route, thermal/VRAM, or release acceptance claim.

## Next gate

Restore an authorized physical-NVE7 X11 route and compare synchronized full post-HUD 60-Hz frames of this commit against base on gameplay with health/focus/max-health transitions. Then separately split camera/shader/film/resource and packet effects before wiring any update-free presentation slot; test >60 Hz only on genuinely advertised physical modes. The current live loop remains one draw per accepted update, not a high-Hz presenter.
