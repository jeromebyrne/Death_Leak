# Waterfall Y-cadence: isolated CPU draw-spy (2026-10-23)

## Verdict

**Confirmed source/port cadence mismatch in emitted UV V, not an image-parity verdict.** The actual `LevelScene::Load`, `UpdateVisuals`, and `Draw` paths, with actual waterfall-top XML IDs 6 and 13, submitted a current-update phase. An independent recurrence from Windows `ParallaxLayer::Draw` and `program.cpp` submitted the *previous rendered iteration's modified delta* and starts at zero. The first Linux draw led by 0.004666667 cycle (ID6) and 0.007333334 cycle (ID13). On the half-delta third iteration the lead halved; this distinguishes cadence from a fixed-offset assumption. The only write in the source tree was this report. No source/test/status edit, GPU/Xorg run, physical capture, or commit.

The synthetic `1/120` is a **modified update delta**, not a claim that the Windows wall-clock loop renders on a sub-target tick: `program.cpp` waits for target delta but its time modifier can make the value stored in `LastUpdateDelta` smaller. The spy did not execute a Windows binary. Pixel magnitude, Windows binary parity, and a production fix remain unproven.

## Method and measured submissions

- Source chain: `Game/src/GameObjects/ParallaxLayer.cpp:114-150,223-231` advances Draw's V using prior `GetLastUpdateDelta()/GetTargetDelta()` and 720/1080 speed scale. `Game/src/program.cpp:115-117,149-177` renders before storing this iteration's delta. Linux `port/game/LevelScene.cpp:397-402,826-836,1267-1271` loads XML speed, advances phase in `UpdateVisuals` with current `dt`, and submits it as `uvShiftY` in `Draw`; `port/linux/main.cpp:1105,1187` calls update then ordinary scene draw.
- A no-window virtual `Renderer` returned nonzero texture handles and recorded `DrawSprite` submissions. It loaded the unmodified `Game/XmlFiles/levels/waterfall_top.xml`; diagnostic ID filter `6,13` yielded exactly two submitted sprites per draw in authored/painter order 6 then 13. Camera and player were held at the loaded scene values. Four calls used float `dt = [1/60,1/60,1/120,1/60]`; iteration 3 passed `objectsPaused=true` to verify modal continuity. Each update was followed by exactly one draw.
- Independent Windows recurrence in double precision: `W_i(n) = frac(W_i(n-1) + speed_i * (720/1080) * .001 * previousDelta/(1/60))`, initially `W_i(0)=0, previousDelta=0`, then store current modified delta **after** the draw. Linux values below are the actual float `render::Sprite::uvShiftY` captured by the spy; discrepancy is Linux minus recurrence. Values shown to nine decimal places.

| Render | dt | ID6 emitted | ID6 Windows recurrence | ID6 lead | ID13 emitted | ID13 Windows recurrence | ID13 lead |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 1/60 | 0.004666667 | 0.000000000 | 0.004666667 | 0.007333334 | 0.000000000 | 0.007333334 |
| 2 | 1/60 | 0.009333335 | 0.004666667 | 0.004666668 | 0.014666668 | 0.007333334 | 0.007333334 |
| 3 (paused objects) | 1/120 | 0.011666669 | 0.009333334 | 0.002333335 | 0.018333334 | 0.014666667 | 0.003666667 |
| 4 | 1/60 | 0.016333336 | 0.011666667 | 0.004666669 | 0.025666669 | 0.018333334 | 0.007333335 |

No phase wrapped during these four frames. Small last-digit differences are float accumulation versus double reference. The captured non-phase draw fields stayed constant across frames: ID6 `opacity=.5`, `flipX=1`, ordinary-path `flipY=1`, wobble disabled; ID13 `opacity≈.3`, `flipX=1`, custom-effect `flipY=0`, wobble enabled at intensity `.005` and speed-mod `10`; both `uvShiftX=0`. Two submissions retained painter slots 0/1. These observations establish that no other submitted field in this narrow spy explained the V delta; they do not measure shader output.

**Controls:** A temporary XML copy changed only ID6 `autoscrollyspeed` to zero and ID13 `autoscrolly` to false plus `cameraparallaxmultipliery=.25`; a draw at loaded `cameraY+100` kept both visible. ID6 emitted V `0` on all four draws. ID13 emitted `-775` on all four, exactly `-(loaded cameraY+100)*.25` (loaded cameraY was 3000); it did not accumulate dt. Alpha, flips, wobble parameters, and two painter slots remained as above. The recurrence columns printed by the primary harness are inapplicable to these altered control inputs.

## Reproduce without GPU or shared-tree writes

From `/srv/openclaw/workspace/death_leak_analysis/source`, create the disposable spy in `/tmp`. It compiles the existing scene implementation read-only with TinyXML; `--gc-sections` excludes unrelated backend code. `Spy::LoadTexture` supplies CPU handles only. The code below reproduces the measured submission path and prints the phase/control fields (the Windows reference table above follows the separately stated recurrence).

```sh
cat > /tmp/waterfall_y_spy.cpp <<'CPP'
#include "/srv/openclaw/workspace/death_leak_analysis/source/port/game/LevelScene.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>
using namespace deathleak;
struct Spy final : render::Renderer {
  std::vector<render::Sprite> draws; render::TextureId next=1;
  bool Initialize(const char*,int,int,std::string&) override {return true;}
  render::TextureId LoadTexture(const std::string&,std::string&) override {return next++;}
  void BeginFrame(render::Color) override {}
  void DrawSprite(const render::Sprite& s) override {draws.push_back(s);}
  void FillRect(render::Rect,render::Color) override {}
  void ApplyScenePostprocess() override {}
  void EndFrame() override {}
  bool PollQuit() override {return false;}
  bool KeyDown(int) const override {return false;}
  render::MouseState Mouse() const override {return {};}
  bool SaveFrame(const std::string&,std::string&) override {return false;}
};
int main(int argc,char** argv) {
  setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","6,13",1);
  Spy spy; game::LevelScene scene; std::string error;
  if(!scene.Load(argc>1?argv[1]:"Game/XmlFiles/levels/waterfall_top.xml",spy,error)) {
    std::cerr<<error<<'\n'; return 1;
  }
  float dt[]={1.f/60.f,1.f/60.f,1.f/120.f,1.f/60.f};
  std::cout<<std::fixed<<std::setprecision(9);
  for(int n=0;n<4;++n) {
    scene.UpdateVisuals(dt[n],0,0,scene.CameraX(),scene.CameraY(),n==2);
    spy.draws.clear();
    scene.Draw(spy,scene.CameraX(),argc>1?scene.CameraY()+100.f:scene.CameraY(),
               scene.PlayerX(),scene.PlayerY());
    if(spy.draws.size()!=2) {std::cerr<<"draw count "<<spy.draws.size()<<'\n';return 2;}
    for(int i=0;i<2;++i) {auto& s=spy.draws[i];
      std::cout<<"frame="<<n+1<<" id="<<(i?13:6)<<" dt="<<dt[n]
       <<" uvV="<<s.uvShiftY<<" alpha="<<s.opacity
       <<" flipX="<<s.flipX<<" flipY="<<s.flipY
       <<" wobble="<<s.pixelWobble.enabled
       <<" intensity="<<s.pixelWobble.intensity
       <<" speedMod="<<s.pixelWobble.speedMod
       <<" uvX="<<s.uvShiftX<<" painter="<<i<<'\n';
    }
  }
}
CPP
/usr/bin/g++ -std=c++17 -O0 -ffunction-sections -fdata-sections -Iport/compat \
  /tmp/waterfall_y_spy.cpp port/game/LevelScene.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp \
  -Wl,--gc-sections -o /tmp/waterfall_y_spy
/tmp/waterfall_y_spy
python3 - <<'PY'
import xml.etree.ElementTree as ET
root=ET.parse('Game/XmlFiles/levels/waterfall_top.xml')
for e in root.iter('ParallaxLayer'):
    if e.get('id')=='6': e.find('autoscrollyspeed').set('value','0')
    if e.get('id')=='13':
        e.find('autoscrolly').set('value','false')
        e.find('cameraparallaxmultipliery').set('value','0.25')
root.write('/tmp/waterfall_y_controls.xml')
PY
/tmp/waterfall_y_spy /tmp/waterfall_y_controls.xml
```

Read provenance at measurement: SHA-256 `port/game/LevelScene.cpp` `cda0ed58236418b27f417b815b749c393622b458fd12034ae85e76a303d684a0` (dirty for unrelated integration work), authored waterfall XML `d1e8a7a4986a54f0d5ba55329237adced9283cd3fc540e32604299967cde02b6`, original `ParallaxLayer.cpp` `eeacbc575fa4e50653c062bbe9e3fb904c6b2f47ce71349cf66227b9db51b073`, original `program.cpp` `a4dcf0efaccf14d95eb1638288a887090ca7a76f83eca74aac27aa3f6faab680`. Unrelated dirty files were left untouched; the concurrent forest source-audit lane was read-only.

## Boundary / handoff

The CPU gate is complete. A future scene/clock owner may decide whether to implement strict preceding-render-delta cadence, but this spy alone does not justify a production timing change or establish RGB severity. Such a change must account for one advance per actual render, first-render zero, prior **rendered modified** delta, and modal continuity; no speculative patch was made here. The earlier physical-GPU preflight varied pixels under a moving camera and cannot attribute the one-render UV lead. A frozen-camera, variable-delta rendered capture and a matched original Windows frame remain separate visual/parity gates.
