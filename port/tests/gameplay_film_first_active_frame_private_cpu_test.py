#!/usr/bin/env python3
"""Compile the production film selector in isolation and exercise its clock boundary."""
from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'port/linux/main.cpp').read_text()
selector = source.split('// BEGIN_GAMEPLAY_FILM_FRAME_SELECTOR\n', 1)[1].split('// END_GAMEPLAY_FILM_FRAME_SELECTOR', 1)[0]
paths = re.findall(r'"(Game/Media/UI/film_frame(?:_[234])?\.png)"', source)
assert paths[:4] == [
    'Game/Media/UI/film_frame.png',
    'Game/Media/UI/film_frame_2.png',
    'Game/Media/UI/film_frame_3.png',
    'Game/Media/UI/film_frame_4.png',
], paths[:4]

# The gameplay branch must retain postprocess -> film -> bars -> meter order,
# full viewport, and the authored 0.2 opacity represented as byte alpha 51.
branch = source.split('renderer->ApplyScenePostprocess();', 1)[1].split('} else {\n                loadFilmFrames();', 1)[1].split('if (paperModal)', 1)[0]
assert branch.index('AdvanceGameplayFilmFrame(') < branch.index('filmFrames[gameplayFilmIndex]')
assert branch.index('filmFrames[gameplayFilmIndex]') < branch.index('if (finalFrame)') < branch.index('hud.Draw(')
assert 'renderer->DrawSprite({filmFrames[gameplayFilmIndex], {}, {0,0,1280,720}, {255,255,255,51}})' in branch
assert 'std::chrono::steady_clock::now().time_since_epoch()' in branch
assert 'std::uint64_t gameplayFilmLastSwitchMilliseconds = 0;' in source

cpp = '''#include <cstddef>
#include <cstdint>
#include <cassert>
#include <array>
''' + selector + '''
int main() {
    const std::array<const char*, 4> paths = {
        "Game/Media/UI/film_frame.png", "Game/Media/UI/film_frame_2.png",
        "Game/Media/UI/film_frame_3.png", "Game/Media/UI/film_frame_4.png"};
    std::uint64_t last = 0;
    size_t index = 0;
    AdvanceGameplayFilmFrame(90, last, index, paths.size());
    assert(index == 0 && last == 0);  // strict first gate
    AdvanceGameplayFilmFrame(91, last, index, paths.size());
    assert(index == 1 && last == 91);
    assert(std::string_view(paths[index]) == "Game/Media/UI/film_frame_2.png");
    last = 0; index = 0;
    AdvanceGameplayFilmFrame(91, last, index, paths.size());
    assert(index == 1 && last == 91); // first active update at 91 ms
    AdvanceGameplayFilmFrame(181, last, index, paths.size());
    assert(index == 1 && last == 91);  // strict next gate
    AdvanceGameplayFilmFrame(182, last, index, paths.size());
    assert(index == 2 && last == 182);
    last = 0; index = 0;
    AdvanceGameplayFilmFrame(100000, last, index, paths.size());
    assert(index == 1 && last == 100000); // one step, no catch-up burst
    AdvanceGameplayFilmFrame(100090, last, index, paths.size());
    assert(index == 1 && last == 100000);
    AdvanceGameplayFilmFrame(100091, last, index, paths.size());
    assert(index == 2 && last == 100091);
    AdvanceGameplayFilmFrame(100182, last, index, paths.size());
    assert(index == 3);
    AdvanceGameplayFilmFrame(100273, last, index, paths.size());
    assert(index == 0); // four-frame wrap
}
'''
cpp = cpp.replace('#include <array>', '#include <array>\n#include <string_view>')
with tempfile.TemporaryDirectory(prefix='gameplay-film-cpu-') as tmp:
    fixture = Path(tmp) / 'fixture.cpp'
    binary = Path(tmp) / 'fixture'
    fixture.write_text(cpp)
    subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', str(fixture), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('PASS: gameplay film first-active-frame CPU selector, assets, draw order')
