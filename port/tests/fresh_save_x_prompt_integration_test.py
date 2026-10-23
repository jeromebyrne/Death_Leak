#!/usr/bin/env python3
"""Focused CPU/source gate for cumulative prompt and post-player draw placement."""
from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
main = (root / 'port/linux/main.cpp').read_text()
scene = (root / 'port/game/LevelScene.cpp').read_text()
header = (root / 'port/game/LevelScene.h').read_text()
clock = re.search(r'struct IntroPromptClock \{.*?\n\};', main, re.S)
assert clock
source = '#include <cassert>\n' + clock.group() + '''
int main() {
    IntroPromptClock p;
    assert(!p.Visible(true, false));
    p.Advance(8.f); assert(!p.Visible(true, false));
    p.Advance(.001f); assert(p.Visible(true, false));
    assert(!p.Visible(true, true)); assert(p.Visible(true, false));
    assert(!p.Visible(false, false));
    p.Reset(); p.Advance(4.f); p.Advance(4.001f);
    assert(p.Visible(true, false));
    p.Reset(); assert(!p.Visible(true, false));
}
'''
with tempfile.TemporaryDirectory() as temp:
    path = Path(temp)
    (path / 'test.cpp').write_text(source)
    subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)

assert main.count('introPromptClock.Advance(dt);') == 1
assert 'steamDeck ? "Game/Media/UI/gamepad_icons/x_deck.png" : "Game/Media/UI/gamepad_icons/x.png"' in main
assert 'const float width = steamDeck ? 60.f : 150.f;' in main
assert 'const float height = steamDeck ? 60.f : 90.f;' in main
assert 'worldX - 35.f - drawCameraX - width / 2.f' in main
assert 'drawCameraY - worldY - 50.f - height / 2.f' in main
assert 'painterItems.push_back({2999,' not in main
assert 'std::move(afterPlayerDraw)' in main
assert 'if (object.player && afterPlayerDraw) afterPlayerDraw();' in scene
assert 'std::function<void()> afterPlayerDraw = {}' in header
body = scene[scene.index('for (const auto& object : sprites_)', scene.index('void LevelScene::Draw')):]
assert body.index('renderer.DrawSprite(draw);') < body.index('if (object.player && afterPlayerDraw) afterPlayerDraw();')
assert body.index('if (object.player && afterPlayerDraw) afterPlayerDraw();') < body.rindex('drawOrbTrails();')
assert main.index('scene.Draw(*renderer, drawCameraX') < main.index('renderer->ApplyScenePostprocess();')
print('fresh-save X prompt integration CPU/source gate PASS')
