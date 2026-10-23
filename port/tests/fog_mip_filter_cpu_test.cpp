// CPU-only contract: no SDL_Init, GL context, window, Xorg, or GPU calls.
#include "../linux/SDLRenderer.cpp"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>

namespace {
using Pixel = std::array<float, 4>;
struct Image { int w, h; std::vector<Pixel> p; };
Pixel mix(Pixel a, Pixel b, float t) {
    Pixel out{};
    for (int c=0; c<4; ++c) out[c]=a[c]*(1-t)+b[c]*t;
    return out;
}
Image reduce(const Image& in) {
    Image out{std::max(1,in.w/2), std::max(1,in.h/2), {}};
    out.p.resize(out.w*out.h);
    for (int y=0;y<out.h;++y) for (int x=0;x<out.w;++x) {
        Pixel sum{}; int n=0;
        for (int dy=0;dy<2;++dy) for (int dx=0;dx<2;++dx) {
            const int sx=2*x+dx, sy=2*y+dy;
            if (sx>=in.w || sy>=in.h) continue;
            for (int c=0;c<4;++c) sum[c]+=in.p[sy*in.w+sx][c];
            ++n;
        }
        for (int c=0;c<4;++c) sum[c]/=n;
        out.p[y*out.w+x]=sum;
    }
    return out;
}
int wrap(int i,int size) { return (i%size+size)%size; }
Pixel bilinear(const Image& image,float u,float v) {
    const float x=u*image.w-.5f,y=v*image.h-.5f;
    const int ix=static_cast<int>(std::floor(x)),iy=static_cast<int>(std::floor(y));
    auto at=[&](int xx,int yy){return image.p[wrap(yy,image.h)*image.w+wrap(xx,image.w)];};
    return mix(mix(at(ix,iy),at(ix+1,iy),x-std::floor(x)),
               mix(at(ix,iy+1),at(ix+1,iy+1),x-std::floor(x)),y-std::floor(y));
}
Pixel sample(const std::vector<Image>& levels,float u,float v,float destW,float destH) {
    const float lambda=std::log2(std::max(levels[0].w/destW,levels[0].h/destH));
    if (lambda<=0) {
        const auto& a=levels[0];
        return a.p[wrap(static_cast<int>(std::floor(u*a.w)),a.w)+
                   wrap(static_cast<int>(std::floor(v*a.h)),a.h)*a.w];
    }
    const float clamped=std::min(lambda,static_cast<float>(levels.size()-1));
    const int lo=static_cast<int>(std::floor(clamped));
    const int hi=std::min(lo+1,static_cast<int>(levels.size()-1));
    return mix(bilinear(levels[lo],u,v),bilinear(levels[hi],u,v),clamped-lo);
}
bool near(float a,float b,float eps=1e-5f) {return std::fabs(a-b)<eps;}

// Pure policy/interop ledger. This specifies branch behavior; it cannot
// attest that an SDL driver preserves the corresponding live GL state.
struct FakeDraw {
    std::map<int,int> cache;
    int uploads=0, attempts=0, sdlBinds=0, sdlUnbinds=0;
    int binding=17, activeUnit=3, program=9, matrixMode=4;
    bool draw(int id, bool generationWorks, bool fallbackBinds) {
        const int savedBinding=binding, savedUnit=activeUnit;
        const int savedProgram=program, savedMatrix=matrixMode;
        activeUnit=0;
        auto found=cache.find(id);
        if (found==cache.end()) {
            ++attempts;
            cache[id]=generationWorks ? 100+id : 0;
            if (generationWorks) ++uploads;
        }
        bool drawn=false;
        if (cache[id]) {
            binding=cache[id];
            drawn=true;
        } else {
            ++sdlBinds;
            if (fallbackBinds) { binding=200+id; drawn=true; ++sdlUnbinds; }
        }
        binding=savedBinding; activeUnit=savedUnit;
        program=savedProgram; matrixMode=savedMatrix;
        return drawn;
    }
    void contextRecreated() {cache.clear();}
    void replaced(int id) {cache.erase(id);}
};
} // namespace
int main() {
    // Exercise the production PNG-to-GL row conversion, with non-opaque alpha.
    const png_byte topFirst[] = {255,0,0,64, 0,255,0,128,
                                  0,0,255,192, 255,255,0,32};
    const auto bottomFirst=deathleak::render::FlipPngRows(topFirst,2,2);
    assert(bottomFirst.size()==sizeof(topFirst));
    assert(bottomFirst[0]==0 && bottomFirst[2]==255 && bottomFirst[3]==192);
    assert(bottomFirst[8]==255 && bottomFirst[11]==64);
    // GL V=1-UV.y: authored top V=0 reads the final (top PNG) GL row.
    assert(bottomFirst[(2-1)*2*4]==topFirst[0]);

    Image pot{2,2,{{255,0,0,64},{0,255,0,128},{0,0,255,192},{255,255,0,32}}};
    const auto pot1=reduce(pot);
    assert(pot1.w==1 && pot1.h==1);
    assert(near(pot1.p[0][0],127.5f) && near(pot1.p[0][3],104.f));
    Image npot{3,5,{}};
    for(int y=0;y<5;++y) for(int x=0;x<3;++x)
        npot.p.push_back({float((x+y)%2*255),float(y*10),float(x*20),float(10+y*30)});
    std::vector<Image> chain{npot};
    while(chain.back().w>1 || chain.back().h>1) chain.push_back(reduce(chain.back()));
    assert(chain.size()==3 && chain[1].w==1 && chain[1].h==2 && chain[2].w==1 && chain[2].h==1);
    assert(near(chain[1].p[0][3],25.f)); // odd final row has explicit edge handling
    assert(near(chain[2].p[0][3],55.f));

    // Exact texel centers, fractional wrap, alpha and both quad flips.
    std::vector<Image> levels{pot,pot1};
    assert(near(bilinear(pot,.25f,.25f)[3],64.f));
    assert(near(bilinear(pot,1.25f,.25f)[3],64.f));
    assert(near(bilinear(pot,-.75f,.25f)[0],255.f));
    assert(near(bilinear(pot,.75f,.75f)[3],32.f));
    assert(near(bilinear(pot,1.f-.25f,.25f)[3],128.f)); // flip X
    assert(near(bilinear(pot,.25f,1.f-.25f)[3],192.f)); // flip Y
    const float fogLambda=std::log2(std::max(1988.f/1325.f,1290.f/860.f));
    assert(near(fogLambda,std::log2(1.5f),.002f));
    const float blend=std::log2(1.5f);
    auto minified=sample(levels,.25f,.25f,2.f/1.5f,2.f/1.5f);
    assert(near(minified[3],64.f*(1-blend)+104.f*blend));
    assert(!near(minified[3],bilinear(pot,.25f,.25f)[3]));
    assert(near(sample(levels,.25f,.25f,4.f,4.f)[3],64.f)); // nearest magnification
    assert(near(sample(levels,.49f,.25f,4.f,4.f)[3],64.f));
    assert(near(sample(levels,.51f,.25f,4.f,4.f)[3],128.f));
    FakeDraw ledger;
    assert(ledger.draw(7,true,true) && ledger.draw(7,false,true));
    assert(ledger.attempts==1 && ledger.uploads==1 && ledger.sdlBinds==0);
    assert(ledger.binding==17 && ledger.activeUnit==3 && ledger.program==9 && ledger.matrixMode==4);
    ledger.replaced(7);
    assert(ledger.draw(7,false,true) && ledger.attempts==2 && ledger.sdlBinds==1 && ledger.sdlUnbinds==1);
    assert(!ledger.draw(7,true,false)); // cached failure; failed SDL bind is not unbound
    assert(ledger.attempts==2 && ledger.sdlBinds==2 && ledger.sdlUnbinds==1);
    ledger.contextRecreated();
    assert(!ledger.draw(7,false,false));
    assert(ledger.attempts==3 && ledger.sdlBinds==3 && ledger.sdlUnbinds==1);
    assert(ledger.binding==17 && ledger.activeUnit==3 && ledger.program==9 && ledger.matrixMode==4);
    std::cout << "PASS: CPU fog mip dimensions, orientation, wrap, flips, alpha, LOD and magnification\n";
}
