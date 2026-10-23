# Independent post-vocal integration QA — 2026-10-23 20:57Z gate

**PASS after scoped repair; software lane releasable.** QA began from owner-released `26a2029` / handoff `c992427` (`c9924279fbf77e7fd7c78f1e8f0ed59af3c3d29f` at initial and final precommit inspection). Review found one out-of-contract path: the port's wall-jump branch also incremented the jump/roll vocal counter, although original `Character::WallJump` uses separate `jump.wav` and not `PlayJumpSFX`. Scoped repair guards that increment, and the CPU fixture now enters an actual wall-contact/impulse branch and asserts zero vocal actions. No unrelated dirty paths or shared status files were changed.

## Contract review

Original `Game/src/GameObjects/Character.cpp` `Jump` calls `PlayJumpSFX` only after `CanJump` and water rejection; `Roll` calls after successful transition; `WallJump` plays separate `jump.wav`. `PlayJumpSFX` returns for non-player and performs exactly one `rand()%20`, mapping 0–4 to five character WAVs and 5–19 to silence. Port `PlayerMotion` now queues one action on successful regular jump or successful immediate/buffered roll, and zero on held/failed jump, failed roll, underwater swim, or wall jump. `main.cpp` consumes once after transitions and draws once per action; combat gating prevents melee/health-invalid input actions. `DispatchJumpRollVocal` gates 0–4. SDL selects indices 10–14 and retains `sfxEnabled_` mute guard. No enemy object uses this player motion path.

## Frozen source and assets

SHA-256 of settled tree after repair, unchanged between final fixture run and this receipt:
- `port/audio/SoundPlayer.h` `4f00f59654bb36948d5998de688e079fe2a23a9553700bf59ffc73c7c2d54885`
- `port/game/PlayerMotion.cpp` `443be13303664e15637d795ec87a84c21dd920c8743422d508fb3f27323a6495`
- `port/game/PlayerMotion.h` `8475f90e685cda6a59700310281358648294ba7543f323b2dc23c24450431ef7`
- `port/linux/SDLSoundPlayer.cpp` `997cd8e344e2e9f54c7aa7fe32bbf5d0f3d6926ca91a3cf9540c174c23bd51d0`
- `port/linux/main.cpp` `e2e6ad47ead975e1d7b38044475072be47662565f40c1067db005beeeda9c68a`
- `port/tests/jump_roll_vocal_cpu_test.cpp` `ac21ebff5e225248ebedef6edf8f29aad97dfe7eb7420228ff5ed82e93c59cd7`
- `port/tests/jump_roll_vocal_sdl_disk_test.cpp` `8424f9c275b592ca2c6ca778414359b23b12faa64d8cec186aa0040f77e1965e`
- `Game/Media/Audio/character/jump_1.wav` `d0c2581431055e596e07d756857a7decd16e1093a727e5ce91062c529e6c67b4`
- `Game/Media/Audio/character/jump_2.wav` `9f085969f4d0bb55cf24cc6004bf9f480eecbbb9ac3a195c332cb44abc4ade5b`
- `Game/Media/Audio/character/jump_3.wav` `693b6e56d5cd1ba0c8f0d6746d05edfa176ad6bd185b24e40ed1f86b23174d4f`
- `Game/Media/Audio/character/jump_4.wav` `2a2b3f78b3cc9f15d64e20fd314f1023ebb7d02428bf211627d04ac9469bfb78`
- `Game/Media/Audio/character/jump_5.wav` `b1a9f3e39a1076445b0c9bca87f578ecde8499ab02528e7aad92013d13ee3ee3`

## Independent verification

Docker `deathleak-linux-build:bookworm`, `g++ -O0 -std=c++17 -UNDEBUG`; no GPU/Xorg. Focused CPU transition fixture rebuilt against final `PlayerMotion.cpp` and run from repository root: **exit 0**. It exercises successful/held/failed jump; successful, failed and buffered landing roll; authored water; all 20 picker values; wall contact plus actual wall-jump impulse with zero vocal action. SDL fixture rebuilt and run with `SDL_AUDIODRIVER=disk`, 48 kHz stereo signed-16 callback output: idle, selected draw 0, and SFX-muted selected draw 0 all **exit 0**. PCM controls:
- idle: 81920 bytes, 0 nonzero signed-16 samples, peak 0; exit 0
- vocal: 86016 bytes, 27580 nonzero signed-16 samples, peak 601; exit 0
- mute: 81920 bytes, 0 nonzero signed-16 samples, peak 0; exit 0
The selected vocal differs from silent control by 27,580 nonzero samples; mute remains fully silent. Callback duration can vary; sample count and zero/nonzero distinction are the acceptance signal. `git diff --check` on scoped repair: exit 0. The fixture verifies software mixing, not physical-speaker audibility.

**Release lane:** scoped software QA pass; commit/push verification to be recorded in owner handoff. Human audio/input acceptance remains a separate device gate. No visual parity or final game release claim.
