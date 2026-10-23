# Fresh-save forest-1 intro caller integration — 2026-10-23

## Scoped result

`port/linux/main.cpp` now offers `--fresh-save-intro` only with a new `--play Game/XmlFiles/levels/forest_exploration_1.xml` session (never `--load-save`). This explicit eligibility avoids interpreting an ordinary `--play` or an absent DL5 field as a new game. The session-only `hasPulledSword` state is not serialized, matching the temporary original `has_ungutted` key. The existing menu New Game route still enters `grass_exploration_3.xml` and resets the intro state to complete; no menu routing was changed.

The caller selects the committed LevelScene phase-1 loop, reads raw held Interact (`I` per scripted frame), resets continuous hold on release, transitions only after `held > 2.75 s`, resets to authored load zoom at that threshold, and waits for `IntroBodyFinished()` before restoring ordinary control. While pending it bypasses movement, jump, roll, melee/fire/combat update, interactable activation, and right-stick target updates, but continues camera follow and scene/animation update. The simulation-only route advances the same intro body terminal signal using the preceding modified delta. A loaded forest-1 save, ordinary `--play`, waterfall, and sea-2 remain on their previous gameplay paths.

The caller keeps immutable `introLoadZoom` and separate `introEffectiveZoom`: hold lowers by `0.05/s` to `0.75 × load`; release returns by `0.10/s` to load; threshold snaps to load. The effective value is supplied to `FollowCameraWithTarget`. This is a camera-follow scalar integration, **not visible world zoom**.

## Verification and limits

`c++ -std=c++17 -fsyntax-only port/linux/main.cpp -Iport -IGame/src` and `git diff --check -- port/linux/main.cpp` passed. A private, non-shared executable build was attempted in `/tmp`; it could not compile because this sandbox lacks SDL2/libpng/GL pkg-config metadata and SDL headers. Therefore no live runtime, rendered-frame, or input-path claim is made. No physical GPU `:0`, capture, or shared build target was used.

Renderer/world zoom remains for a separate owner: scene geometry currently uses fixed 640/360 center and fixed scale; a world-pass transform must scale orthographic extents from 1920×1080 to 1440×810 at 0.75 while preserving HUD/postprocess separation and painter/parallax order. That owner must also prove a CPU camera spy and fresh-save rendered visual sequence. Intro audio, shake, blood, rumble, achievement, and idle prompt remain outside this narrow seam. The existing menu grass entry would require separately owned routing before a menu-origin forest intro is possible.

## Ownership release

Only `port/linux/main.cpp` and this report were edited and staged for the scoped commit. Their file ownership is released on commit. `port/linux/SDLRenderer.cpp`, other dirty files, `port/build`, display/capture, and release artifacts remain with their existing owners or unclaimed; this commit does **not** release or approve them. No shared integration/test/checkpoint file was touched.
