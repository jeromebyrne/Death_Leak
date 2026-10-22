// Confirms the SDL primitive used by SDLRenderer preserves D3D's local
// negative-X scale before rotation, even with the port's inverted Y UV.
#include <SDL.h>
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Window* window = SDL_CreateWindow("flip test", 0, 0, 200, 200, SDL_WINDOW_HIDDEN);
    assert(window);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    assert(renderer);
    SDL_Surface* src = SDL_CreateRGBSurfaceWithFormat(0, 2, 2, 32, SDL_PIXELFORMAT_RGBA32);
    assert(src);
    // Source top-left red, top-right green, bottom-left yellow, bottom-right blue.
    const uint32_t colors[4] = {0xff0000ff, 0xff00ff00, 0xffffff00, 0xffff0000};
    for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x)
        static_cast<uint32_t*>(src->pixels)[y * src->pitch / 4 + x] = colors[y * 2 + x];
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, src);
    assert(texture);
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    assert(SDL_RenderClear(renderer) == 0);
    const SDL_FRect destination{50, 50, 100, 100};
    assert(SDL_RenderCopyExF(renderer, texture, nullptr, &destination, 45, nullptr,
        static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL)) == 0);
    SDL_Surface* out = SDL_CreateRGBSurfaceWithFormat(0, 200, 200, 32, SDL_PIXELFORMAT_RGBA32);
    assert(out);
    assert(SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32, out->pixels, out->pitch) == 0);
    auto pixel = [&](int x, int y) { return static_cast<uint32_t*>(out->pixels)[y * out->pitch / 4 + x]; };
    assert(pixel(100, 135) == colors[0]); // flipped top-left then rotated to bottom
    assert(pixel(65, 100) == colors[1]);  // flipped top-right then rotated left
    assert(pixel(135, 100) == colors[2]); // flipped bottom-left then rotated right
    assert(pixel(100, 65) == colors[3]);  // flipped bottom-right then rotated top
    SDL_FreeSurface(out); SDL_DestroyTexture(texture); SDL_FreeSurface(src);
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
    std::cout << "PASS: SDL local horizontal/vertical flip precedes centered rotation\n";
}
