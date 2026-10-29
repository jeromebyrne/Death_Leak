// Image-space regression for custom GL interop with SDL render targets.
// Requires strict hardware OpenGL; offscreen is diagnostic, x11 is display evidence.
#include "../render/Renderer.h"
#include "../linux/LogicalViewport.h"
#include <SDL.h>
#include <png.h>
#include <cassert>
#include <cstring>
#include <array>
#include <iostream>
#include <vector>
int main(int argc, char** argv) {
    using namespace deathleak::render;
    assert(argc == 4);
    const int width = std::stoi(argv[1]), height = std::stoi(argv[2]);
    const std::string prefix = argv[3];
    std::vector<unsigned char> pixels(180*120*4);
    constexpr std::array<std::array<unsigned char,4>,4> colors{{
        {{255,0,0,255}}, {{0,255,0,255}}, {{255,255,0,255}}, {{0,0,255,255}}}};
    for (int y=0;y<120;++y) for(int x=0;x<180;++x)
        for(int c=0;c<4;++c) pixels[(y*180+x)*4+c]=colors[(y>=60)*2+(x>=90)][c];
    png_image png{}; png.version=PNG_IMAGE_VERSION; png.width=180; png.height=120;
    png.format=PNG_FORMAT_RGBA;
    assert(png_image_write_to_file(&png,(prefix+"-source.png").c_str(),0,pixels.data(),0,nullptr));
    auto renderer = CreateRenderer(); std::string error;
    if (!renderer->Initialize("Death Leak target-orientation regression",width,height,false,error)) {
        std::cerr << error << '\n'; return 1;
    }
    const auto texture = renderer->LoadTexture(prefix+"-source.png",error); assert(texture);
    renderer->BeginFrame({0,0,0,255});
    for (int i=0;i<6;++i) {
        Sprite sprite{texture,{}, {float(50+i*200),70,120,80}};
        if(i==1) {sprite.pixelWobble.enabled=true; sprite.flipY=true;}
        if(i==2) sprite.rgbGain=1.001f;
        if(i==3) sprite.uvShiftX=1.f;
        if(i==4) {sprite.repeatX=true; sprite.rotationDegrees=.01f;}
        if(i==5) {sprite.foliageSway.enabled=true; sprite.flipY=true;}
        renderer->DrawSprite(sprite);
    }
    renderer->ResolveUnprocessedFrame();
    assert(renderer->SaveFrame(prefix+"-output.bmp",error));
    SDL_Surface* surface=SDL_LoadBMP((prefix+"-output.bmp").c_str()); assert(surface);
    const auto viewport=deathleak::linux_port::LogicalViewport::ForSize(width,height);
    auto at=[&](float x,float y) {
        const int px=viewport.x+int(x*viewport.width/1280.f),py=viewport.y+int(y*viewport.height/720.f);
        Uint32 value=0; std::memcpy(&value,static_cast<Uint8*>(surface->pixels)+py*surface->pitch+px*surface->format->BytesPerPixel,surface->format->BytesPerPixel);
        std::array<unsigned char,4> rgba{}; SDL_GetRGBA(value,surface->format,&rgba[0],&rgba[1],&rgba[2],&rgba[3]); return rgba;
    };
    for(int i=0;i<6;++i) {
        for(int corner=0;corner<4;++corner) {
            const auto actual=at(float(50+i*200+(corner%2?90:30)),float(corner>=2?130:90));
            if(actual!=colors[corner]) {
                std::cerr<<"family="<<i<<" corner="<<corner<<" got="<<int(actual[0])<<','<<int(actual[1])<<','<<int(actual[2])<<'\n';return 2;
            }
        }
        assert((at(float(80+i*200),610)==std::array<unsigned char,4>{0,0,0,255}));
    }
    SDL_FreeSurface(surface); renderer->EndFrame();
    std::cout << "PASS: ordinary/wobble/damage/parallax/rotated-repeat/foliage top/bottom, placement and letterbox "<<width<<'x'<<height<<'\n';
}
