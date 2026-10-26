# Intro X-prompt fixed-update presentation snapshot — 2026-10-26

Base: clean detached remote `5a1a1b408877634ce31b0afab10b45e89337d27e`; sole source/build/physical-NVE7 owner. Primary unrelated dirty and untracked files were not build inputs.

## Bounded change and rejected candidates

`IntroPromptPresentation::Prepare()` snapshots the prompt admission boolean after scene/player visual updates and before the live catch-up/present branch. It samples scripted/live Interact, looped intro phase, cumulative prompt clock, capture suppression, and art residency exactly once per fixed update. Ordinary draw reads only `Visible()` for admission; existing sprite geometry and player-adjacent callback/painter order are unchanged. A same-clock repeated presentation no longer polls input or mutable intro phase/clock for this family. This does not make the full draw read-only: `scene.Draw`, combat painter traversal, rain/weather sprite generation, HUD/vitals, renderer internals, and capture/readback still need distinct audits. Moving them as a group was rejected as unsafe without per-family state and ordering contracts. Pre-HUD postprocess readback and post-overlay capture order are unchanged.

Live catch-up now snapshots prompt input on each fixed update; if input changes between update and draw, visibility reflects the last update rather than a later draw-time poll. This is an intentional timing seam, not an assertion of arbitrary wall-clock parity.

## Verification

- Existing prompt placement/source contract plus enhanced focused C++17 test passed: authored `>8s` threshold, held/suppressed/no-art/nonlooped rejection and 100,000 repeated const packet reads. Final restored-native-source Docker production build passed (`b84e283606df62e00e6bd178dfd8ee5438c5cd11000be4b1d01ce9b63d81c899`); `git diff --check` passed.
- Strict serial physical `:0` X11/OpenGL Mesa nouveau NVE7, accelerated=1 software=0 vsync=1, 1280×720 fixed-60 A/B/A fresh-save Forest-1 frame 100 full post-HUD BMPs byte-identical, SHA-256 `47acfd41ac7f17f9ec4e23e9745412f0ba28ea6d2122448d704bd7b44a4037d8`, zero changed pixels; gameplay endpoint summaries match. An identical test-only fixture in baseline/candidate set prompt clock to 9s at frame 100 and forced admission's looped-state input true, plus identical frame-indexed film clocks. Prompt suppression changed 2,971 RGB pixels and SHA-256 to `1ba9ce9857e7d1f65088034dca445b534e597ffa66333d7512bbf46391a6224b`, establishing that the visible prompt path was actually exercised. All test-only source changes were removed before final build/commit. An earlier ordinary `--play` route was rejected for visible-prompt parity because it never loaded fresh-save prompt art; its A/B/A was only a negative gate.
- Evidence: ignored primary `port/build/gameplay-snapshot-20261026/` BMPs/logs/build logs. Native arbitrary wall-clock phase, high-Hz output, 8K, sustained FPS, full update-free presentation and release acceptance are not established.
