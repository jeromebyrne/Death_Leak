# Forest modal visual-clock fix — 2026-10-22

## Scope and source contract

Windows paper/upgrade modals set `Game::mPaused` (`Game/src/Game.cpp:308-333`); `GameObjectManager::Update` skips object/weather `Update` while paused (`Game/src/GameObjects/GameObjectManager.cpp:205-265`). Consequently the authored forest fog ScrollingSprites 119/122 must retain their scroll phase, XY, alpha and tint while either Linux modal is open. In contrast, Windows `ParallaxLayer::Draw` advances auto-scroll UVs during pause (`Game/src/GameObjects/ParallaxLayer.cpp:37-62,114-155`), and draw shader time remains live.

`LevelScene::UpdateVisuals` now advances only parallax auto-scroll before an `objectsPaused` early return; particle sprays/trails, ScrollingSprite, occlusion fade and animation updates stay frozen. Linux passes the **post-input current** `paperModal || upgradeModal` state and also skips combat/player visual updates in that state. Opening a modal in the current frame therefore freezes object clocks; dismissing it permits exactly that frame's normal tick, without accumulating paused time. Weather/rain already had a corresponding modal guard. Shader clock is unchanged.

## Focused verification

- The existing `port/tests/scrolling_sprite_test.cpp` compiled and passed: normal 119/122 motion/fade, update zone, cycle endpoint.
- A focused source harness loaded real `forest_exploration_1.xml`, drew IDs 119/122, stepped 120 paused 1/60-second frames in-zone, and asserted unchanged destination XY, opacity and tint alpha for both. One unpaused tick advanced XY and fade by one tick (no catch-up). The same harness, in a separate process because XML-ID filtering is cached, loaded `waterfall_top.xml` ID6 and asserted its authored speed-7 auto-scroll UV advanced 0.56 over 120 paused frames. Both cases passed.
- `make -C port -j2` on the sandbox host could not build because SDL2/libpng/GL development pkg-config files are absent. The established `deathleak-linux-build:bookworm` Docker image successfully built `make -C port -j2` (exit 0; only existing TinyXML warnings).
- No physical-GPU capture or Windows frame comparison was run here. This is a source/CPU regression gate, not visual-parity acceptance.

Reproduce the focused harness from this report in the repository root:

```sh
sed -n '/^```cpp$/,/^```$/p' port/FOREST_MODAL_VISUAL_CLOCK_FIX_2026-10-22.md | sed '1d;$d' > /tmp/deathleak-forest-modal-clock-test.cpp
g++ -std=c++17 -O0 -Iport/compat /tmp/deathleak-forest-modal-clock-test.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/deathleak-forest-modal-clock-test
/tmp/deathleak-forest-modal-clock-test
/tmp/deathleak-forest-modal-clock-test waterfall
```

```cpp
#include "/srv/openclaw/workspace/death_leak_analysis/source/port/game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace deathleak;
class CaptureRenderer final : public render::Renderer {
public:
 std::vector<render::Sprite> drawn;
 bool Initialize(const char*,int,int,std::string&) override { return true; }
 render::TextureId LoadTexture(const std::string&,std::string&) override { return 1; }
 void BeginFrame(render::Color) override {}
 void DrawSprite(const render::Sprite& s) override { drawn.push_back(s); }
 void FillRect(render::Rect,render::Color) override {}
 void ApplyScenePostprocess() override {}
 void EndFrame() override {}
 bool PollQuit() override { return false; }
 bool KeyDown(int) const override { return false; }
 render::MouseState Mouse() const override { return {}; }
 bool SaveFrame(const std::string&,std::string&) override { return false; }
};
int main(int argc, char** argv) {
 const bool waterfallOnly = argc > 1;
 setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", waterfallOnly ? "6" : "119,122", 1);
 CaptureRenderer renderer;
 std::string error;
 if (!waterfallOnly) {
 game::LevelScene forest;
 assert(forest.Load("Game/XmlFiles/levels/forest_exploration_1.xml",renderer,error));
 auto fog = [&] { renderer.drawn.clear(); forest.Draw(renderer,0,3400,0,3400); assert(renderer.drawn.size()==2); return renderer.drawn; };
 const auto before=fog();
 for(int i=0;i<120;++i) forest.UpdateVisuals(1.f/60.f,0,3400,0,3400,true);
 const auto frozen=fog();
 for(int i=0;i<2;++i) {
  assert(frozen[i].destination.x==before[i].destination.x);
  assert(frozen[i].destination.y==before[i].destination.y);
  assert(frozen[i].opacity==before[i].opacity);
  assert(frozen[i].tint.a==before[i].tint.a);
 }
 forest.UpdateVisuals(1.f/60.f,0,3400,0,3400,false);
 const auto resumed=fog();
 for(int i=0;i<2;++i) {
  assert(resumed[i].destination.x!=before[i].destination.x);
  assert(resumed[i].opacity<before[i].opacity);
  assert(resumed[i].opacity>0);
 }
 std::cout<<"PASS: modal freezes forest fog 119/122; resume has one tick\n";
 return 0;
 }
 game::LevelScene waterfall;
 assert(waterfall.Load("Game/XmlFiles/levels/waterfall_top.xml",renderer,error));
 renderer.drawn.clear(); waterfall.Draw(renderer,300,2600,300,2600);
 assert(renderer.drawn.size()==1);
 const float initial=renderer.drawn[0].uvShiftY;
 for(int i=0;i<120;++i) waterfall.UpdateVisuals(1.f/60.f,300,2600,300,2600,true);
 renderer.drawn.clear(); waterfall.Draw(renderer,300,2600,300,2600);
 assert(renderer.drawn.size()==1);
 assert(std::abs(renderer.drawn[0].uvShiftY - (initial+120.f*7.f*(2.f/3.f)*.001f))<.001f);
 std::cout<<"PASS: paused waterfall parallax continues\n";
}

```
