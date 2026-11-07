# Fresh-save sword-pull blood burst: next visual gate (17:39, 2026-10-23)

## One source-proven discrepancy

**The two directed burst textures are horizontally mirrored relative to Windows.** This is the highest-impact remaining *established* burst-only discrepancy from this bounded read-only audit: it affects the silhouette of all 70 particles, especially the asymmetric 512×512 `bloodparticle3.png`, throughout the focal burst. This is a source/implementation finding, **not** a measured Windows-vs-Linux screenshot difference or visual acceptance.

- The original event creates 20 `bloodparticle2.png` and 50 `bloodparticle3.png` particles only at the held-Interact `>2.75f` sword-pull transition (`Game/src/GameObjects/Player.cpp:1025-1083`). Current Linux fires the burst at that same threshold (`port/linux/main.cpp:681-692`) and creates the two matching recipes (`port/game/LevelScene.cpp:1072-1116`). The separately spawned `FireBloodSpatter` flight is not part of this finding.
- Original `ParticleSpray.cpp:8-11` assigns the particle quad's world **left** vertices U=1 and world **right** vertices U=0. Those assignments are actually used at `:191-219`; the alternative per-particle flip code at `:182-189` is commented out. World Y increases upward, so the V assignment becomes the usual top-to-bottom image orientation after camera projection; **U remains reversed**.
- Linux `port/game/ParticleSpray.h:83-100` submits each particle as an SDL sprite without setting `leaf.flipX`; its default is false (`port/render/Renderer.h`, `Sprite` definition). `port/linux/SDLRenderer.cpp:235-236` uses the sprite flip flags to select SDL horizontal/vertical reflection. Thus the Linux particle uses ordinary left-to-right U, unlike the source. `LevelScene.cpp:1438-1439` uses precisely this spray draw path for the intro burst.
- The art is not horizontally symmetric: alpha-channel mean absolute difference versus its horizontal mirror is 14.66/255 for `bloodparticle2.png` (128²) and 30.78/255 for `bloodparticle3.png` (512²), measured with Pillow `ImageChops.difference`/`ImageStat` on the repo PNGs. The latter is 50 of the 70 particles. The source call constructs a generic spray (`ParticleEmitterManager.cpp:515-525`, `new ParticleSpray(false, ...)`), so a BloodParticle.fx-specific tint change is **not** the remedy.

## Minimal next implementation and discrimination gate

After the exclusive X-prompt painter/GPU owner releases `main.cpp`/build/Xorg/GPU and the renderer seam, set `leaf.flipX = true` **in the particle spray draw path** before `renderer.DrawSprite(leaf)`; keep `flipY` false. This is a one-line scoped change to `port/game/ParticleSpray.h` and restores the original U map for the intro burst without changing emission, physics, art, alpha or depth. Because the same draw path serves XML and movement sprays, a burst-only switch is an option if broader particle parity is not yet established.

Discriminate on the released physical GPU with one fresh-save held-pull capture at a fixed frame early in the burst, A/B only the flipX flag with identical scripted input/seed/camera. Inspect a single isolated `bloodparticle3` particle or use a temporary diagnostic one-particle mask: its left edge must sample source U=1 and its right edge U=0, while V remains unchanged. A CPU renderer-spy can first assert the submitted intro particle's flipX flag, but cannot establish final pixels. If full Windows visual parity is claimed, compare against a same-phase Windows frame; no such rendered proof was gathered here.

## Scope and lane release

Read-only inspection of original source/art and current Linux source only. No source, build, Docker, GPU, Xorg, shared status/checkpoint, or existing report files were changed. **This blood-burst audit lane is released.**
