# Fractional parallax renderer integration gate — 2026-10-22

## Result

**Integrated and pushed** the verified fractional-parallax renderer slice as `2c61efb` (`Bind fractional parallax UVs in SDL GL renderer`) on `feature/linux-renderer-foundation`; remote `origin` accepted `4661632..2c61efb`.

The commit contains exactly three software files: `port/linux/SDLRenderer.cpp`, the previously untracked but required `port/linux/ParallaxUV.h`, and the `port/Makefile` prerequisite for the already-tracked `ParallaxUVCommand.h`. No Combat, gameplay, controller-interface, capture, or status files were staged. The renderer working diff contained an unrelated `ControllerAxes()` override coupled to the concurrently dirty `port/render/Renderer.h`; that one hunk was excluded from the commit. A byte-for-byte check confirmed the staged renderer matched the physical-GPU-accepted working renderer with **only** that override removed. Both controller edits remain dirty for their owner.

## Verification and bounds

- `git diff --cached --check` passed before commit; staged names/stat were inspected and limited to the three files above.
- Four assertion-enabled focused CPU binaries compiled and passed locally from `fractional_parallax_uv_test.cpp`, `fractional_parallax_command_test.cpp`, `fractional_parallax_binding_cpu_acceptance_test.cpp`, and `fractional_parallax_nonfinite_cpu_gate_test.cpp` (the last is silent on success).
- The preceding physical-GPU acceptance documented a forced production build and accelerated Xorg/OpenGL same-frame float/fallback discrimination on the then-dirty working source: `port/FRACTIONAL_PARALLAX_GPU_ACCEPTANCE_2026-10-22.md`. This gate ran **no Docker, GPU, or display operation**. Local SDL/GL development pkg-config dependencies were absent, so no fresh build of the isolated commit was possible here. The only staged-vs-accepted source difference was the unrelated controller override, but this is not a fresh-checkout build claim.
- The acceptance does not establish exact Windows filter/mip parity, held-camera fractional-motion measurements, all GL-state getters, or a physically padded texture fallback; those remain bounded visual/runtime follow-ups, not blockers to committing this proven renderer slice.

The shared tree remains dirty with other workers' files. No unrelated staged changes were present at commit time. This report is a separate untracked handoff, not part of the software commit.
