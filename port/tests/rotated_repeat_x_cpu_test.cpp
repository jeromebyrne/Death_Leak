#include "../linux/RotatedRepeatX.h"
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
using deathleak::render::RotatedRepeatXQuad;
using deathleak::render::MakeRotatedRepeatXQuad;

struct Pixel { int r,g,b,a; };
static bool near(float a, float b) { return std::abs(a-b) < .0003f; }
static float frac(float x) { return x-std::floor(x); }
static Pixel pattern(int col, int row) {
    return {11+col*27, 17+row*35, 31+((col+row*3)%7)*23,
            col==2 && row==1 ? 128 : 255};
}
static Pixel over(Pixel src, Pixel dst, float modulation=1.f) {
    const float a=src.a/255.f*modulation;
    return {int(std::lround(src.r*a+dst.r*(1-a))),
            int(std::lround(src.g*a+dst.g*(1-a))),
            int(std::lround(src.b*a+dst.b*(1-a))),255};
}
// Independent inverse-transform oracle. PNG upload is bottom-first: port's
// flipY=true means source top at screen top.
static bool reference(float px,float py,float x,float y,float w,float h,
                      float degrees,float span,bool fx,bool fy,float& u,float& v) {
    const float a=degrees*3.14159265358979323846f/180.f;
    const float dx=px-(x+w/2),dy=py-(y+h/2);
    const float lx=std::cos(a)*dx+std::sin(a)*dy;
    const float ly=-std::sin(a)*dx+std::cos(a)*dy;
    if(std::abs(lx)>=w/2 || std::abs(ly)>=h/2) return false;
    const float q=lx/w+.5f,t=ly/h+.5f;
    u=span*(fx ? 1-q:q); v=fy?t:1-t;
    return true;
}
// Decode the implementation's four submitted vertices as two axes; this
// intentionally does not use the oracle's inverse-rotation formula.
static bool submitted(float px,float py,const RotatedRepeatXQuad& q,float& u,float& v) {
    const auto& a=q.vertices[0]; const auto& b=q.vertices[1];
    const auto& d=q.vertices[3];
    const float ex=b.x-a.x,ey=b.y-a.y,fx=d.x-a.x,fy=d.y-a.y;
    const float det=ex*fy-ey*fx;
    const float dx=px-a.x,dy=py-a.y;
    const float s=(dx*fy-dy*fx)/det,t=(ex*dy-ey*dx)/det;
    if(s<=0 || s>=1 || t<=0 || t>=1) return false;
    u=a.u+s*(b.u-a.u); v=a.v+t*(d.v-a.v);
    return true;
}
int main() {
    constexpr float degrees=5.153927f;
    constexpr float span=23.f/7.f;
    for(float angle : {degrees,-degrees}) for(bool fx : {false,true})
        for(bool fy : {false,true}) for(float x : {4.f,-5.f}) {
            RotatedRepeatXQuad q{};
            // Authored width 23 world units (base screen width is 2/3 of that); projected geometry may zoom.
            const float zoom=x<0 ? 1.4f:1.f;
            const float w=23*zoom,h=12*zoom;
            assert(MakeRotatedRepeatXQuad(x,5,w,h,23*(2.f/3.f),7,angle,fx,fy,q));
            assert(near(q.span,span));
            assert(near(q.vertices[0].u,fx?span:0));
            assert(near(q.vertices[1].u,fx?0:span));
            assert(near(q.vertices[0].v,fy?0:1));
            for(int yy=0;yy<24;++yy) for(int xx=0;xx<32;++xx) {
                float ru=0,rv=0,su=0,sv=0;
                const bool r=reference(xx+.5f,yy+.5f,x,5,w,h,angle,span,fx,fy,ru,rv);
                const bool s=submitted(xx+.5f,yy+.5f,q,su,sv);
                assert(r==s);
                if(!r) continue;
                assert(near(ru,su) && near(rv,sv));
                const int ci=int(std::floor(frac(ru)*7))%7;
                const int ri=int(std::floor(frac(rv)*5))%5;
                const Pixel prior{3,5,7,255};
                const Pixel base=over(pattern(ci,ri),prior,.5f);
                const Pixel later=over({200,10,20,128},base);
                assert(later.r!=prior.r || later.g!=prior.g);
            }
        }
    // ID17's fractional repeat is at left; ID18's is at right.
    float u=0,v=0;
    assert(reference(1,6,0,0,23,12,0,span,true,true,u,v));
    assert(u>3 && frac(u)>.1f && frac(u)<.3f);
    assert(reference(22,6,0,0,23,12,0,span,false,true,u,v));
    assert(u>3 && frac(u)>.1f && frac(u)<.3f);
    std::cout << "PASS rotated repeatX geometry, wrap phase, flips, crop/zoom, painter alpha\n";
}
