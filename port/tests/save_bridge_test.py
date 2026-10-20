#!/usr/bin/env python3
"""Integration coverage for the explicit permanent-save subset and isolation."""
import hashlib
import importlib.util
import sys
sys.dont_write_bytecode = True
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET

spec = importlib.util.spec_from_file_location('save_bridge', 'port/tools/save_bridge.py')
bridge = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bridge)
level = 'Game/XmlFiles/levels/grass_exploration_3.xml'
with tempfile.TemporaryDirectory(prefix='save-bridge-') as root:
    root = Path(root)
    dl = root / 'source.dl5'
    dl.write_text('\n'.join(['DL5', level, '577.692871 -1242.459717 25 100', 'B:7',
                             'C:2', 'G:pt-br', 'K:granary', 'K:dojo', 'K:village_hut',
                             'K:has_repair_tools', 'D:barn_entrance', 'D:dojo',
                             'D:village_hut_entrance', 'U:kDeflection',
                             'Game/XmlFiles/levels/grass_exploration_3.xml#99']) + '\n')
    before = hashlib.sha256(dl.read_bytes()).digest()
    win = root / 'export.xml'
    imported = root / 'import.dl5'
    def run(*args, success=True):
        result = subprocess.run(['python3', 'port/tools/save_bridge.py', *map(str, args)],
                                capture_output=True, text=True)
        assert (result.returncode == 0) == success, result.stderr
        return result
    run('export', dl, win, success=False)  # explicit runtime-loss acknowledgement required
    assert not win.exists()
    run('export', dl, win, '--discard-runtime')
    fields = bridge.windows_fields(win)
    assert fields['num_times_game_completed'] == ('int', '2')
    assert fields['language_locale_key'] == ('string', 'pt-br')
    assert fields['gameplay_features_unlocked'] == ('vector', [('int', '4')])
    for key in bridge.KEYS: assert fields[key] == ('bool', 'true')
    for door in bridge.DOORS: assert fields[door + '_unlocked'] == ('bool', 'true')
    run('import', win, imported, '--level', level, success=False)
    assert not imported.exists()
    run('import', win, imported, '--level', level, '--fresh-session')
    lines = imported.read_text().splitlines()
    assert lines[:2] == ['DL5', level]
    assert lines[3] == 'B:0'
    assert 'C:2' in lines and 'G:pt-br' in lines and 'U:kDeflection' in lines
    assert not any('#99' in line or line == 'B:7' for line in lines)
    assert len([line for line in lines if line.startswith('K:')]) == 4
    assert len([line for line in lines if line.startswith('D:')]) == 3
    assert hashlib.sha256(dl.read_bytes()).digest() == before
    result = subprocess.run(['port/build/deathleak-linux', '--load-save', str(imported),
                             '--frames', '1', '--fixed-dt', '0.0166667', '--simulate-only', '--combat-state'],
                            capture_output=True, text=True, check=True)
    assert 'keys=4' in result.stdout and 'balance=0' in result.stdout, result.stdout
    assert 'Deflection unlocked=1' in result.stdout, result.stdout
    # Unsupported permanent fields fail closed; no replacement or partial output.
    unsupported = root / 'unsupported.xml'
    root_xml = ET.Element('save_data')
    ET.SubElement(root_xml, 'player_max_health', {'type':'int', 'value':'65'})
    import base64
    wrapper = ET.Element('a', {'z':bridge.cipher(base64.b64encode(ET.tostring(root_xml)).decode())})
    unsupported.write_bytes(ET.tostring(wrapper))
    target = root / 'bad.dl5'
    bad = run('import', unsupported, target, '--level', level, '--fresh-session', success=False)
    assert 'player_max_health' in bad.stderr and not target.exists()
    # Existing source sample contains several intentionally unsupported permanent fields.
    sample = run('import', 'Game/bargon.xml', target, '--level', level, '--fresh-session', success=False)
    assert 'unmapped feature' in sample.stderr or 'unmapped permanent' in sample.stderr
    assert not target.exists()
    run('import', win, imported, '--level', level, '--fresh-session', success=False)  # no overwrite
    completed = root / 'completed.dl6'
    completed.write_text('DL6\nMENU\nC:3\nG:de\n')
    completed_win = root / 'completed.xml'
    run('export', completed, completed_win, '--discard-runtime')
    assert bridge.windows_fields(completed_win) == {'num_times_game_completed': ('int', '3'), 'language_locale_key': ('string', 'de')}
    stale = root / 'stale.dl6'
    stale.write_text('DL6\nMENU\nC:3\nK:granary\n')
    run('export', stale, root / 'stale.xml', '--discard-runtime', success=False)
    assert not (root / 'stale.xml').exists()
print('PASS: save bridge permanent subset, active load, loss acknowledgement, rejection and isolation')
