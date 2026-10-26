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
presentation = re.search(r'struct IntroPromptPresentation \{.*?\n\};', main, re.S)
assert clock and presentation
source = '#include <cassert>\n' + clock.group() + '\n' + presentation.group() + '''
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
    IntroPromptPresentation packet;
    packet.Prepare(true, false, false, true, p); assert(!packet.Visible());
    p.Advance(8.001f);
    packet.Prepare(true, false, false, true, p); assert(packet.Visible());
    // Draw-side repeated reads cannot consume input/clock or mutate the packet.
    const IntroPromptPresentation& draw = packet;
    for (int i = 0; i < 100000; ++i) assert(draw.Visible());
    assert(p.Visible(true, false));
    packet.Prepare(true, true, false, true, p); assert(!packet.Visible());
    packet.Prepare(true, false, true, true, p); assert(!packet.Visible());
    packet.Prepare(true, false, false, false, p); assert(!packet.Visible());
    packet.Prepare(false, false, false, true, p); assert(!packet.Visible());
}
'''
with tempfile.TemporaryDirectory() as temp:
    path = Path(temp)
    (path / 'test.cpp').write_text(source)
    subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)

assert main.count('introPromptClock.Advance(dt);') == 1
assert main.index('introPromptPresentation.Prepare(') < main.index('if (fixedDt <= 0.f && !livePacer.PresentNow())')
assert 'if (introPromptPresentation.Visible())' in main
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
