#!/usr/bin/env python3
"""Deterministic gameplay consumer gate; not physical pointer-delivery evidence."""
import pathlib
import re
import subprocess
import tempfile

binary = 'port/build/deathleak-linux'
with tempfile.TemporaryDirectory(prefix='deathleak-mouse-') as directory:
    events = pathlib.Path(directory) / 'mouse.csv'

    def run(rows, *extra):
        events.write_text(rows)
        result = subprocess.run([
            binary, '--play', 'Game/XmlFiles/levels/forest_exploration_1.xml',
            '--mouse-script', str(events), '--frames', '30', '--fixed-dt', '0.0166667',
            '--simulate-only', '--combat-state', *extra,
        ], check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        endpoint = re.search(r'^Simulation ended at .* fired=(\d+) ', result.stdout, re.M)
        assert endpoint, result.stdout
        return int(endpoint[1]), result.stdout

    count, log = run('0,900,360,1\n')
    assert count == 3, log
    shots = re.findall(r'^Shot player .* vx=([-\d.]+)', log, re.M)
    assert shots and all(float(x) > 0 for x in shots), log
    count, log = run('0,0,360,1\n')
    assert count == 3, log
    shots = re.findall(r'^Shot player .* vx=([-\d.]+)', log, re.M)
    assert shots and all(float(x) < 0 for x in shots), log
    count, log = run('0,900,360,1\n1,900,360,0\n')
    assert count == 1, log
    for x, y in ((-1, 360), (1280, 360), (900, -1), (900, 720)):
        count, log = run(f'0,{x},{y},1\n')
        assert count == 0, log
    count, log = run('0,900,360,1\n', '--script', '.:30')
    assert count == 0, log  # Ordinary control scripts own input when supplied.
print('PASS: gameplay mouse left/right aim, held cadence, release, viewport rejection and scripted-input priority')
