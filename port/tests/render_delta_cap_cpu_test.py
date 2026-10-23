#!/usr/bin/env python3
"""CPU-only production-loop regression for the Windows 60 Hz update cap."""
import os
import re
import subprocess

BINARY = 'port/build/deathleak-linux'
LEVEL = 'Game/XmlFiles/levels/forest_exploration_1.xml'


def run(delta):
    result = subprocess.run(
        [BINARY, '--play', LEVEL, '--script', 'R:12', '--fixed-dt', delta,
         '--simulate-only'], check=True, capture_output=True, text=True,
        env={**os.environ, 'SDL_VIDEODRIVER': 'dummy',
             'SDL_RENDER_DRIVER': 'software',
             'DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER': '1'})
    summary = next(line for line in result.stdout.splitlines()
                   if line.startswith('Simulation ended at '))
    return re.sub(r' elapsed=[^ ]+', '', summary)


capped = run('0.05')
target = run('0.016666666666666666')
short = run('0.008')
assert capped == target, (capped, target)
assert short != target, (short, target)
assert re.search(r'player=\(([^,]+),', target).group(1) != re.search(
    r'player=\(([^,]+),', short).group(1), (short, target)
print('PASS: production simulation caps 50 ms update to 1/60; subtarget physics remains distinct')
