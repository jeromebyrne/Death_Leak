# ParticleSpray horizontal UV flip closure — 2026-10-23 20:45 UTC

## Decision

The existing uncommitted `leaf.flipX = true` is source-correct and was not already committed. Original `Game/src/Particles/ParticleSpray.cpp:8-11,185-214` hard-codes U=1 at world-left and U=0 at world-right. Those vertices put V=0 at world-top (+Y). Linux `ParticleSpray::Draw` projects world-top to screen-top, and `render::Sprite::flipX` is consumed by both SDL and GL renderer paths as horizontal UV reflection. Thus flipX=true, flipY=false reproduces the source corner map. The old per-particle random flip branches are commented out in Windows source and are not the live contract.

The prior `port/PRIVATE_POST_ID37_PARTICLE_SPRAY_GATE_20261023T2021Z.md` described the dirty in-tree flip as already present; it was not a separate landed fix. At the checked HEAD (`0a215bc`), `ParticleSpray.h` omitted the assignment. This closure commits the pre-existing owner change, without duplicating a committed behavior or altering authored XML/art.

## Verification and scope

Added `port/tests/particle_spray_flip_cpu_test.cpp`: a renderer spy checks the emitted quad bounds and its four source-equivalent UV corners (left U=1, right U=0, top V=0, bottom V=1). An isolated `deathleak-linux-build:bookworm` Docker C++17 compile/run passed with `-Wall -Wextra -Werror`. An actual-process inspection found no competing make/docker/g++ build before the test. No physical GPU, gameplay capture, or full integration suite was used. This is source/CPU geometry verification only, **not** Windows visual parity or human visual acceptance.

Only `port/game/ParticleSpray.h`, the new test, and this report were staged. Shared checkpoint/coordinator files and all unrelated dirty/untracked work were preserved.
