#pragma once
#include "PaperText.h"
#include <algorithm>
#include <string>
namespace deathleak::game {
// Original upgrade_modal.xml / UIUpgradeModal::Draw rectangles, projected from
// centered1920x1080 to1280x720. Font remains the explicit bitmap fallback;
// original Impact is a Windows system font, not a retained repository asset.
constexpr render::Rect UpgradeWidget(float x,float y,float width,float height) {
 return {(960.f+x)*2.f/3.f,(540.f-y-height)*2.f/3.f,width*2.f/3.f,height*2.f/3.f};
}
inline void DrawUpgradeModal(render::Renderer& renderer,const std::string& description,
 int cost,int balance,render::TextureId background,render::TextureId orb,
 render::TextureId purchase,render::TextureId back,bool ready) {
 if(background)renderer.DrawSprite({background,{}, {0,0,1280,720},{255,255,255,204},1.f,0.f,false,true});
 if(orb)renderer.DrawSprite({orb,{},UpgradeWidget(-170,-310,83.5f,127.5f)});
 if(purchase)renderer.DrawSprite({purchase,{},UpgradeWidget(60,-325,225,135)});
 if(back && ready)renderer.DrawSprite({back,{},UpgradeWidget(860,-500,64,64)});
 constexpr float scale=4.f,lineHeight=38.f;
 auto centered=[&](const std::string& text,float center,float top,render::Color color) {
  DrawBitmapText(renderer,text,center-static_cast<float>(text.size())*6.f*scale*.5f,top,scale,color);
 };
 const render::Color gray{153,153,153,255},white{255,255,255,255},red{255,0,0,255};
 centered("NEW TECHNIQUE",640.f,100.f*2.f/3.f,gray);
 // Source description bounds260..1660/-35..965: center(960,465),
 // width1400; choose fallback-font wrapping inside the exact projected width.
 const auto lines=WrapPaperText(description,38);
 float y=465.f*2.f/3.f-static_cast<float>(lines.size())*lineHeight*.5f;
 for(const auto& line:lines){centered(line,640.f,y,white);y+=lineHeight;}
 // Source cost is the number alone, bottom aligned to835, centered in110..1760.
 // The HUD already owns balance; no extra overlapping balance/title is invented.
 centered(std::to_string(cost),935.f*2.f/3.f,835.f*2.f/3.f-7.f*scale,balance>=cost?gray:red);
 if(balance<cost)centered("NOT ENOUGH ORBS",640.f,950.f*2.f/3.f-7.f*scale,red);
}
}
