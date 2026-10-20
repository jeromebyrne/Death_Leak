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
    # Truly unknown permanent fields fail closed; no replacement or partial output.
    unsupported = root / 'unsupported.xml'
    root_xml = ET.Element('save_data')
    ET.SubElement(root_xml, 'unknown_progress_flag', {'type':'int', 'value':'65'})
    import base64
    wrapper = ET.Element('a', {'z':bridge.cipher(base64.b64encode(ET.tostring(root_xml)).decode())})
    unsupported.write_bytes(ET.tostring(wrapper))
    target = root / 'bad.dl5'
    bad = run('import', unsupported, target, '--level', level, '--fresh-session', success=False)
    assert 'unknown_progress_flag' in bad.stderr and not target.exists()
    for field, kind, value in [('granary', 'bool', 'false'), ('gameplay_features_unlocked', 'vector', None)]:
        bad_root = ET.Element('save_data')
        node = ET.SubElement(bad_root, field, {'type': kind})
        if value is None:
            ET.SubElement(node, 'e_1', {'type': 'int', 'value': '99'})
        else:
            node.set('value', value)
        bad_wrapper = ET.Element('a', {'z': bridge.cipher(base64.b64encode(ET.tostring(bad_root)).decode())})
        unsupported.write_bytes(ET.tostring(bad_wrapper))
        run('import', unsupported, target, '--level', level, '--fresh-session', success=False)
        assert not target.exists()
    # The shipped save now imports, survives an actual Linux load/resave, and exports
    # every typed permanent value, including the source's ordered feature vector.
    shipped = bridge.windows_fields('Game/bargon.xml')
    run('import', 'Game/bargon.xml', target, '--level', level, '--fresh-session')
    assert sum(line.startswith('W1:') for line in target.read_text().splitlines()) == 1
    assert 'H:65' in target.read_text().splitlines()
    assert 'U:kSpeedIncrease' in target.read_text().splitlines()
    resumed = root / 'resumed.dl5'
    loaded = subprocess.run(['port/build/deathleak-linux', '--load-save', str(target), '--save-file', str(resumed),
                    '--frames', '1', '--fixed-dt', '0.0166667', '--simulate-only'],
                   capture_output=True, text=True, check=True)
    assert 'max_health=65 speed_upgrade=1' in loaded.stdout, loaded.stdout
    # The imported unlock changes actual traversal speed, not just a status flag.
    import re
    def movement(save):
        result = subprocess.run(['port/build/deathleak-linux', '--load-save', str(save),
                                 '--script', 'R:30', '--fixed-dt', '0.0166667', '--simulate-only'],
                                capture_output=True, text=True, check=True)
        return float(re.search(r'player=\(([-0-9.]+),', result.stdout).group(1))
    no_speed = root / 'no-speed.dl5'
    no_speed.write_text(target.read_text().replace('U:kSpeedIncrease\n', ''))
    assert movement(target) > movement(no_speed) + 15
    # Saved current HP above the old 25 cap must survive under a 65-HP cap.
    high_hp = root / 'high-hp.dl5'
    hp_lines = target.read_text().splitlines()
    hp_position = hp_lines[2].split()
    hp_position[2] = '50'
    hp_lines[2] = ' '.join(hp_position)
    high_hp.write_text('\n'.join(hp_lines) + '\n')
    hp_result = subprocess.run(['port/build/deathleak-linux', '--load-save', str(high_hp),
                                '--frames', '1', '--fixed-dt', '0.0166667', '--simulate-only'],
                               capture_output=True, text=True, check=True)
    assert 'health=50' in hp_result.stdout and 'max_health=65' in hp_result.stdout, hp_result.stdout
    assert sum(line.startswith('W1:') for line in resumed.read_text().splitlines()) == 1
    assert 'H:65' in resumed.read_text().splitlines()
    assert 'U:kSpeedIncrease' in resumed.read_text().splitlines()
    for unlock in ('kProjectileDamageIncrease', 'kProjectileSpeedIncrease'):
        assert 'U:' + unlock in target.read_text().splitlines()
        assert 'U:' + unlock in resumed.read_text().splitlines()
    def simulate_projectiles(lines, script, encounter=False):
        fresh = root / ('projectile-' + str(len(list(root.glob('projectile-*.dl5')))) + '.dl5')
        data = lines.copy()
        if encounter:
            data[1] = 'port/tests/fixtures/encounter_open.xml'
            data[2] = '0 107.44937 25 100'
        fresh.write_text('\n'.join(data) + '\n')
        result = subprocess.run(['port/build/deathleak-linux', '--load-save', str(fresh),
                                 '--script', script, '--fixed-dt', '0.0166667',
                                 '--simulate-only', '--combat-state'],
                                capture_output=True, text=True, check=True)
        return result.stdout
    upgraded = target.read_text().splitlines()
    ordinary = [line for line in upgraded if line not in ('U:kProjectileDamageIncrease', 'U:kProjectileSpeedIncrease')]
    fast = simulate_projectiles(upgraded, 'F:120')
    slow = simulate_projectiles(ordinary, 'F:120')
    assert 'fired=12' in fast and 'fired=8' in slow, (fast, slow)
    fast_fall = simulate_projectiles(upgraded, 'F:1,.:5')
    slow_fall = simulate_projectiles(ordinary, 'F:1,.:5')
    assert float(re.search(r'Shot player .* vy=([-0-9.]+)', fast_fall).group(1)) > float(re.search(r'Shot player .* vy=([-0-9.]+)', slow_fall).group(1)), (fast_fall, slow_fall)
    damage_only = [line for line in upgraded if line != 'U:kProjectileSpeedIncrease']
    upgraded_hit = simulate_projectiles(damage_only, 'F:110', encounter=True)
    ordinary_hit = simulate_projectiles(ordinary, 'F:110', encounter=True)
    assert 'hits=5 kills=1' in upgraded_hit and 'kills=0' in ordinary_hit, (upgraded_hit, ordinary_hit)
    restored = root / 'restored.xml'
    run('export', resumed, restored, '--discard-runtime')
    # Windows defaults absent completion count to zero; export writes it explicitly.
    assert bridge.windows_fields(restored) == (shipped | {'num_times_game_completed': ('int', '0')})
    conflict = root / 'conflict.dl5'
    conflict.write_text(resumed.read_text().replace('H:65', 'H:25'))
    run('export', conflict, root / 'conflict.xml', '--discard-runtime', success=False)
    assert not (root / 'conflict.xml').exists()
    conflict.write_text(resumed.read_text().replace('U:kSpeedIncrease\n', ''))
    run('export', conflict, root / 'conflict.xml', '--discard-runtime', success=False)
    for unlock in ('kProjectileDamageIncrease', 'kProjectileSpeedIncrease'):
        conflict.write_text(resumed.read_text().replace('U:' + unlock + '\n', ''))
        run('export', conflict, root / 'conflict.xml', '--discard-runtime', success=False)
    damaged = root / 'damaged.dl5'
    damaged.write_text(resumed.read_text().replace('W1:', 'W1:!'))
    run('export', damaged, root / 'damaged.xml', '--discard-runtime', success=False)
    assert not (root / 'damaged.xml').exists()
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
