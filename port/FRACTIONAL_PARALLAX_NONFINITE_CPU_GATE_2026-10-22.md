# Fractional parallax nonfinite CPU gate — 2026-10-22

## Scope and result

Added `port/tests/fractional_parallax_nonfinite_cpu_gate_test.cpp`, a standalone command-to-nested-legacy-copy test. It covers NaN, +infinity and -infinity independently on X, Y and both axes; all four flip combinations; a finite opposite axis retaining its two correctly ordered segments; full-source zero-offset normalization of invalid axes; zero/negative X and Y spans and their no-copy behavior. The command's original destination, flips and shift values are checked, and each nested copy is checked in Y-outer/X-inner order.

**Gate status: expected failure, not executed.** The existing `port/linux/UVWrap.h:15-18` computes a nonfinite phase and casts `phase * pixels` to `int` before clamp. For positive spans, invoking the new gate on this source would reach undefined float-to-int conversion; under `-fsanitize=float-cast-overflow`, it should report that conversion. This is the exact production defect, not a test defect. No production renderer/helper was changed, per task boundary. The CPU gate cannot pass until the shared helper neutralizes nonfinite shift to phase zero *before* floor/conversion.

## Verification boundary and handoff

No compilation/build, Docker, GPU, Xorg, capture or integration run was performed because the shared build/physical lane was reserved. Source was inspected against `port/FRACTIONAL_PARALLAX_RUNTIME_SAFETY_AUDIT_2026-10-22.md` and the existing `fractional_parallax_command_test.cpp`. The new test is intentionally separate from the existing command test, which stops before the unsafe sink.

After helper ownership clears: add a pre-conversion `std::isfinite` guard in `ForEachUVWrapSegment` (invalid shift → phase zero; retain the `pixels <= 0` early return), then compile this file as a standalone C++17 assertion-enabled test with float-cast-overflow sanitizer and run it. Also rerun the existing finite wrap/command tests to ensure cut and flip behavior are unchanged. This gate does not prove SDL/GL binding, renderer wiring, or physical parity.
