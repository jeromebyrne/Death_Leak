#include "../linux/UVWrap.h"
#include <SDL.h>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
using deathleak::render::ForEachUVWrapSegment;
using deathleak::render::UVWrapSegment;
int main() {
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Window* window = SDL_CreateWindow("UV wrap", 0, 0, 8, 8, SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_Surface* src = SDL_CreateRGBSurfaceWithFormat(0, 1, 8, 32, SDL_PIXELFORMAT_RGBA32);
    assert(renderer && src);
    const uint32_t colors[8] = {0xff0000ff,0xff3300ff,0xff6600ff,0xff9900ff,
                                0xffcc00ff,0xffff00ff,0xff00ffff,0xff00ccff};
    for (int y=0;y<8;++y) static_cast<uint32_t*>(src->pixels)[y*src->pitch/4]=colors[y];
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, src);
    assert(tex);
    SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(tex, 128);
    SDL_Surface* out = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA32);
    assert(out);
    auto pixel = [&](int y) { return static_cast<uint32_t*>(out->pixels)[y*out->pitch/4]; };
    for (bool flipped : {false,true}) for (float phase : {0.f,.25f,.75f,.999f,1.f,1.001f}) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        assert(SDL_RenderClear(renderer)==0);
        int copies=0;
        ForEachUVWrapSegment(8, phase, flipped, [&](UVWrapSegment seg) {
            assert(seg.length>0);
            SDL_Rect source{0,seg.source,1,seg.length};
            SDL_FRect dest{0,seg.offset*8.f,1,static_cast<float>(seg.length)};
            assert(SDL_RenderCopyExF(renderer, tex, &source, &dest, 0, nullptr, flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE)==0);
            ++copies;
        });
        assert(SDL_RenderReadPixels(renderer,nullptr,SDL_PIXELFORMAT_RGBA32,out->pixels,out->pitch)==0);
        const int cut=static_cast<int>((phase-std::floor(phase))*8);
        assert(copies==(cut ? 2 : 1));
        for (int y=0;y<8;++y) {
            int sourceY=flipped ? (cut-1-y+16)%8 : (cut+y)%8;
            uint32_t actual=pixel(y), expected=colors[sourceY];
            // 50% alpha on black: channel tolerance allows SDL blend rounding.
            for (int shift : {0,8,16})
                if (std::abs(int((actual>>shift)&255)-int(((expected>>shift)&255)/2))>1) { std::cerr << "flipped=" << flipped << " phase=" << phase << " y=" << y << " source=" << sourceY << " shift=" << shift << " actual=" << std::hex << actual << " expected=" << expected << std::dec << "\n"; std::abort(); }
        }
    }
    SDL_FreeSurface(out); SDL_DestroyTexture(tex); SDL_FreeSurface(src);
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
    std::cout << "PASS: whole-quad wrap direction, seam, integer boundary and tile alpha\n";
}
