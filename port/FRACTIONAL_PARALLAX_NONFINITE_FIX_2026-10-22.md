# Fractional parallax nonfinite CPU fix — 2026-10-22

## Change

`ForEachUVWrapSegment` keeps its `pixels <= 0` early return. For a positive span, NaN or either infinity now selects phase zero before `floor` and before `static_cast<int>`. Finite shift computation, integer cut, local flip ordering, and segment offsets are unchanged. An invalid axis consequently yields one full-source segment at offset zero, while a finite opposite axis retains its normal segmentation.

The existing standalone `fractional_parallax_nonfinite_cpu_gate_test.cpp` was retained and its stale expected-failure comment updated. It exercises command preparation through a nested legacy-copy sink, invalid X/Y/both axes, all flips, and zero/negative spans.

## CPU verification

Host compiler: `c++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`.

From repository root, with `build_dir=$(mktemp -d /tmp/deathleak-uv-nonfinite.XXXXXX)`, each of these was compiled and executed:

```sh
for test_name in fractional_parallax_nonfinite_cpu_gate_test fractional_parallax_command_test fractional_parallax_uv_test; do
  c++ -std=c++17 -O1 -g -UNDEBUG -fsanitize=undefined,float-cast-overflow -fno-sanitize-recover=all "port/tests/${test_name}.cpp" -o "${build_dir}/${test_name}"
  "${build_dir}/${test_name}"
  printf 'PASS %s\n' "$test_name"
done
```

All three passed with assertions enabled and no sanitizer diagnostics. `git diff --check -- port/linux/UVWrap.h port/tests/fractional_parallax_nonfinite_cpu_gate_test.cpp` passed. The finite UV geometry and command-seam tests passed. The SDL-dependent waterfall wrap test was not run because this task excludes renderer/SDL binding and the shared build lane. No Docker, GPU, Xorg, renderer binding, or integration test was used. This establishes only the CPU helper/command-sink contract, not renderer wiring or visual parity.
