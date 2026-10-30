// Actual emitted pixel coverage for the source mixed-case technique descriptions.
#include "../game/PaperText.h"
#include "../game/TechniqueCatalog.h"
#include <cassert>
#include <cctype>
#include <iostream>
#include <vector>
struct TextSpy final:deathleak::render::Renderer {
 std::vector<deathleak::render::Rect> pixels;
 bool Initialize(const char*,int,int,bool,std::string&)override{return true;}
 deathleak::render::TextureId LoadTexture(const std::string&,std::string&)override{return 0;}
 void BeginFrame(deathleak::render::Color)override{} void DrawSprite(const deathleak::render::Sprite&)override{}
 void FillRect(deathleak::render::Rect r,deathleak::render::Color)override{pixels.push_back(r);}
 void FillRects(const deathleak::render::Rect* r,size_t n,deathleak::render::Color)override{pixels.insert(pixels.end(),r,r+n);}
 void ApplyScenePostprocess()override{} void EndFrame()override{}bool PollQuit()override{return false;}
 bool KeyDown(int)const override{return false;}deathleak::render::MouseState Mouse()const override{return {};}
 bool SaveFrame(const std::string&,std::string&)override{return false;}
};
int main(){
 TextSpy renderer;deathleak::game::PaperText strings;assert(strings.Load("Game/XmlFiles/strings.xml"));
 for(const auto& technique:deathleak::game::Techniques){
  auto original=strings.Lookup(std::string("dojo_scroll_desc_")+technique.id,"en-gb");assert(!original.empty());
  auto upper=original;for(char& c:upper)c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  deathleak::game::DrawBitmapText(renderer,original,140,280,2);auto mixed=renderer.pixels;renderer.pixels.clear();
  deathleak::game::DrawBitmapText(renderer,upper,140,280,2);assert(renderer.pixels.size()==mixed.size() && mixed.size()>200);
  for(size_t i=0;i<mixed.size();++i){const auto&a=mixed[i];const auto&b=renderer.pixels[i];assert(a.x==b.x && a.y==b.y && a.width==b.width && a.height==b.height);}
  renderer.pixels.clear();
 }
 std::cout<<"PASS: all8 source mixed-case technique descriptions emit every lowercase/uppercase glyph at identical positions, no blank-letter cells\n";
}
