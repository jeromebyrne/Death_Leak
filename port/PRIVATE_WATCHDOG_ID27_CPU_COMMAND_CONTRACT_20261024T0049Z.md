# Waterfall ID27 CPU command-contract comparator — 2026-10-24 00:49Z

## Result

**PASS for the bounded authored-XML / Linux physical-packet command contract.** ID2 player has depth `kPlayer=3000`; ID27 `tree2_blur` has `kFarForeground=1500`. Descending-depth order places the player first, and frame638 records arm/body ordinals 21/22 before ID27 ordinal 101. ID27's authored effect is `effectlighttexture`, alpha is 0.950000, horizontal flip is true, vertical flip false, fade is disabled, repetition is disabled, and all authored effect-displacement terms are zero. The Linux packet submits tint alpha 242, corresponding to 242/255 = 0.9490196078 versus authored float 0.95, a 0.0009803922 alpha difference. Its `flipY=true` is the known Y-axis convention conversion, not an authored vertical flip. The packet's projected 741.544922×1828.92273 destination magnifies the 669×1650 PNG by about 1.108438 in each axis, so the magnified ID27 copy selects nearest filtering under the current Linux scene-copy rule.

This establishes a **command-parameter precision difference**, not a demonstrated pixel difference or a Windows-vs-Linux matched-state comparison. The packet explicitly has `matchedStateEligible=false`; no Windows runtime draw state, bound resource, or pixel readback was acquired here. Nothing in this CPU gate warrants changing alpha, art, XML, or drawing order to make the tree less occluding.

## Inputs and source contracts

- Authored input: `Game/XmlFiles/levels/waterfall_top.xml`, ID2 and ID27 (`:35-57`, `:502-516`). Depth constants: `Game/src/GameObjects/GameObject.h:44,52`; Windows descending comparator: `Game/src/GameObjects/GameObjectManager.cpp:58-65`.
- Physical packet: `port/build/private-watchdog-waterfall-player-packet-20261024t0040z/frame638.json`, `dynamicDraw.orderedSceneDraws`. Texture header: `Game/Media/objects/tree2_blur.png`.
- Windows float submission: `Game/src/GameObjects/Sprite.cpp:626-669` (`SetAlpha(m_alpha)`); Linux byte tint / filter selection: `port/linux/SDLRenderer.cpp:206-240`. Authored Y=false versus packet Y=true follows the coordinate conversion documented in `port/game/LevelScene.cpp:1408-1429` and the preceding draw-contract audit. No display, GPU, Docker build, or runtime execution was used.

## Exact CPU command

Run from `/srv/openclaw/workspace/death_leak_analysis/source`:

```sh
python3 - <<'PY'
from pathlib import Path
import xml.etree.ElementTree as ET, json, struct, math
root=ET.parse('Game/XmlFiles/levels/waterfall_top.xml').getroot()
player=root.find(".//Player[@id='2']")
tree=root.find(".//Sprite[@id='27']")
a=json.loads(Path('port/build/private-watchdog-waterfall-player-packet-20261024t0040z/frame638.json').read_text())
draws=a['dynamicDraw']['orderedSceneDraws']
arm=next(d for d in draws if d['ordinal']==21)
body=next(d for d in draws if d['ordinal']==22)
t=next(d for d in draws if d['ordinal']==101)
assert player is not None and tree is not None
assert player.find('position').get('depth_layer')=='kPlayer'
assert tree.find('position').get('depth_layer')=='kFarForeground'
assert float(tree.find('alpha').get('value'))==.95
assert tree.find('effect').get('name')=='effectlighttexture'
assert tree.find('horizontalflip').get('value')=='true'
assert tree.find('verticalflip').get('value')=='false'
assert tree.find('fade_when_player_occluded').get('value')=='false'
assert tree.find('dimensions').get('repeatX')=='false' and tree.find('dimensions').get('repeatY')=='false'
assert all(float(tree.find('effect').get(k))==0 for k in ['noiseintensity','wobbleintensity','pixelwobbleintensity','pixelwobblespeedmod'])
assert tree.find('sine_wave_props').get('active')=='false'
assert all(float(v)==0 for k,v in tree.find('foliage_sway_props').attrib.items() if k!='time_multiplier')
assert arm['ordinal'] < body['ordinal'] < t['ordinal']
assert t['texturePath']=='Game/Media/objects/tree2_blur.png'
assert t['tint']==[255,255,255,round(.95*255)]
assert t['flipX'] and t['flipY'] and not t['repeatX']
assert not t['foliageSway'] and not t['pixelWobble'] and t['uvShift']==[0,0]
assert t['rotationDegrees']==0
assert math.isclose(t['opacity'],.95,abs_tol=2e-8)
png=Path('Game/Media/objects/tree2_blur.png').read_bytes()[:24]
assert png[:8]==b'\x89PNG\r\n\x1a\n'
w,h=struct.unpack('>II',png[16:24]); assert (w,h)==(669,1650)
assert t['destination'][2]>w and t['destination'][3]>h
print('PASS XML ID2 depth=kPlayer=3000; ID27 depth=kFarForeground=1500; descending 3000>1500')
print('PASS ID27 effect=effectlighttexture alpha=0.950000 Hflip=true Vflip=false fade=false repeatX/Y=false; effect noise/wobble=0; sine=false; foliage intensities=0')
print('PASS frame638 ordinals arm=21 body=22 tree=101; tree texture='+t['texturePath'])
print('PASS tree packet tint='+str(t['tint'])+' opacity='+str(t['opacity'])+' flipX/Y='+str((t['flipX'],t['flipY']))+' repeatX=false UVshift=[0,0] foliageSway=false pixelWobble=false rotation=0')
print(f'PASS source PNG={w}x{h}, destination={t["destination"][2]:.6f}x{t["destination"][3]:.5f}; scales={t["destination"][2]/w:.9f}x{t["destination"][3]/h:.9f}')
print(f'alpha byte={round(.95*255)}, normalized={round(.95*255)/255:.10f}, delta={.95-round(.95*255)/255:.10f}')
print('packet matchedStateEligible='+str(a['matchedStateEligible']))
PY
```

## Exact output

```text
PASS XML ID2 depth=kPlayer=3000; ID27 depth=kFarForeground=1500; descending 3000>1500
PASS ID27 effect=effectlighttexture alpha=0.950000 Hflip=true Vflip=false fade=false repeatX/Y=false; effect noise/wobble=0; sine=false; foliage intensities=0
PASS frame638 ordinals arm=21 body=22 tree=101; tree texture=Game/Media/objects/tree2_blur.png
PASS tree packet tint=[255, 255, 255, 242] opacity=0.949999988 flipX/Y=(True, True) repeatX=false UVshift=[0,0] foliageSway=false pixelWobble=false rotation=0
PASS source PNG=669x1650, destination=741.544922x1828.92273; scales=1.108437851x1.108438018
alpha byte=242, normalized=0.9490196078, delta=0.0009803922
packet matchedStateEligible=False
```

## Implication and boundary

The source-authored and physical Linux command evidence supports ordinary painter overdraw of the player by ID27, with no active per-player occlusion fade, repeat, or effect displacement. Float-versus-byte alpha quantization is real but too small to infer a visible pixel mismatch, especially without a matched Windows command/readback. The next parity discriminator remains the separate original-Windows draw-state and matched-pose pixel acquisition described in `port/PRIVATE_WATCHDOG_ID27_WINDOWS_LINUX_DRAW_CONTRACT_20261024T0048Z.md`; this report does not claim that result.
