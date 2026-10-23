#!/usr/bin/env python3
"""Real non-simulation loop, CPU/software-only body texture-command discriminator."""
import os
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LEVEL = 'Game/XmlFiles/levels/forest_exploration_1.xml'
SOURCES = ['port/linux/main.cpp',
           'port/tests/private_body_slow_frame_cpu_backend_20261023.cpp', 'port/game/LevelScene.cpp',
           'port/game/Combat.cpp', 'port/game/PlayerMotion.cpp',
           'port/game/Hud.cpp', 'Game/tinyxml/tinyxml.cpp',
           'Game/tinyxml/tinyxmlerror.cpp', 'Game/tinyxml/tinyxmlparser.cpp',
           'Game/tinyxml/tinystr.cpp']
ROW = re.compile(r'^BODY_COMMAND ordinal=(\d+) raw=([^ ]+) modified=([^ ]+) '
                 r'prior=([^ ]+) eligible=(\d+) grounded=(\d+) count=(\d+) path=(\S+)$')


def run(binary, delta):
    env = {**os.environ, 'SDL_VIDEODRIVER': 'dummy', 'SDL_RENDER_DRIVER': 'software',
           'SDL_AUDIODRIVER': 'dummy', 'DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER': '1',
           'DEATHLEAK_RENDER_ONLY_XML_IDS': '2'}
    output = subprocess.run([str(binary), '--play', LEVEL, '--script', '.:14',
                             '--fixed-dt', delta, '--frames', '14'], cwd=ROOT,
                            env=env, capture_output=True, text=True, check=True)
    rows = [ROW.fullmatch(line) for line in output.stdout.splitlines()
            if line.startswith('BODY_COMMAND ')]
    assert len(rows) == 14 and all(rows), output.stdout + output.stderr
    return [(int(m[1]), float(m[2]), float(m[3]), float(m[4]),
             int(m[5]), int(m[6]), int(m[7]), m[8]) for m in rows]


with tempfile.TemporaryDirectory(prefix='deathleak-body-command-') as temp:
    binary = Path(temp) / 'deathleak-body-command'
    cmd = ['g++', '-O0', '-std=c++17', '-DDEATHLEAK_TEST_BODY_SLOW_FRAME_COMMAND',
           '-Iport/compat', *SOURCES, '-o', str(binary)]
    subprocess.run(cmd, cwd=ROOT, check=True)
    slow = run(binary, '0.05')
    target = run(binary, '0.016666666666666666')

assert [row[0] for row in slow] == list(range(14)), slow
assert [row[0] for row in target] == list(range(14)), target
assert all(abs(row[1] - .05) < 1e-7 for row in slow), slow
assert all(abs(row[1] - 1/60) < 1e-7 for row in target), target
expected_prior = [0] + [1/60] * 13
for seq in (slow, target):
    for index, row in enumerate(seq):
        assert abs(row[2] - 1/60) < 1e-7, row
        assert abs(row[3] - expected_prior[index]) < 1e-7, row
        assert row[4] == 1, row
assert [(r[2], r[3], r[7]) for r in slow] == [(r[2], r[3], r[7]) for r in target], (slow, target)
# Spawn-to-ground settling is a real production prelude; only 11..13 are
# assessed as consecutive grounded Still body draws. Their second is _000.
for seq in (slow, target):
    for row in seq[11:14]:
        assert row[5] == 1 and row[6] == 1, row
assert slow[12][7].endswith('/idle_default_000.png'), slow[11:14]
print('PASS: three consecutive grounded production ordinals; raw 50 ms caps to 1/60 and body commands match 60 Hz control')
for row in slow[11:14]:
    print('ordinal=%d raw=%.9f modified=%.9f prior=%.9f eligible=%d grounded=%d body_commands=%d path=%s' % row)
