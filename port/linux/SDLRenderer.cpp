#include "../render/Renderer.h"
#include "RepeatX.h"
#include "UVWrap.h"
#include "ParallaxUVCommand.h"
#include "WobbleUV.h"
#include "FoliageSway.h"
#include "EffectQuadGeometry.h"
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <png.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace deathleak::render {
namespace {
// PNG rows are top-first; GL level-0 rows are bottom-first. The wobble
// shader's 1-V therefore continues to address the authored top at V=0.
std::vector<png_byte> FlipPngRows(const png_byte* pixels, size_t width, size_t height) {
    const size_t stride = width * 4;
    std::vector<png_byte> bottomFirst(stride * height);
    for (size_t y = 0; y < height; ++y)
        std::memcpy(bottomFirst.data() + y * stride,
                    pixels + (height - 1 - y) * stride, stride);
    return bottomFirst;
}

class SDLRenderer final : public Renderer {
public:
    explicit SDLRenderer(bool testAllowSoftware) : testAllowSoftware_(testAllowSoftware) {}
    ~SDLRenderer() override {
        if (std::getenv("DEATHLEAK_RENDER_PROFILE"))
            std::cerr << "SDL profile calls=" << drawCalls_ << " copy=" << copySeconds_
                      << "s present=" << presentSeconds_ << "s\n";
        if (gamepad_) SDL_GameControllerClose(gamepad_);
        for (const auto& entry : wobbleMipTextures_)
            if (entry.second.name) glDeleteTextures(1, &entry.second.name);
        if (sepiaTexture_) glDeleteTextures(1, &sepiaTexture_);
        if (sepiaProgram_) glDeleteProgram(sepiaProgram_);
        if (pixelWobbleProgram_) glDeleteProgram(pixelWobbleProgram_);
        if (damageBurstProgram_) glDeleteProgram(damageBurstProgram_);
        for (auto& texture : textures_) SDL_DestroyTexture(texture.second);
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        if (initialized_) SDL_Quit();
    }

    bool Initialize(const char* title, int width, int height, std::string& error) override {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) { error = SDL_GetError(); return false; }
        initialized_ = true;
        window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window_) { error = SDL_GetError(); return false; }
        constexpr Uint32 required = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
        renderer_ = SDL_CreateRenderer(window_, -1, required);
        std::string acceleratedError;
        if (!renderer_) {
            acceleratedError = SDL_GetError();
            // Vsync is desirable but not necessary for the acceleration requirement.
            renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer_) acceleratedError += "; without vsync: " + std::string(SDL_GetError());
        }
        if (!renderer_ && testAllowSoftware_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        if (!renderer_) {
            error = "accelerated SDL renderer required; attempts failed: " + acceleratedError;
            if (testAllowSoftware_) error += "; test-only software attempt: " + std::string(SDL_GetError());
            std::cerr << "SDL renderer selection failed (mode="
                      << (testAllowSoftware_ ? "test-allow-software" : "strict-accelerated")
                      << "): " << error << "\n";
            return false;
        }
        SDL_RendererInfo info{};
        if (SDL_GetRendererInfo(renderer_, &info) != 0) {
            error = "cannot inspect selected SDL renderer: " + std::string(SDL_GetError());
            return false;
        }
        std::cerr << "SDL renderer selected video_driver="
                  << (SDL_GetCurrentVideoDriver() ? SDL_GetCurrentVideoDriver() : "unknown")
                  << " name=" << (info.name ? info.name : "unknown")
                  << " flags=" << info.flags
                  << " accelerated=" << ((info.flags & SDL_RENDERER_ACCELERATED) ? 1 : 0)
                  << " software=" << ((info.flags & SDL_RENDERER_SOFTWARE) ? 1 : 0)
                  << " vsync=" << ((info.flags & SDL_RENDERER_PRESENTVSYNC) ? 1 : 0)
                  << " mode=" << (testAllowSoftware_ ? "test-allow-software" : "strict-accelerated") << "\n";
        if (!(info.flags & SDL_RENDERER_ACCELERATED) || (info.flags & SDL_RENDERER_SOFTWARE)) {
            if (!testAllowSoftware_) {
                error = "selected SDL renderer is not accelerated; refusing software rendering";
                return false;
            }
            std::cerr << "TEST-ONLY software renderer accepted; this is not release/GPU evidence\n";
        }
        // SDL's ACCELERATED flag also covers OpenGL implemented by Mesa's CPU
        // rasterizers. Query the live GL context before claiming a hardware path.
        const std::string driver = info.name ? info.name : "";
        if ((info.flags & SDL_RENDERER_ACCELERATED) && !(info.flags & SDL_RENDERER_SOFTWARE) &&
            driver != "opengl" && driver != "opengles2") {
            if (!testAllowSoftware_) {
                error = "physical GPU renderer required; cannot verify SDL driver " + driver;
                return false;
            }
            std::cerr << "TEST-ONLY unverified renderer accepted; this is not release/GPU evidence\n";
        }
        if (driver == "opengl" || driver == "opengles2") {
            using GetString = const unsigned char* (*)(unsigned int);
            auto getString = reinterpret_cast<GetString>(SDL_GL_GetProcAddress("glGetString"));
            const char* vendor = getString && SDL_GL_GetCurrentContext()
                ? reinterpret_cast<const char*>(getString(0x1F00)) : nullptr; // GL_VENDOR
            const char* device = getString && SDL_GL_GetCurrentContext()
                ? reinterpret_cast<const char*>(getString(0x1F01)) : nullptr; // GL_RENDERER
            std::cerr << "OpenGL device vendor=" << (vendor ? vendor : "unavailable")
                      << " renderer=" << (device ? device : "unavailable") << "\n";
            std::string identity = std::string(vendor ? vendor : "") + " " + (device ? device : "");
            std::transform(identity.begin(), identity.end(), identity.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            const bool softwareGL = identity.find("llvmpipe") != std::string::npos ||
                identity.find("softpipe") != std::string::npos ||
                identity.find("swrast") != std::string::npos ||
                identity.find("software rasterizer") != std::string::npos;
            if (!vendor || !device || softwareGL) {
                if (!testAllowSoftware_) {
                    error = "physical GPU OpenGL renderer required; software or unidentified GL device refused";
                    return false;
                }
                std::cerr << "TEST-ONLY software/unidentified OpenGL accepted; this is not release/GPU evidence\n";
            }
        }
        if (SDL_RenderSetLogicalSize(renderer_, 1280, 720) != 0) { error = SDL_GetError(); return false; }
        if (driver == "opengl") {
            if (!InitializeSepia(error)) return false;
            if (!InitializePixelWobble(error)) return false;
            if (!InitializeDamageBurst(error)) return false;
        } else if (!testAllowSoftware_) {
            error = "Sepia.fx requires the SDL OpenGL renderer; selected " + driver;
            return false;
        } else {
            std::cerr << "TEST-ONLY renderer: Sepia.fx GPU pass unavailable\n";
        }
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        for (int i = 0; i < SDL_NumJoysticks(); ++i)
            if (SDL_IsGameController(i)) { gamepad_ = SDL_GameControllerOpen(i); break; }
        return true;
    }

    TextureId LoadTexture(const std::string& path, std::string& error) override {
        auto cached = paths_.find(path);
        if (cached != paths_.end()) return cached->second;
        const bool traceTexture = std::getenv("DEATHLEAK_TRACE_TEXTURE_LOAD") != nullptr;
        const auto textureStart = std::chrono::steady_clock::now();
        auto phaseMs = [&] { return std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - textureStart).count(); };
        png_image image{};
        image.version = PNG_IMAGE_VERSION;
        if (!png_image_begin_read_from_file(&image, path.c_str())) {
            error = image.message; return InvalidTexture;
        }
        const double headerMs = traceTexture ? phaseMs() : 0;
        image.format = PNG_FORMAT_RGBA;
        std::vector<png_byte> pixels(PNG_IMAGE_SIZE(image));
        if (!png_image_finish_read(&image, nullptr, pixels.data(), 0, nullptr)) {
            error = image.message; png_image_free(&image); return InvalidTexture;
        }
        const double decodeMs = traceTexture ? phaseMs() : 0;
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(),
            static_cast<int>(image.width), static_cast<int>(image.height), 32,
            static_cast<int>(image.width * 4), SDL_PIXELFORMAT_RGBA32);
        if (!surface) { error = SDL_GetError(); png_image_free(&image); return InvalidTexture; }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_FreeSurface(surface);
        if (!texture) { error = SDL_GetError(); png_image_free(&image); return InvalidTexture; }
        if (traceTexture)
            std::cerr << "Texture load " << path << " size=" << image.width << "x" << image.height
                      << " header_ms=" << headerMs << " decode_ms=" << decodeMs - headerMs
                      << " upload_ms=" << phaseMs() - decodeMs << "\n";
        png_image_free(&image);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        TextureId id = nextId_++;
        textures_.emplace(id, texture);
        paths_.emplace(path, id);
        texturePaths_.emplace(id, path);
        return id;
    }

    void BeginFrame(Color clear) override {
        sceneSamplingPhase_ = true;
        worldZoom_ = 1.f;
        SDL_SetRenderDrawColor(renderer_, clear.r, clear.g, clear.b, clear.a);
        SDL_RenderClear(renderer_);
    }
    void SetWorldZoom(float zoom) override {
        worldZoom_ = sceneSamplingPhase_ && std::isfinite(zoom) && zoom > 0.f ? zoom : 1.f;
    }
    SDL_FRect ProjectWorld(SDL_FRect rect) const {
        const Rect projected = ProjectWorldRect({rect.x, rect.y, rect.w, rect.h}, worldZoom_);
        return {projected.x, projected.y, projected.width, projected.height};
    }
    void DrawSprite(const Sprite& sprite) override {
        const auto began = std::chrono::steady_clock::now();
        auto it = textures_.find(sprite.texture);
        if (it == textures_.end()) return;
        SDL_Texture* texture = it->second;
        SDL_Rect source;
        SDL_Rect* sourcePtr = nullptr;
        if (sprite.source.width > 0 && sprite.source.height > 0) {
            source = {static_cast<int>(sprite.source.x), static_cast<int>(sprite.source.y),
                      static_cast<int>(sprite.source.width), static_cast<int>(sprite.source.height)};
            sourcePtr = &source;
        }
        SDL_FRect authoredDestination{sprite.destination.x, sprite.destination.y,
                                      sprite.destination.width, sprite.destination.height};
        SDL_FRect destination = ProjectWorld(authoredDestination);
        SDL_SetTextureColorMod(texture, sprite.tint.r, sprite.tint.g, sprite.tint.b);
        SDL_SetTextureAlphaMod(texture, sprite.tint.a);
        // BasicLightTexture.fx and the other ordinary scene shaders use
        // MIN_LINEAR_MAG_POINT. SDL exposes one scale mode for both directions,
        // so select linear only while this scene copy is minified; later HUD
        // copies retain nearest sampling, including when reusing a texture.
        int textureWidth = 0, textureHeight = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
        const int sampledWidth = sourcePtr ? source.w : textureWidth;
        const int sampledHeight = sourcePtr ? source.h : textureHeight;
        const bool minified = sprite.repeatX ||
            (sampledWidth > 0 && destination.w < sampledWidth) ||
            (sampledHeight > 0 && destination.h < sampledHeight);
        SDL_SetTextureScaleMode(texture, sceneSamplingPhase_ && minified
            ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
        SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(
            (sprite.flipX ? SDL_FLIP_HORIZONTAL : 0) | (sprite.flipY ? SDL_FLIP_VERTICAL : 0));
        if (sprite.rgbGain > 1.f && damageBurstProgram_) {
            DrawDamageBurst(sprite, texture, destination);
        } else if (sprite.pixelWobble.enabled && pixelWobbleProgram_ && !sourcePtr) {
            DrawPixelWobble(sprite, texture, destination);
        } else if (sprite.foliageSway.enabled && !sourcePtr) {
            DrawFoliageSway(sprite, texture, authoredDestination);
        } else if (sprite.repeatX && sprite.rotationDegrees == 0 && !sourcePtr) {
            int width = 0, height = 0;
            SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
            // The scene is authored in 1920x1080 world units and displayed at 2/3.
            const float tileWidth = width * (2.f / 3.f);
            if (tileWidth > 0) {
                ForEachRepeatXTile(authoredDestination.x, authoredDestination.w, tileWidth, width,
                                   1280.f, sprite.flipX, [&](RepeatXTile tile) {
                    SDL_Rect part{0, 0, tile.sourceWidth, height};
                    SDL_FRect target{authoredDestination.x + tile.offset, authoredDestination.y,
                                     tile.width, authoredDestination.h};
                    target = ProjectWorld(target);
                    SDL_RenderCopyExF(renderer_, texture, &part, &target, 0, nullptr, flip);
                });
            }
        } else if ((sprite.uvShiftX != 0 || sprite.uvShiftY != 0) && sprite.rotationDegrees == 0 && !sourcePtr) {
            // D3D's ParallaxLayer scrolls one full texture UV over its authored quad.
            if (!DrawFractionalParallax(sprite, texture, destination, textureWidth, textureHeight)) {
                // Preserve the original integer-copy fallback.
                ForEachUVWrapSegment(textureHeight, sprite.uvShiftY, sprite.flipY, [&](UVWrapSegment y) {
                    ForEachUVWrapSegment(textureWidth, sprite.uvShiftX, sprite.flipX, [&](UVWrapSegment x) {
                        SDL_Rect part{x.source, y.source, x.length, y.length};
                        SDL_FRect target{destination.x + destination.w * x.offset,
                                         destination.y + destination.h * y.offset,
                                         destination.w * x.length / textureWidth,
                                         destination.h * y.length / textureHeight};
                        SDL_RenderCopyExF(renderer_, texture, &part, &target, 0, nullptr, flip);
                    });
                });
            }
        } else if (sprite.rotationDegrees == 0 && flip == SDL_FLIP_NONE)
            SDL_RenderCopyF(renderer_, texture, sourcePtr, &destination);
        else
            SDL_RenderCopyExF(renderer_, texture, sourcePtr, &destination,
                              sprite.rotationDegrees, nullptr, flip);
        ++drawCalls_;
        copySeconds_ += std::chrono::duration<double>(std::chrono::steady_clock::now()-began).count();
    }
    bool DrawFractionalParallax(const Sprite& sprite, SDL_Texture* texture,
                                const SDL_FRect& destination, int logicalWidth, int logicalHeight) {
        const ParallaxUVScrollInput input{{destination.x, destination.y, destination.w, destination.h},
            sprite.uvShiftX, sprite.uvShiftY, sprite.flipX, sprite.flipY, true, true};
        ParallaxUVBinding binding{};
        auto command = PrepareParallaxUVCommand(input, binding);
        if (command.kind == ParallaxUVCommandKind::NotApplicable ||
            !std::isfinite(input.shiftX) || !std::isfinite(input.shiftY) ||
            !std::isfinite(destination.x) || !std::isfinite(destination.y) ||
            !std::isfinite(destination.w) || !std::isfinite(destination.h) ||
            destination.w <= 0.f || destination.h <= 0.f ||
            logicalWidth <= 0 || logicalHeight <= 0 || !SDL_GL_GetCurrentContext())
            return false;
        SDL_RendererInfo info{};
        if (SDL_GetRendererInfo(renderer_, &info) != 0 || !info.name ||
            std::strcmp(info.name, "opengl") != 0) return false;
        binding.glRenderer = true;
        if (SDL_RenderFlush(renderer_) != 0) return false;
        GLint oldProgram = 0, oldMatrixMode = 0, oldActiveTexture = 0, oldBinding = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glActiveTexture(GL_TEXTURE0);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
        bool bound = false, samplerChanged = false, projectionPushed = false, modelviewPushed = false;
        bool drawn = false;
        GLint oldWrapS = 0, oldWrapT = 0, oldMin = 0, oldMag = 0;
        GLint allocationWidth = 0, allocationHeight = 0, object = 0;
        float spanX = 0.f, spanY = 0.f;
        bool npotSupported = false;
        do {
            if (SDL_GL_BindTexture(texture, &spanX, &spanY) != 0) break;
            bound = true;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &object);
            if (object == 0) break;
            while (glGetError() != GL_NO_ERROR) {}
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &allocationWidth);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &allocationHeight);
            if (glGetError() != GL_NO_ERROR || allocationWidth <= 0 || allocationHeight <= 0) break;
            auto powerOfTwo = [](int n) { return n > 0 && (n & (n - 1)) == 0; };
            binding.npotTexture = !powerOfTwo(allocationWidth) || !powerOfTwo(allocationHeight);
            const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
            if (version && version[0] >= '0' && version[0] <= '9') {
                char* end = nullptr;
                const long major = std::strtol(version, &end, 10);
                if (end && *end == '.' && end[1] >= '0' && end[1] <= '9') {
                    const long minor = std::strtol(end + 1, &end, 10);
                    npotSupported = (*end == '\0' || *end == ' ') &&
                        (major > 2 || (major == 2 && minor >= 0));
                    if (!npotSupported) {
                        const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
                        const char* name = "GL_ARB_texture_non_power_of_two";
                        const size_t length = std::strlen(name);
                        for (const char* p = extensions; p && (p = std::strstr(p, name)); p += length)
                            if ((p == extensions || p[-1] == ' ') &&
                                (p[length] == ' ' || p[length] == '\0')) { npotSupported = true; break; }
                    }
                }
            }
            binding.bindSucceeded = true;
            binding.spanX = spanX; binding.spanY = spanY;
            binding.npotRepeatSupported = npotSupported;
            command = PrepareParallaxUVCommand(input, binding);
            if (command.kind != ParallaxUVCommandKind::FloatQuad) break;
            while (glGetError() != GL_NO_ERROR) {}
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &oldWrapS);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &oldWrapT);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &oldMin);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &oldMag);
            if (glGetError() != GL_NO_ERROR) break;
            samplerChanged = true;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            if (glGetError() != GL_NO_ERROR) break;
            glUseProgram(0);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            glColor4f(sprite.tint.r / 255.f, sprite.tint.g / 255.f,
                      sprite.tint.b / 255.f, sprite.tint.a / 255.f);
            glMatrixMode(GL_PROJECTION); glPushMatrix(); projectionPushed = true; glLoadIdentity();
            glOrtho(0, 1280, 720, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW); glPushMatrix(); modelviewPushed = true; glLoadIdentity();
            if (glGetError() != GL_NO_ERROR) break;
            const float x[4] = {destination.x, destination.x + destination.w,
                                destination.x + destination.w, destination.x};
            const float y[4] = {destination.y, destination.y,
                                destination.y + destination.h, destination.y + destination.h};
            glBegin(GL_QUADS);
            for (int i = 0; i < 4; ++i) {
                glTexCoord2f(command.uv[i].u, command.uv[i].v);
                glVertex2f(x[i], y[i]);
            }
            glEnd();
            drawn = true;
        } while (false);
        if (samplerChanged) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, oldWrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, oldWrapT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, oldMin);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, oldMag);
        }
        if (modelviewPushed) { glMatrixMode(GL_MODELVIEW); glPopMatrix(); }
        if (projectionPushed) { glMatrixMode(GL_PROJECTION); glPopMatrix(); }
        glUseProgram(static_cast<GLuint>(oldProgram));
        glMatrixMode(oldMatrixMode);
        if (bound) SDL_GL_UnbindTexture(texture);
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldBinding));
        glPopAttrib();
        glActiveTexture(oldActiveTexture);
        if (parallaxLogged_.insert(sprite.texture).second)
            std::cerr << "Parallax UV texture=" << sprite.texture
                      << " logical=" << logicalWidth << "x" << logicalHeight
                      << " allocated=" << allocationWidth << "x" << allocationHeight
                      << " span=" << spanX << "," << spanY
                      << " npot_repeat=" << npotSupported
                      << " path=" << (drawn ? "float" : "legacy") << "\n";
        return drawn;
    }
    void DrawDamageBurst(const Sprite& sprite, SDL_Texture* texture, const SDL_FRect& destination) {
        SDL_RenderFlush(renderer_);
        GLint oldProgram = 0, oldMatrixMode = 0, oldActiveTexture = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glActiveTexture(GL_TEXTURE0);
        float textureSpanX = 1.f, textureSpanY = 1.f;
        if (SDL_GL_BindTexture(texture, &textureSpanX, &textureSpanY) != 0) {
            glPopAttrib(); glActiveTexture(oldActiveTexture); return;
        }
        int textureWidth = 0, textureHeight = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
        const float left = sprite.source.width > 0.f ? sprite.source.x / textureWidth : 0.f;
        const float right = sprite.source.width > 0.f ? (sprite.source.x + sprite.source.width) / textureWidth : 1.f;
        const float top = sprite.source.height > 0.f ? sprite.source.y / textureHeight : 0.f;
        const float bottom = sprite.source.height > 0.f ? (sprite.source.y + sprite.source.height) / textureHeight : 1.f;
        GLint oldMin = 0, oldMag = 0;
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &oldMin);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &oldMag);
        glEnable(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrtho(0, 1280, 720, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glUseProgram(damageBurstProgram_);
        glUniform1f(glGetUniformLocation(damageBurstProgram_, "rgbGain"), sprite.rgbGain);
        glUniform3f(glGetUniformLocation(damageBurstProgram_, "tintRgb"),
                    sprite.tint.r / 255.f, sprite.tint.g / 255.f, sprite.tint.b / 255.f);
        glUniform1f(glGetUniformLocation(damageBurstProgram_, "spriteAlpha"),
                    sprite.tint.a / 255.f * sprite.opacity);
        glUniform2f(glGetUniformLocation(damageBurstProgram_, "textureSpan"), textureSpanX, textureSpanY);
        const float radians = sprite.rotationDegrees * 3.14159265358979323846f / 180.f;
        const float cosine = std::cos(radians), sine = std::sin(radians);
        const float cx = destination.x + destination.w * .5f, cy = destination.y + destination.h * .5f;
        const float uLeft = sprite.flipX ? right : left, uRight = sprite.flipX ? left : right;
        // Match the existing custom shader's SDL-owned texture orientation.
        const float vTop = sprite.flipY ? bottom : top, vBottom = sprite.flipY ? top : bottom;
        struct Corner { float x, y, u, v; };
        const Corner corners[4] = {
            {-.5f,-.5f,uLeft,vTop}, {.5f,-.5f,uRight,vTop},
            {.5f,.5f,uRight,vBottom}, {-.5f,.5f,uLeft,vBottom}
        };
        glBegin(GL_QUADS);
        for (const auto& corner : corners) {
            const float x = corner.x * destination.w, y = corner.y * destination.h;
            glTexCoord2f(corner.u, corner.v);
            glVertex2f(cx + x*cosine-y*sine, cy + x*sine+y*cosine);
        }
        glEnd();
        glUseProgram(static_cast<GLuint>(oldProgram));
        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(oldMatrixMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, oldMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, oldMag);
        SDL_GL_UnbindTexture(texture);
        glPopAttrib();
        glActiveTexture(oldActiveTexture);
    }
    void DrawPixelWobble(const Sprite& sprite, SDL_Texture* texture, const SDL_FRect& destination) {
        // SDL owns the ordinary texture; the complete mip chain is private.
        SDL_RenderFlush(renderer_);
        GLint oldProgram = 0, oldMatrixMode = 0, oldActiveTexture = 0, oldBinding = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glActiveTexture(GL_TEXTURE0);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
        float textureWidth = 1.f, textureHeight = 1.f;
        const GLuint privateTexture = WobbleMipTexture(sprite.texture);
        const bool usePrivate = privateTexture != 0;
        bool sdlBound = false;
        GLint oldWrapS = 0, oldWrapT = 0, oldMin = 0, oldMag = 0;
        if (usePrivate) {
            glBindTexture(GL_TEXTURE_2D, privateTexture);
        } else {
            if (SDL_GL_BindTexture(texture, &textureWidth, &textureHeight) != 0) {
                glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldBinding));
                glPopAttrib(); glActiveTexture(oldActiveTexture); return;
            }
            sdlBound = true;
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &oldWrapS);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &oldWrapT);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &oldMin);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &oldMag);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrtho(0, 1280, 720, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glUseProgram(pixelWobbleProgram_);
        glUniform1f(glGetUniformLocation(pixelWobbleProgram_, "effectTime"), sprite.pixelWobble.time);
        glUniform1f(glGetUniformLocation(pixelWobbleProgram_, "speedMod"), sprite.pixelWobble.speedMod);
        glUniform1f(glGetUniformLocation(pixelWobbleProgram_, "intensity"), sprite.pixelWobble.intensity);
        glUniform1f(glGetUniformLocation(pixelWobbleProgram_, "spriteAlpha"), sprite.opacity);
        glUniform2f(glGetUniformLocation(pixelWobbleProgram_, "textureSpan"), textureWidth, textureHeight);
        const float cx = destination.x + destination.w * .5f;
        const float cy = destination.y + destination.h * .5f;
        glBegin(GL_QUADS);
        for (const auto& corner : {std::pair<bool, bool>{false, false}, {true, false},
                                  {true, true}, {false, true}}) {
            const auto vertex = PixelWobbleQuadVertex(cx, cy, destination.w, destination.h,
                sprite.rotationDegrees, sprite.flipX, sprite.flipY,
                sprite.uvShiftX, sprite.uvShiftY, corner.first, corner.second);
            glTexCoord2f(vertex.u, vertex.v);
            glVertex2f(vertex.x, vertex.y);
        }
        glEnd();
        glUseProgram(static_cast<GLuint>(oldProgram));
        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(oldMatrixMode);
        if (sdlBound) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, oldWrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, oldWrapT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, oldMin);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, oldMag);
            SDL_GL_UnbindTexture(texture);
        }
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldBinding));
        glPopAttrib();
        glActiveTexture(oldActiveTexture);
    }
    void DrawFoliageSway(const Sprite& sprite, SDL_Texture* texture, const SDL_FRect& destination) {
        int textureWidth = 0, textureHeight = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
        if (textureWidth <= 0 || textureHeight <= 0) return;
        const auto& sway = sprite.foliageSway;
        const float scale = 2.f / 3.f;
        const float cx = destination.x + destination.w * .5f;
        const float cy = destination.y + destination.h * .5f;
        // Preserve BL/BR/TR/TL source-vertex intensity identity before the
        // flip, world rotation and X-before-Y sway.
        struct Corner { float x, y; SwayIntensity amount; };
        const Corner corners[4] = {
            {-1.f, -1.f, sway.bottomLeft}, {1.f, -1.f, sway.bottomRight},
            {1.f, 1.f, sway.topRight}, {-1.f, 1.f, sway.topLeft}
        };
        SDL_FPoint point[4];
        for (int i = 0; i < 4; ++i) {
            const auto p = FoliageQuadVertex(cx, cy, destination.w, destination.h,
                sprite.rotationDegrees, sprite.flipX, sprite.flipY,
                corners[i].x, corners[i].y,
                {corners[i].amount.x, corners[i].amount.y},
                {sway.worldX, sway.worldY}, sway.time, sway.timeMultiplier,
                sway.globalTimeMultiplier);
            point[i] = {p.x, p.y};
        }
        const int topLeft = sprite.flipY ? 0 : 3, topRight = sprite.flipY ? 1 : 2;
        const int bottomLeft = sprite.flipY ? 3 : 0, bottomRight = sprite.flipY ? 2 : 1;
        const SDL_FPoint tl = point[sprite.flipX ? topRight : topLeft];
        const SDL_FPoint tr = point[sprite.flipX ? topLeft : topRight];
        const SDL_FPoint bl = point[sprite.flipX ? bottomRight : bottomLeft];
        const SDL_FPoint br = point[sprite.flipX ? bottomLeft : bottomRight];
        const SDL_Color white{255, 255, 255, 255};
        auto drawTile = [&](float fractionLeft, float fractionRight, float uLeft, float uRight) {
            const auto lerp = [](SDL_FPoint a, SDL_FPoint b, float t) {
                return SDL_FPoint{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
            };
            const float vTop = FoliageScreenV(sprite.flipY, true);
            const float vBottom = FoliageScreenV(sprite.flipY, false);
            SDL_Vertex vertices[4] = {
                {lerp(tl, tr, fractionLeft), white, {uLeft, vTop}},
                {lerp(tl, tr, fractionRight), white, {uRight, vTop}},
                {lerp(bl, br, fractionRight), white, {uRight, vBottom}},
                {lerp(bl, br, fractionLeft), white, {uLeft, vBottom}}
            };
            if (worldZoom_ != 1.f) for (auto& vertex : vertices) {
                vertex.position.x = 640.f + (vertex.position.x - 640.f) / worldZoom_;
                vertex.position.y = 360.f + (vertex.position.y - 360.f) / worldZoom_;
            }
            const int indices[6] = {0, 1, 3, 1, 2, 3};
            SDL_RenderGeometry(renderer_, texture, vertices, 4, indices, 6);
        };
        if (sprite.repeatX && sprite.rotationDegrees == 0) {
            const float tileWidth = textureWidth * scale;
            ForEachRepeatXTile(destination.x, destination.w, tileWidth, textureWidth,
                               1280.f, sprite.flipX, [&](RepeatXTile tile) {
                const float from = tile.offset / destination.w;
                const float to = (tile.offset + tile.width) / destination.w;
                const float fraction = static_cast<float>(tile.sourceWidth) / textureWidth;
                drawTile(from, to, sprite.flipX ? fraction : 0.f,
                         sprite.flipX ? 0.f : fraction);
            });
        } else {
            drawTile(0.f, 1.f, sprite.flipX ? 1.f : 0.f,
                     sprite.flipX ? 0.f : 1.f);
        }
    }
    void FillRect(Rect rect, Color color) override {
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_FRect box = ProjectWorld({rect.x, rect.y, rect.width, rect.height});
        SDL_RenderFillRectF(renderer_, &box);
    }
    void ApplyScenePostprocess() override {
        sceneSamplingPhase_ = false;
        worldZoom_ = 1.f;
        if (!sepiaProgram_) return; // Only the explicit test-only software path.
        SDL_RenderFlush(renderer_);
        int width = 0, height = 0;
        SDL_GetRendererOutputSize(renderer_, &width, &height);
        if (width <= 0 || height <= 0) return;

        GLint oldProgram = 0, oldMatrixMode = 0, oldActiveTexture = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sepiaTexture_);
        // Copy the already-composited scene on the GPU. No CPU readback or
        // per-sprite shader approximation; HUD is drawn after this pass.
        if (width != sepiaWidth_ || height != sepiaHeight_) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            sepiaWidth_ = width;
            sepiaHeight_ = height;
        }
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
        glViewport(0, 0, width, height);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glUseProgram(sepiaProgram_);
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(-1, -1);
        glTexCoord2f(1, 0); glVertex2f( 1, -1);
        glTexCoord2f(0, 1); glVertex2f(-1,  1);
        glTexCoord2f(1, 1); glVertex2f( 1,  1);
        glEnd();
        glUseProgram(static_cast<GLuint>(oldProgram));
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(oldMatrixMode);
        glPopAttrib();
        glActiveTexture(oldActiveTexture);
    }
    void EndFrame() override {
        const auto began = std::chrono::steady_clock::now();
        SDL_RenderPresent(renderer_);
        presentSeconds_ += std::chrono::duration<double>(std::chrono::steady_clock::now()-began).count();
    }
    bool PollQuit() override {
        SDL_Event event;
        bool quit = false;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;
            if (event.type == SDL_CONTROLLERDEVICEADDED && !gamepad_ && SDL_IsGameController(event.cdevice.which))
                gamepad_ = SDL_GameControllerOpen(event.cdevice.which);
            if (event.type == SDL_CONTROLLERDEVICEREMOVED && gamepad_ &&
                SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad_)) == event.cdevice.which) {
                SDL_GameControllerClose(gamepad_); gamepad_ = nullptr;
            }
        }
        return quit;
    }
    bool KeyDown(int key) const override {
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        auto button = [this](SDL_GameControllerButton b) { return gamepad_ && SDL_GameControllerGetButton(gamepad_, b); };
        auto axis = [this](SDL_GameControllerAxis a) { return gamepad_ ? SDL_GameControllerGetAxis(gamepad_, a) : 0; };
        switch (key) {
            case Left: return keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A] || button(SDL_CONTROLLER_BUTTON_DPAD_LEFT) || axis(SDL_CONTROLLER_AXIS_LEFTX) < -8000;
            case Right: return keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D] || button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || axis(SDL_CONTROLLER_AXIS_LEFTX) > 8000;
            case Up: return keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_SPACE] || button(SDL_CONTROLLER_BUTTON_DPAD_UP) || button(SDL_CONTROLLER_BUTTON_A);
            case Down: return keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S] || button(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            case Fire: return keys[SDL_SCANCODE_F];
            case Restart: return keys[SDL_SCANCODE_R];
            case Interact: return keys[SDL_SCANCODE_E] || button(SDL_CONTROLLER_BUTTON_X);
            case Purchase: return keys[SDL_SCANCODE_Y] || button(SDL_CONTROLLER_BUTTON_Y);
            case Roll: return keys[SDL_SCANCODE_LSHIFT] || button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            case Melee: return keys[SDL_SCANCODE_C] || button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            case Escape: return keys[SDL_SCANCODE_ESCAPE] || button(SDL_CONTROLLER_BUTTON_START);
            case ModalBack: return button(SDL_CONTROLLER_BUTTON_B);
            default: return false;
        }
    }
    ControllerSticks ControllerAxes() const override {
        if (!gamepad_ || !SDL_GameControllerGetAttached(gamepad_)) return {};
        return {SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_LEFTX),
                SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_RIGHTX),
                SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_RIGHTY), true};
    }
    MouseState Mouse() const override {
        int x = 0, y = 0;
        const Uint32 buttons = SDL_GetMouseState(&x, &y);
        float logicalX = -1, logicalY = -1;
        SDL_RenderWindowToLogical(renderer_, x, y, &logicalX, &logicalY);
        return {logicalX, logicalY, (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0};
    }
    bool SaveFrame(const std::string& path, std::string& error) override {
        int width = 0, height = 0;
        if (SDL_GetRendererOutputSize(renderer_, &width, &height) != 0) { error = SDL_GetError(); return false; }
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
        if (!surface) { error = SDL_GetError(); return false; }
        bool ok = SDL_RenderReadPixels(renderer_, nullptr, surface->format->format,
                                       surface->pixels, surface->pitch) == 0 &&
                  SDL_SaveBMP(surface, path.c_str()) == 0;
        if (!ok) error = SDL_GetError();
        SDL_FreeSurface(surface);
        return ok;
    }
private:
    struct WobbleMip {
        GLuint name = 0;
        bool attempted = false;
    };
    GLuint WobbleMipTexture(TextureId id) {
        auto& entry = wobbleMipTextures_[id];
        if (entry.attempted) return entry.name;
        entry.attempted = true; // Cache failures too; do not decode every frame.
        std::string failure;
        auto path = texturePaths_.find(id);
        if (path == texturePaths_.end()) failure = "no source path";

        // Full NPOT repeat+mipmap support is core in desktop GL 2.0. Mipmap
        // generation itself needs core 3.0 or EXT_framebuffer_object.
        int major = 0, minor = 0;
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        if (failure.empty() && (!version || std::sscanf(version, "%d.%d", &major, &minor) != 2 ||
            major < 2)) failure = "desktop GL 2.0 NPOT support unavailable";
        const bool coreGenerator = major >= 3;
        const char* extensions = coreGenerator ? nullptr :
            reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        const bool extGenerator = extensions &&
            std::strstr(extensions, "GL_EXT_framebuffer_object") != nullptr;
        using GenerateMipmap = void (APIENTRY *)(GLenum);
        auto generate = reinterpret_cast<GenerateMipmap>(SDL_GL_GetProcAddress(
            coreGenerator ? "glGenerateMipmap" : "glGenerateMipmapEXT"));
        if (failure.empty() && (!generate || (!coreGenerator && !extGenerator)))
            failure = "mipmap generation entry point unavailable";

        png_image image{};
        image.version = PNG_IMAGE_VERSION;
        std::vector<png_byte> pixels;
        if (failure.empty()) {
            if (!png_image_begin_read_from_file(&image, path->second.c_str()))
                failure = image.message;
            else {
                image.format = PNG_FORMAT_RGBA;
                pixels.resize(PNG_IMAGE_SIZE(image));
                if (!png_image_finish_read(&image, nullptr, pixels.data(), 0, nullptr))
                    failure = image.message;
            }
        }
        GLuint name = 0;
        if (failure.empty()) {
            GLint oldBinding = 0, oldUnpack = 0, oldRowLength = 0;
            GLint oldSkipRows = 0, oldSkipPixels = 0, oldUnpackBuffer = 0;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
            glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpack);
            glGetIntegerv(GL_UNPACK_ROW_LENGTH, &oldRowLength);
            glGetIntegerv(GL_UNPACK_SKIP_ROWS, &oldSkipRows);
            glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &oldSkipPixels);
            glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &oldUnpackBuffer);
            // SDL leaves its atlas row length active. A standalone PNG upload
            // must use its own tightly packed rows, then restore SDL's state.
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            // This isolated upload must not inherit stale errors from SDL.
            while (glGetError() != GL_NO_ERROR) {}
            bool uploaded = false;
            glGenTextures(1, &name);
            if (name) {
                glBindTexture(GL_TEXTURE_2D, name);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                const auto bottomFirst = FlipPngRows(pixels.data(), image.width, image.height);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                    static_cast<GLsizei>(image.width), static_cast<GLsizei>(image.height),
                    0, GL_RGBA, GL_UNSIGNED_BYTE, bottomFirst.data());
                if (glGetError() == GL_NO_ERROR) {
                    generate(GL_TEXTURE_2D);
                    if (glGetError() == GL_NO_ERROR) {
                        const int largest = std::max(image.width, image.height);
                        int maxLevel = 0;
                        for (int size = largest; size > 1; size >>= 1) ++maxLevel;
                        int expectedWidth = static_cast<int>(image.width);
                        int expectedHeight = static_cast<int>(image.height);
                        bool complete = true;
                        for (int level = 0; level <= maxLevel; ++level) {
                            GLint width = 0, height = 0;
                            glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
                            glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &height);
                            complete &= width == expectedWidth && height == expectedHeight;
                            expectedWidth = std::max(1, expectedWidth / 2);
                            expectedHeight = std::max(1, expectedHeight / 2);
                        }
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxLevel);
                        uploaded = complete && glGetError() == GL_NO_ERROR;
                    }
                }
            }
            if (!name || !uploaded)
                failure = "GL allocation, mip generation, or sampler setup failed";
            glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpack);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, oldRowLength);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, oldSkipRows);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, oldSkipPixels);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(oldUnpackBuffer));
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldBinding));
        }
        png_image_free(&image);
        if (!failure.empty()) {
            if (name) glDeleteTextures(1, &name);
            std::cerr << "PixelWobble mip fallback for texture " << id << ": "
                      << failure << "; using SDL linear minification\n";
            return 0;
        }
        entry.name = name;
        return name;
    }
    bool InitializeDamageBurst(std::string& error) {
        constexpr const char* source = R"(
            #version 120
            uniform sampler2D diffuse;
            uniform float rgbGain, spriteAlpha;
            uniform vec3 tintRgb;
            uniform vec2 textureSpan;
            void main() {
                vec4 sampled = texture2D(diffuse, gl_TexCoord[0].xy * textureSpan);
                gl_FragColor = vec4(sampled.rgb * tintRgb * rgbGain, sampled.a * spriteAlpha);
            }
        )";
        GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            char log[1024]{};
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            error = "damage burst shader compile failed: " + std::string(log);
            glDeleteShader(shader);
            return false;
        }
        damageBurstProgram_ = glCreateProgram();
        glAttachShader(damageBurstProgram_, shader);
        glLinkProgram(damageBurstProgram_);
        glDeleteShader(shader);
        GLint linked = GL_FALSE;
        glGetProgramiv(damageBurstProgram_, GL_LINK_STATUS, &linked);
        if (!linked) {
            char log[1024]{};
            glGetProgramInfoLog(damageBurstProgram_, sizeof(log), nullptr, log);
            error = "damage burst shader link failed: " + std::string(log);
            return false;
        }
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(damageBurstProgram_);
        glUniform1i(glGetUniformLocation(damageBurstProgram_, "diffuse"), 0);
        glUseProgram(static_cast<GLuint>(oldProgram));
        return true;
    }
    bool InitializePixelWobble(std::string& error) {
        constexpr const char* source = R"(
            #version 120
            uniform sampler2D diffuse;
            uniform float effectTime, speedMod, intensity, spriteAlpha;
            uniform vec2 textureSpan;
            void main() {
                vec2 uv = gl_TexCoord[0].xy;
                uv.y += sin(uv.x * (effectTime * speedMod)) * intensity;
                uv.x += sin(uv.y * (effectTime * speedMod)) * intensity;
                vec4 sampled = texture2D(diffuse, vec2(uv.x, 1.0-uv.y) * textureSpan);
                gl_FragColor = vec4(sampled.rgb, sampled.a * spriteAlpha);
            }
        )";
        GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            char log[1024]{};
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            error = "PixelWobble.fx GPU shader compile failed: " + std::string(log);
            glDeleteShader(shader);
            return false;
        }
        pixelWobbleProgram_ = glCreateProgram();
        glAttachShader(pixelWobbleProgram_, shader);
        glLinkProgram(pixelWobbleProgram_);
        glDeleteShader(shader);
        GLint linked = GL_FALSE;
        glGetProgramiv(pixelWobbleProgram_, GL_LINK_STATUS, &linked);
        if (!linked) {
            char log[1024]{};
            glGetProgramInfoLog(pixelWobbleProgram_, sizeof(log), nullptr, log);
            error = "PixelWobble.fx GPU shader link failed: " + std::string(log);
            return false;
        }
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(pixelWobbleProgram_);
        glUniform1i(glGetUniformLocation(pixelWobbleProgram_, "diffuse"), 0);
        glUseProgram(static_cast<GLuint>(oldProgram));
        return true;
    }
    bool InitializeSepia(std::string& error) {
        // Direct translation of Game/ShaderFiles/Sepia.fx::PS. step(edge,x)
        // returns one at equality; the source's two gates both select there.
        constexpr const char* source = R"(
            #version 120
            uniform sampler2D scene;
            void main() {
                vec4 color = texture2D(scene, gl_TexCoord[0].xy);
                vec3 sepia = vec3(
                    dot(color.rgb, vec3(0.293, 0.669, 0.089)),
                    dot(color.rgb, vec3(0.249, 0.586, 0.068)),
                    dot(color.rgb, vec3(0.172, 0.434, 0.031))) * 0.75;
                float redExcess = color.r - (color.g + color.b);
                float sepiaTrue = step(redExcess, 0.5);
                float redTrue = step(0.5, redExcess);
                gl_FragColor = vec4(sepiaTrue * sepia +
                    redTrue * color.rgb * vec3(0.6, 1.0, 1.0), 1.0);
            }
        )";
        GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            char log[1024]{};
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            error = "Sepia.fx GPU shader compile failed: " + std::string(log);
            glDeleteShader(shader);
            return false;
        }
        sepiaProgram_ = glCreateProgram();
        glAttachShader(sepiaProgram_, shader);
        glLinkProgram(sepiaProgram_);
        glDeleteShader(shader);
        GLint linked = GL_FALSE;
        glGetProgramiv(sepiaProgram_, GL_LINK_STATUS, &linked);
        if (!linked) {
            char log[1024]{};
            glGetProgramInfoLog(sepiaProgram_, sizeof(log), nullptr, log);
            error = "Sepia.fx GPU shader link failed: " + std::string(log);
            return false;
        }
        glGenTextures(1, &sepiaTexture_);
        glBindTexture(GL_TEXTURE_2D, sepiaTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(sepiaProgram_);
        glUniform1i(glGetUniformLocation(sepiaProgram_, "scene"), 0);
        glUseProgram(static_cast<GLuint>(oldProgram));
        std::cerr << "Sepia.fx GPU postprocess initialized (scene before HUD)\n";
        return true;
    }
    bool testAllowSoftware_ = false;
    bool sceneSamplingPhase_ = false;
    float worldZoom_ = 1.f;
    size_t drawCalls_ = 0;
    double copySeconds_ = 0, presentSeconds_ = 0;
    bool initialized_ = false;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    GLuint sepiaProgram_ = 0, sepiaTexture_ = 0;
    GLuint pixelWobbleProgram_ = 0;
    GLuint damageBurstProgram_ = 0;
    int sepiaWidth_ = 0, sepiaHeight_ = 0;
    SDL_GameController* gamepad_ = nullptr;
    TextureId nextId_ = 1;
    std::unordered_map<TextureId, SDL_Texture*> textures_;
    std::unordered_set<TextureId> parallaxLogged_;
    std::unordered_map<std::string, TextureId> paths_;
    std::unordered_map<TextureId, std::string> texturePaths_;
    std::unordered_map<TextureId, WobbleMip> wobbleMipTextures_;
};
}
std::unique_ptr<Renderer> CreateRenderer(bool testAllowSoftware) { return std::make_unique<SDLRenderer>(testAllowSoftware); }
} // namespace deathleak::render
