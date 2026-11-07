# Private sea_2 final-film CPU preflight

Date: 2026-10-23. Scope: isolated CPU fixture plus read-only original/Linux source inspection. This is a proposed correction, **not** an integrated patch or original-vs-Linux pixel parity claim.

## Source contract and concrete mismatch

- Original `Game/XmlFiles/UI/final_scene_hud.xml:3-16`: four `film_frame*.png` textures, 11 FPS, alpha 0.25, 1920×1080 authored quad. `Boat.cpp:88-95` swaps `game_hud` for `final_scene_hud`. The screen/widget is retained rather than constructed at the boat transition (`UIManager.cpp:70-102`). The screen update traverses its widgets (`UIScreen.cpp:131-136`).
- `Game/src/UI/UIAnimatedSprite.cpp:4-7,20-39,61-77`: `(last,index)=(0,0)` at construction; `mFramesPerSecond` is float (`UIAnimatedSprite.h:11`). An active UI update samples integral `timeGetTime()` and advances **one** index iff `last + 1000/11 < now`, wraps modulo four, then sets `last=now`. In integral milliseconds this is `now-last >= 91` (assuming monotonic/no legacy 32-bit wrap). The update precedes render (`Game.cpp:254`, `program.cpp:168-175,287-317`). There is no elapsed-scene interpolation or catch-up loop.
- Current Linux `port/linux/main.cpp:1360-1365` uses `int(finalSceneTime*11.f)%4`. `finalSceneTime` is capped gameplay-delta accumulation (`:408,:645-655`) and must remain for the independent 15/35-second title/end gates. The ordinary `game_hud` already has a different monotonic selector (`:43-53,:424-426,:1373-1380`); reusing its mutable state would couple separate original widget instances.
- Current Linux postprocess, four loaded texture paths, 1280×720 logical viewport, alpha byte 64, top/bottom bar positions and flip, and film-before-bars/title draw order are already the intended branch shape. Do not change them in this clock patch. The original alpha 0.25 versus port byte 64 is existing quantization, not tested pixel parity here.

## Exact integration plan after main.cpp owner releases it

1. Beside `gameplayFilmIndex` and `gameplayFilmLastSwitchMilliseconds` near `main.cpp:424`, add **separate** `size_t finalFilmIndex = 0; std::uint64_t finalFilmLastSwitchMilliseconds = 0;`. Retain them across level transitions in this run, matching the retained original screen/widget; do not reset at `finalSceneTime=0`. If a new-game path reconstructs the entire UI instance in future, revisit that lifecycle explicitly. First entry is the acceptance target.
2. In the `sea_2` HUD branch, immediately before the film `DrawSprite`, sample `std::chrono::steady_clock::now()` in integral milliseconds and call `AdvanceGameplayFilmFrame(static_cast<std::uint64_t>(sample), finalFilmLastSwitchMilliseconds, finalFilmIndex, filmFrames.size());` using the existing helper's **strict** 90-ms integer comparison (`last+90 < now`, equivalent to 91-ms eligibility for integer samples). Select `filmFrames[finalFilmIndex]`. The call must happen once per active sea_2 HUD update/draw, after postprocess and before the film draw; the helper advances by one at most and records actual sampled uptime. For exact original update-before-render ordering, sample/update before the HUD draw; no other scene-time or renderer change is needed. Consider naming/extracting the generic helper only after the shared owner has reconciled its current diff.
3. Leave the `else` ordinary gameplay selector untouched, preserve the existing four asset paths, viewport, alpha, bars and title order. Retain `finalSceneTime` and title/end gates unchanged. Do not alter menu film, intro burst, or renderer.
4. On integration, add a focused clock-injected test at the selector boundary and a recording draw spy at the HUD branch. The fixture below specifies expected vectors. Then the separate GPU owner can capture controlled-index lossless sea_2 A/B on the physical renderer; a truly matched Windows frame is needed for original pixel parity.

**Caveat:** The existing helper uses `last+90 < now`; near unsigned 64-bit overflow this differs from a subtraction guard. The source's `timeGetTime()` itself is 32-bit and wraps. Neither startup nor normal run reaches 64-bit overflow, and this preflight asserts the practical monotonic range only. No claim about 32-bit wrap parity or repeated scene-entry parity is made.

## Isolated executable fixture and result

The following Python 3 fixture is self-contained; save it anywhere outside the repo (e.g. `/tmp/sea2_final_film_cpu_preflight_20261023.py`) and run `python3` on it. It models the proposed independent clock, statically checks the original/Linux source markers and asset paths, and uses a recording draw spy. It does **not** execute shared `main.cpp` or prove an integrated patch.

```python
from dataclasses import dataclass
from pathlib import Path

root = Path('/srv/openclaw/workspace/death_leak_analysis/source')
source = (root / 'port/linux/main.cpp').read_text()
original = (root / 'Game/src/UI/UIAnimatedSprite.cpp').read_text()
xml = (root / 'Game/XmlFiles/UI/final_scene_hud.xml').read_text()
assert 'mLastFrameSwitch + (1000/mFramesPerSecond)) < currentTime' in original
assert 'mCurrentFrame++' in original and 'mLastFrameSwitch = currentTime' in original
assert 'fps ="11"' in xml and 'alpha="0.25"' in xml
assert 'const int filmIndex = static_cast<int>(finalSceneTime * 11.f) % 4;' in source
assert source.index('renderer->ApplyScenePostprocess();') < source.index('if (currentLevel == "Game/XmlFiles/levels/sea_2.xml") {', source.index('renderer->ApplyScenePostprocess();'))
paths = tuple('Game/Media/UI/film_frame' + ('' if n == 1 else '_' + str(n)) + '.png' for n in range(1, 5))
for p in paths:
    assert p in source and p.replace('Game/', '').replace('/', '\\') in xml

@dataclass
class Clock:
    last: int = 0
    index: int = 0
    def update(self, now: int):
        # For integral millisecond samples, now > last + 1000/11 iff now-last >= 91.
        assert now >= self.last
        if now - self.last >= 91:
            self.index = (self.index + 1) % 4
            self.last = now

class Spy:
    def __init__(self): self.events = []
    def scene(self): self.events.append('scene')
    def postprocess(self): self.events.append('postprocess')
    def sprite(self, name, rect, alpha=255, flip_y=False): self.events.append((name,rect,alpha,flip_y))
    def title(self): self.events.append('title')

def draw(spy, clock, title):
    spy.scene(); spy.postprocess()
    spy.sprite(paths[clock.index], (0,0,1280,720), 64)
    spy.sprite('Game/Media/UI/frame_top.png', (0,0,1280,100))
    spy.sprite('Game/Media/UI/frame_top.png', (0,620,1280,100), flip_y=True)
    if title: spy.title()

final = Clock(); gameplay = Clock()
for now, expected_index, expected_last in [(90,0,0),(91,1,91),(181,1,91),(182,2,182),(273,3,273),(364,0,364)]:
    final.update(now)
    assert (final.index, final.last) == (expected_index, expected_last), (now, final)
    assert gameplay == Clock()
print('PASS boundary 90/91; last+90/91; 3->0 wrap; independent ordinary state')
late = Clock(); late.update(10000)
assert late == Clock(10000,1)
late.update(10000); assert late.index == 1
print('PASS one step per eligible update, no catch-up, no second step at same uptime')
final = Clock(); scene_time = 0.0; title = False; ended = False
final.update(91)
assert (final.index, scene_time, title, ended) == (1,0.0,False,False)
scene_time = 20.0; title = scene_time > 15.0
final.update(181)
assert (final.index, scene_time, title, ended) == (1,20.0,True,False)
print('PASS uptime advances film while scene time fixed; scene time advances without film jump')
spy = Spy(); draw(spy, final, title)
assert spy.events == ['scene','postprocess', (paths[1],(0,0,1280,720),64,False),
    ('Game/Media/UI/frame_top.png',(0,0,1280,100),255,False),
    ('Game/Media/UI/frame_top.png',(0,620,1280,100),255,True),'title']
print('PASS four exact asset paths, viewport/alpha, postprocess->film->bars->title spy contract')
print('NOTE fixture is isolated proposed logic; no shared source, project build, or GPU execution')
```

Observed invocation: `python3 /tmp/sea2_final_film_cpu_preflight_20261023.py` exited 0. It reported PASS for 90/91 ms first gate, `last+90/91`, one-step late update and same-time no-repeat, 3→0 wrap, ordinary HUD state separation, scene-time independence in both directions, exact four paths, viewport/alpha and `scene→postprocess→film→top/bottom bars→title` order. The draw spy verifies the proposed command sequence; static source checks verify current strings/order markers, not live renderer behavior. No project build, game, Docker, GPU, capture, source edit, shared status edit, commit or push was performed.

## Lane release

The only repository write is this uniquely named private report. Existing dirty work, including `main.cpp` and shared status files, was left untouched. **Private sea_2 final-film CPU-preflight lane explicitly released now.**
