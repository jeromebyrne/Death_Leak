#include "../render/Renderer.h"
#include <SDL.h>
#include <png.h>
#include <unordered_map>
#include <vector>

namespace deathleak::render {
namespace {
class SDLRenderer final : public Renderer {
public:
    ~SDLRenderer() override {
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
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        if (!renderer_) { error = SDL_GetError(); return false; }
        if (SDL_RenderSetLogicalSize(renderer_, 1280, 720) != 0) { error = SDL_GetError(); return false; }
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        for (int i = 0; i < SDL_NumJoysticks(); ++i)
            if (SDL_IsGameController(i)) { gamepad_ = SDL_GameControllerOpen(i); break; }
        return true;
    }

    TextureId LoadTexture(const std::string& path, std::string& error) override {
        auto cached = paths_.find(path);
        if (cached != paths_.end()) return cached->second;
        png_image image{};
        image.version = PNG_IMAGE_VERSION;
        if (!png_image_begin_read_from_file(&image, path.c_str())) {
            error = image.message; return InvalidTexture;
        }
        image.format = PNG_FORMAT_RGBA;
        std::vector<png_byte> pixels(PNG_IMAGE_SIZE(image));
        if (!png_image_finish_read(&image, nullptr, pixels.data(), 0, nullptr)) {
            error = image.message; png_image_free(&image); return InvalidTexture;
        }
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(),
            static_cast<int>(image.width), static_cast<int>(image.height), 32,
            static_cast<int>(image.width * 4), SDL_PIXELFORMAT_RGBA32);
        if (!surface) { error = SDL_GetError(); png_image_free(&image); return InvalidTexture; }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_FreeSurface(surface);
        png_image_free(&image);
        if (!texture) { error = SDL_GetError(); return InvalidTexture; }
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
    }
    void EndFrame() override { SDL_RenderPresent(renderer_); }
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
            case Escape: return keys[SDL_SCANCODE_ESCAPE] || button(SDL_CONTROLLER_BUTTON_START);
            default: return false;
        }
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
    bool initialized_ = false;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_GameController* gamepad_ = nullptr;
    TextureId nextId_ = 1;
    std::unordered_map<TextureId, SDL_Texture*> textures_;
    std::unordered_map<std::string, TextureId> paths_;
};
}
std::unique_ptr<Renderer> CreateRenderer() { return std::make_unique<SDLRenderer>(); }
} // namespace deathleak::render
