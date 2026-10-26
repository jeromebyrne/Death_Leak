#include "../linux/LogicalViewport.h"
#include <SDL.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
int main(int argc, char** argv) {
    if (argc != 4) return 2;
    const int requestedWidth = std::atoi(argv[1]), requestedHeight = std::atoi(argv[2]);
    const int requestedHz = std::atoi(argv[3]);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) return 3;
    SDL_Window* window = SDL_CreateWindow("Death Leak input probe",SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,requestedWidth,requestedHeight,SDL_WINDOW_SHOWN);
    if (!window) return 4;
    const int display = SDL_GetWindowDisplayIndex(window);
    SDL_DisplayMode chosen{}; bool found = false;
    for (int i=0; i<SDL_GetNumDisplayModes(display); ++i) {
        SDL_DisplayMode mode{};
        if (!SDL_GetDisplayMode(display,i,&mode) && mode.w==requestedWidth &&
            mode.h==requestedHeight && mode.refresh_rate==requestedHz) {chosen=mode;found=true;break;}
    }
    if (!found || SDL_SetWindowDisplayMode(window,&chosen) ||
        SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN)) return 5;
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer || SDL_RenderSetLogicalSize(renderer,1280,720)) return 6;
    int outputWidth=0,outputHeight=0,windowWidth=0,windowHeight=0;
    SDL_DisplayMode active{};
    for (int i=0; i<40; ++i) {
        SDL_PumpEvents(); SDL_GetRendererOutputSize(renderer,&outputWidth,&outputHeight);
        SDL_GetWindowSize(window,&windowWidth,&windowHeight);
        SDL_GetCurrentDisplayMode(display,&active);
        if (outputWidth==requestedWidth && outputHeight==requestedHeight &&
            windowWidth==requestedWidth && windowHeight==requestedHeight &&
            active.refresh_rate==requestedHz) break;
        SDL_Delay(25);
    }
    if (outputWidth!=requestedWidth || outputHeight!=requestedHeight ||
        windowWidth!=requestedWidth || windowHeight!=requestedHeight ||
        active.w!=requestedWidth || active.h!=requestedHeight || active.refresh_rate!=requestedHz) return 7;
    SDL_RaiseWindow(window);
    SDL_Delay(100); SDL_PumpEvents();
    SDL_WarpMouseGlobal(requestedWidth/2,requestedHeight/2);
    SDL_Delay(150); SDL_PumpEvents();
    SDL_Event event{}; int motionCount=0, lastX=-1,lastY=-1;
    while (SDL_PollEvent(&event)) if (event.type==SDL_MOUSEMOTION) {
        ++motionCount; lastX=event.motion.x; lastY=event.motion.y;
    }
    int mouseX=0,mouseY=0; SDL_GetMouseState(&mouseX,&mouseY);
    std::fprintf(stderr,"motions=%d last=%d,%d\n",motionCount,lastX,lastY);
    int globalX=0,globalY=0,windowX=0,windowY=0;
    SDL_GetGlobalMouseState(&globalX,&globalY); SDL_GetWindowPosition(window,&windowX,&windowY);
    std::fprintf(stderr,"flags=%u window-pos=%d,%d global=%d,%d error=%s\n",SDL_GetWindowFlags(window),windowX,windowY,globalX,globalY,SDL_GetError());
    auto v=deathleak::linux_port::LogicalViewport::ForSize(outputWidth,outputHeight);
    auto p=v.WindowToLogical(float(mouseX),float(mouseY),windowWidth,windowHeight,
                             outputWidth,outputHeight);
    std::printf("mode=%dx%d@%d window=%dx%d output=%dx%d viewport=%d,%d,%d,%d mouse=%d,%d logical=%.3f,%.3f\n",
        active.w,active.h,active.refresh_rate,windowWidth,windowHeight,outputWidth,outputHeight,
        v.x,v.y,v.width,v.height,mouseX,mouseY,p.x,p.y);
    const bool good=std::fabs(mouseX-requestedWidth/2)<=1 &&
        std::fabs(mouseY-requestedHeight/2)<=1 &&
        std::fabs(p.x-640.f)<=1.f && std::fabs(p.y-360.f)<=1.f;
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();
    return good ? 0 : 8;
}
