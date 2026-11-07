#!/usr/bin/env python3
"""Private source-coupled CPU gate for the fresh-save X prompt; no renderer/GPU."""
from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
main = (root / 'port/linux/main.cpp').read_text()
clock = re.search(r'struct IntroPromptClock \{.*?\n\};', main, re.S)
assert clock, 'missing production prompt clock'
fixture = '''#include <cassert>\n''' + clock.group() + r'''
int main() {
    IntroPromptClock c;
    assert(!c.Visible(true, false));
    c.Advance(8.f);
    assert(!c.Visible(true, false)); // strict threshold
    c.Advance(0.001f);
    assert(c.Visible(true, false));
    assert(!c.Visible(true, true)); // hold hides without clock reset
    assert(c.Visible(true, false)); // release restores immediately
    assert(!c.Visible(false, false)); // Pull/completion/other phase
    c.Reset();
    c.Advance(4.f); // release/rehold does not restart total
    c.Advance(4.001f);
    assert(c.Visible(true, false));
    c.Reset();
    assert(!c.Visible(true, false));
}
'''
with tempfile.TemporaryDirectory(prefix='fresh-x-prompt-') as tmp:
    src = Path(tmp) / 'clock.cpp'
    exe = Path(tmp) / 'clock'
    src.write_text(fixture)
    subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', str(src), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)

assert main.count('introPromptClock.Advance(dt);') == 1
assert 'introPromptClock.Reset();' in main
assert 'steamDeck ? "Game/Media/UI/gamepad_icons/x_deck.png" : "Game/Media/UI/gamepad_icons/x.png"' in main
assert 'const float width = steamDeck ? 60.f : 150.f;' in main
assert 'const float height = steamDeck ? 60.f : 90.f;' in main
assert 'worldX - 35.f - drawCameraX - width / 2.f' in main
assert 'drawCameraY - worldY - 50.f - height / 2.f' in main
assert 'painterItems.push_back({2999,' in main
assert 'renderer->SetWorldZoom(!hasPulledSword ? introEffectiveZoom : 1.f);' in main
assert main.index('painterItems.push_back({2999,') < main.index('scene.Draw(*renderer, drawCameraX')
assert main.index('scene.Draw(*renderer, drawCameraX') < main.index('renderer->ApplyScenePostprocess();')
print('fresh-save X prompt CPU/source gate PASS')
