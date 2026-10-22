#include "../render/Renderer.h"
#include <SDL.h>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <png.h>
#include <unordered_map>
#include <vector>

namespace deathleak::render {
namespace {
class SDLRenderer final : public Renderer {
public:
    explicit SDLRenderer(bool testAllowSoftware) : testAllowSoftware_(testAllowSoftware) {}
    ~SDLRenderer() override {
        if (std::getenv("DEATHLEAK_RENDER_PROFILE"))
            std::cerr << "SDL profile calls=" << drawCalls_ << " copy=" << copySeconds_
                      << "s present=" << presentSeconds_ << "s\n";
        if (gamepad_) SDL_GameControllerClose(gamepad_);
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
        return id;
    }

    void BeginFrame(Color clear) override {
        SDL_SetRenderDrawColor(renderer_, clear.r, clear.g, clear.b, clear.a);
        SDL_RenderClear(renderer_);
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
        SDL_FRect destination{sprite.destination.x, sprite.destination.y,
                              sprite.destination.width, sprite.destination.height};
        SDL_SetTextureColorMod(texture, sprite.tint.r, sprite.tint.g, sprite.tint.b);
        SDL_SetTextureAlphaMod(texture, sprite.tint.a);
        SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(
            (sprite.flipX ? SDL_FLIP_HORIZONTAL : 0) | (sprite.flipY ? SDL_FLIP_VERTICAL : 0));
        if (sprite.rotationDegrees == 0 && flip == SDL_FLIP_NONE)
            SDL_RenderCopyF(renderer_, texture, sourcePtr, &destination);
        else
            SDL_RenderCopyExF(renderer_, texture, sourcePtr, &destination,
                              sprite.rotationDegrees, nullptr, flip);
        ++drawCalls_;
        copySeconds_ += std::chrono::duration<double>(std::chrono::steady_clock::now()-began).count();
    }
    void FillRect(Rect rect, Color color) override {
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_FRect box{rect.x, rect.y, rect.width, rect.height};
        SDL_RenderFillRectF(renderer_, &box);
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
    bool testAllowSoftware_ = false;
    size_t drawCalls_ = 0;
    double copySeconds_ = 0, presentSeconds_ = 0;
    bool initialized_ = false;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_GameController* gamepad_ = nullptr;
    TextureId nextId_ = 1;
    std::unordered_map<TextureId, SDL_Texture*> textures_;
    std::unordered_map<std::string, TextureId> paths_;
};
}
std::unique_ptr<Renderer> CreateRenderer(bool testAllowSoftware) { return std::make_unique<SDLRenderer>(testAllowSoftware); }
} // namespace deathleak::render
