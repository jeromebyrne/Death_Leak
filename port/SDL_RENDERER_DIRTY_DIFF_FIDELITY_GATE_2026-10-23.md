# Dirty SDL renderer diff: Forest-1 fidelity gate (2026-10-23)

## Verdict

**Not complete or visually verified.** Against current HEAD `5c2d335`, the only renderer diff is a 76-line addition/one-line replacement: an opt-in `DEATHLEAK_RAFT_MIP_EXPERIMENT=on` GL replacement for ordinary scene draws of `Media/objects/raft.png`. With the variable unset or any value other than `on`, `RaftMipExperimentEnabled` returns false and the prior SDL draw branch executes; the diff does **not** establish a default-mode Forest-1 regression. With it on, Forest-1 raft IDs 11/57/58 are candidates, so a changed raft/alpha-edge appearance is a concrete **possible** Forest-1 fidelity regression, not a proven defect or improvement. This is an experiment, not a production-accepted parity fix.

The switch is path-scoped, not level/ID-scoped: the previous `SDL_RENDERER_DIRTY_VISUAL_REVIEW_1529_2026-10-23.md` inventories candidate rafts in other levels. A Forest-only visual check cannot certify the entire enabled footprint. This gate does not reopen the separately handled Forest legibility/ID37 or waterfall ID59 questions.

## Exact working-tree diff

`git diff -- port/linux/SDLRenderer.cpp` at review time (no other renderer hunks):

```diff
diff --git a/port/linux/SDLRenderer.cpp b/port/linux/SDLRenderer.cpp
index 202d17d..0941c91 100644
--- a/port/linux/SDLRenderer.cpp
+++ b/port/linux/SDLRenderer.cpp
@@ -225,7 +225,10 @@ public:
             ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
         SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(
             (sprite.flipX ? SDL_FLIP_HORIZONTAL : 0) | (sprite.flipY ? SDL_FLIP_VERTICAL : 0));
-        if (sprite.rgbGain > 1.f && damageBurstProgram_) {
+        if (RaftMipExperimentEnabled(sprite, sourcePtr) &&
+            DrawRaftMipExperiment(sprite, destination)) {
+            // Private GL draw at this exact painter slot.
+        } else if (sprite.rgbGain > 1.f && damageBurstProgram_) {
             DrawDamageBurst(sprite, texture, destination);
         } else if (sprite.pixelWobble.enabled && pixelWobbleProgram_ && !sourcePtr) {
             DrawPixelWobble(sprite, texture, destination);
@@ -268,6 +271,78 @@ public:
         ++drawCalls_;
         copySeconds_ += std::chrono::duration<double>(std::chrono::steady_clock::now()-began).count();
     }
+    bool RaftMipExperimentEnabled(const Sprite& sprite, const SDL_Rect* source) const {
+        const char* setting = std::getenv("DEATHLEAK_RAFT_MIP_EXPERIMENT");
+        if (!sceneSamplingPhase_ || !setting || std::strcmp(setting, "on") != 0 ||
+            source || sprite.repeatX || sprite.pixelWobble.enabled ||
+            sprite.foliageSway.enabled || sprite.rgbGain > 1.f ||
+            sprite.uvShiftX != 0 || sprite.uvShiftY != 0 ||
+            !SDL_GL_GetCurrentContext()) return false;
+        const auto path = texturePaths_.find(sprite.texture);
+        if (path == texturePaths_.end()) return false;
+        constexpr const char* suffix = "Media/objects/raft.png";
+        const auto& value = path->second;
+        const size_t length = std::strlen(suffix);
+        return value.size() >= length &&
+            value.compare(value.size() - length, length, suffix) == 0 &&
+            (value.size() == length || value[value.size() - length - 1] == '/');
+    }
+    bool DrawRaftMipExperiment(const Sprite& sprite, const SDL_FRect& destination) {
+        if (!std::isfinite(destination.x) || !std::isfinite(destination.y) ||
+            !std::isfinite(destination.w) || !std::isfinite(destination.h) ||
+            destination.w <= 0 || destination.h <= 0) return false;
+        SDL_RenderFlush(renderer_);
+        // Reuse the existing private PNG mip uploader; failed allocation
+        // falls through to the unchanged SDL copy path.
+        const GLuint name = WobbleMipTexture(sprite.texture);
+        if (!name) return false;
+        GLint oldProgram = 0, oldMatrixMode = 0, oldActiveTexture = 0, oldBinding = 0;
+        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
+        glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
+        glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);
+        glPushAttrib(GL_ALL_ATTRIB_BITS);
+        glActiveTexture(GL_TEXTURE0);
+        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
+        glBindTexture(GL_TEXTURE_2D, name);
+        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
+        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
+        glEnable(GL_TEXTURE_2D);
+        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
+        glEnable(GL_BLEND);
+        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
+        glDisable(GL_DEPTH_TEST);
+        glUseProgram(0);
+        glColor4f(sprite.tint.r / 255.f, sprite.tint.g / 255.f,
+                  sprite.tint.b / 255.f, sprite.tint.a / 255.f * sprite.opacity);
+        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
+        glOrtho(0, 1280, 720, 0, -1, 1);
+        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
+        const float radians = sprite.rotationDegrees * 3.14159265358979323846f / 180.f;
+        const float cosine = std::cos(radians), sine = std::sin(radians);
+        const float cx = destination.x + destination.w * .5f;
+        const float cy = destination.y + destination.h * .5f;
+        const float uLeft = sprite.flipX ? 1.f : 0.f, uRight = 1.f - uLeft;
+        // FlipPngRows makes V=1 the authored PNG top.
+        const float vTop = sprite.flipY ? 0.f : 1.f, vBottom = 1.f - vTop;
+        struct Corner { float x, y, u, v; };
+        const Corner corners[4] = {{-.5f,-.5f,uLeft,vTop}, {.5f,-.5f,uRight,vTop},
+                                   {.5f,.5f,uRight,vBottom}, {-.5f,.5f,uLeft,vBottom}};
+        glBegin(GL_QUADS);
+        for (const auto& corner : corners) {
+            const float x = corner.x * destination.w, y = corner.y * destination.h;
+            glTexCoord2f(corner.u, corner.v);
+            glVertex2f(cx + x*cosine-y*sine, cy + x*sine+y*cosine);
+        }
+        glEnd();
+        glMatrixMode(GL_MODELVIEW); glPopMatrix();
+        glMatrixMode(GL_PROJECTION); glPopMatrix();
+        glUseProgram(static_cast<GLuint>(oldProgram));
+        glMatrixMode(oldMatrixMode);
+        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldBinding));
+        glPopAttrib();
+        glActiveTexture(oldActiveTexture);
+        return true;
+    }
     bool DrawFractionalParallax(const Sprite& sprite, SDL_Texture* texture,
                                 const SDL_FRect& destination, int logicalWidth, int logicalHeight) {
         const ParallaxUVScrollInput input{{destination.x, destination.y, destination.w, destination.h},
```

## Source-backed reading

- `port/linux/SDLRenderer.cpp:225-230,271-344`: the predicate requires scene phase, exact `on`, GL context, full-image/no-repeat/no-UV-shift/no-special-effect draw, and a raft-path suffix. It flushes SDL, uploads/reuses a separate PNG GL texture with generated mips, clamps UVs, draws a rotated/flipped quad at the same painter call site, and returns true. A zero upload handle falls back to SDL; the new draw has no post-draw GL-error check and still returns true after GL calls, so a GL draw error could suppress the SDL fallback. That is a conditional failure path, not an observed one.
- `Game/XmlFiles/levels/forest_exploration_1.xml:191-205,928-957`: IDs 11/57/58 are ordinary `effectlighttexture` rafts sharing the filename, each authored alpha 1, nonrepeating. `port/game/LevelScene.cpp:1254-1307` supplies the scene destination and rotation/flips before submitting each sprite. Therefore the new branch specifically has a plausible Forest-1 pixel footprint but does not alter scene painter ordering.
- `Game/ShaderFiles/BasicLightTexture.fx:10-16,53-59` asks for linear minification with linear mip selection and WRAP; `Game/src/Core/TextureManager.cpp:50-79` passes null D3DX10 load info. Neither proves the resource/SRV mip range actually bound by original Windows at a raft draw. The generated GL mips and CLAMP_TO_EDGE are not a demonstrated equivalent. The old SDL path uses a single texture with linear scale mode while scene-minified (`SDLRenderer.cpp:209-225`).
- Existing `RAFT_MIP_RENDERER_EXPERIMENT_2026-10-23.md` reports only syntax compilation and diff check; `RAFT_MIP_WINDOWS_BINDING_GATE_2026-10-23.md` reports no captured bound Windows SRV/resource descriptor or matched lossless Windows frame. `CURRENT_RENDERER_DIRTY_VISUAL_AUDIT_2026-10-23.md` already documents the conditional opacity double-modulation and sampler-boundary uncertainty. Nothing in this review converts those into an observed Forest-1 mismatch.

## Next executable discriminator

In an exclusive visual lane, first verify the current tree and capture the **same frozen Forest-1 camera/object frame** with the variable unset and exactly `on`, at native output resolution, lossless, before HUD/encoding; compare crops of all three raft IDs including alpha boundaries and a non-raft control, and record GL errors and whether the private branch actually drew. Any off/on difference establishes local sensitivity only. To judge *fidelity*, pair that state with an identified original-Windows draw whose bound `txDiffuse` SRV/resource mip range is recorded and with a registered same-state lossless Windows frame (per `RAFT_MIP_WINDOWS_BINDING_GATE_2026-10-23.md`). If the Windows view exposes only mip 0, or the on crop worsens the matched raft pixels, leave the switch off/remove the experiment; if it improves them, constrain the intended scope at the scene submission point by object ID or broaden the acceptance set to differently scaled non-Forest rafts before production use. No broad renderer change is justified by this source-only review.

Review was read-only except this report. `git diff --check -- port/linux/SDLRenderer.cpp` passed. No build, GPU/Xorg, runtime capture, code/test/shared-status edit, commit, or push was performed; unrelated dirty files were preserved.
