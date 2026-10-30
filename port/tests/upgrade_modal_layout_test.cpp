// Original XML widget rectangles and source text-color/bottom placement.
#include "../game/UpgradeModal.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
struct ModalSpy final:deathleak::render::Renderer {
 std::vector<deathleak::render::Sprite> sprites;
 struct Batch{deathleak::render::Color color;std::vector<deathleak::render::Rect> pixels;};std::vector<Batch> batches;
 bool Initialize(const char*,int,int,bool,std::string&)override{return true;}
 deathleak::render::TextureId LoadTexture(const std::string&,std::string&)override{return 0;}
 void BeginFrame(deathleak::render::Color)override{}void DrawSprite(const deathleak::render::Sprite&s)override{sprites.push_back(s);}
 void FillRect(deathleak::render::Rect,deathleak::render::Color)override{}
 void FillRects(const deathleak::render::Rect*r,size_t n,deathleak::render::Color c)override{batches.push_back({c,{r,r+n}});}
 void ApplyScenePostprocess()override{}void EndFrame()override{}bool PollQuit()override{return false;}
 bool KeyDown(int)const override{return false;}deathleak::render::MouseState Mouse()const override{return {};}
 bool SaveFrame(const std::string&,std::string&)override{return false;}
};
bool near(float a,float b){return std::abs(a-b)<.002f;}
int main(){
 using namespace deathleak;ModalSpy renderer;game::PaperText strings;assert(strings.Load("Game/XmlFiles/strings.xml"));
 const auto desc=strings.Lookup("dojo_scroll_desc_kRoll","en-gb");
 game::DrawUpgradeModal(renderer,desc,25,0,1,2,3,4,true);assert(renderer.sprites.size()==4);
 TiXmlDocument xml("Game/XmlFiles/UI/upgrade_modal.xml");assert(xml.LoadFile());
 for(const auto& pair:std::vector<std::pair<const char*,size_t>>{{"5_orb",1},{"4_upgrade_button_image_gamepad",2},{"3_back_button_image_keyboard",3}}){
  auto* object=xml.RootElement()->FirstChildElement("sprite");while(object && std::string(object->Attribute("name"))!=pair.first)object=object->NextSiblingElement("sprite");assert(object);
  double x=0,y=0,w=0,h=0;object->QueryDoubleAttribute("posX",&x);object->QueryDoubleAttribute("posY",&y);object->QueryDoubleAttribute("width",&w);object->QueryDoubleAttribute("height",&h);
  const auto expected=game::UpgradeWidget(x,y,w,h);const auto actual=renderer.sprites[pair.second].destination;
  assert(near(actual.x,expected.x)&&near(actual.y,expected.y)&&near(actual.width,expected.width)&&near(actual.height,expected.height));
 }
 auto within=[](const ModalSpy::Batch& batch,float left,float top,float right,float bottom){for(const auto&r:batch.pixels)assert(r.x>=left-.002f && r.y>=top-.002f && r.x+r.width<=right+.002f && r.y+r.height<=bottom+.002f);};
 assert(renderer.batches.size()==game::WrapPaperText(desc,38).size()+3);
 within(renderer.batches.front(),160.f*2/3,100.f*2/3,1760.f*2/3,200.f*2/3);
 assert(renderer.batches.front().color.r==153 && renderer.batches.front().color.g==153);
 for(const auto& batch:renderer.batches)if(batch.color.r==255 && batch.color.g==255 && batch.color.b==255)within(batch,260.f*2/3,-35.f*2/3,1660.f*2/3,965.f*2/3);
 within(renderer.batches[renderer.batches.size()-2],110.f*2/3,735.f*2/3,1760.f*2/3,835.f*2/3);
 int red=0;for(const auto& batch:renderer.batches)if(batch.color.r==255&&batch.color.g==0&&batch.color.b==0)++red;assert(red==2);
 const auto& price=renderer.batches[renderer.batches.size()-2];float bottom=0;for(const auto& r:price.pixels)bottom=std::max(bottom,r.y+r.height);assert(near(bottom,835.f*2.f/3.f));
 renderer.sprites.clear();renderer.batches.clear();game::DrawUpgradeModal(renderer,desc,25,25,1,2,3,4,false);assert(renderer.sprites.size()==3);
 for(const auto& batch:renderer.batches)assert(!(batch.color.r==255&&batch.color.g==0&&batch.color.b==0));
 std::cout<<"PASS: original upgrade XML orb/purchase/back rectangles, source header/description bounds, bottom-aligned cost, red insufficiency and dismiss warmup; no duplicate/overlapping balance row\n";
}
