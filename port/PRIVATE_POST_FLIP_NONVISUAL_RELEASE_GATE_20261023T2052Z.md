# Private post-flip nonvisual release gate — 2026-10-23 20:52Z

## Disposition

**Actionable, unowned audio discrepancy: successful player jumps and rolls omit the original's occasional vocal cue.** This is a source-parity gap in active gameplay, not a device/callback failure. The 2026-10-23 20:31Z audio source gate first identified it, but the current tree still has no implementation or assigned owner for that correction. This audit reconfirms it after the ParticleSpray flip and selects it as the next bounded nonvisual correction. Physical keyboard/controller and speaker-listener acceptance remain open but require human/device actuation; visual parity is separately blocked by the unavailable matched Windows D3D10 runtime and Jerome review.

## Source evidence

- Windows `Game/src/GameObjects/Character.cpp:1239-1258`: `Character::Jump` rejects failed `CanJump` and water before `PlayJumpSFX()`. `Character::Roll` calls the same picker only after successful roll state transition (`:1460-1494`).
- Windows `Character::PlayJumpSFX` (`:1497-1525`) returns for non-players, then `rand()%20` maps exactly 0–4 to `character/jump_1.wav` … `jump_5.wav`; the other 15 outcomes are silent. All five authored WAVs are present in `Game/Media/Audio/character/`.
- Port `port/game/PlayerMotion.cpp:32-43,76-93` has successful roll and jump transitions. `port/linux/main.cpp:795-797,812-814,832-837` consumes roll focus and advances motion, including buffered landing roll, but emits no jump/roll sound event. The jump transition is internal to `PlayerMotion::Update`, so merely testing the held input in main would overfire.
- The current port cue enum `port/audio/SoundPlayer.h:12` has six cues and no jump vocal. The SDL sample list and cue mapping `port/linux/SDLSoundPlayer.cpp:27-33,53-59` contain none of the five jump WAVs. A targeted search of current `port/game`, `port/linux`, `port/audio`, and `port/tests` for `PlayJumpSFX`, `jump_[1-5].wav`, or `JumpVocal` yielded no match.
- `port/DISK_AUDIO_MIX_TEST_2026-10-22.md` proves callback PCM for the existing knife cue, not jump/roll coverage. `port/PRIVATE_POST_CLIP_REAL_INPUT_GATE_20261023T2032Z.md` documents physical D-key acceptance as unverified and requires a focused person/device; XTest is inconclusive. Neither gate closes this source discrepancy.

## Exact next bounded action

Assign a single audio/gameplay owner for `PlayerMotion`, `main`, `SoundPlayer`, and `SDLSoundPlayer`. Expose a one-shot successful **player** jump event from the actual motion transition, and dispatch once on each successful immediate or buffered roll. Add the five existing WAVs to the SDL sample bank. For each successful action, draw one value in [0,19], play its corresponding asset only for 0–4, and otherwise remain silent; honor the existing SFX mute. Do not emit on held jump frames, failed/cancelled jump or roll, swimming, or enemy actions. Keep the separate original wall-jump `jump.wav` path out of this narrow change unless its event semantics are independently established.

Verification after implementation: a CPU fixture exercises one successful jump, sustained held jump, failed/water jump, successful roll, failed roll, and buffered landing roll, asserting exactly one picker invocation per qualifying action and no invocation otherwise. Use a controlled picker/random seam to cover all five vocal branches and the silent branch deterministically. Then run one non-GPU SDL disk-audio callback comparison for a selected vocal branch, recording nonzero extra PCM against an equal-state idle/control run. This proves event dispatch and mixing, not physical speaker audibility.

## Status and scope

**Open; correction not performed.** This is a read-only audit plus this unique report only. No source/build/GPU/display/checkpoint/commit/message action was taken; all pre-existing dirty files remain untouched. The earlier 20:31Z report is evidence, not an implementation handoff.
