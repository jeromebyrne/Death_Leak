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
    # Both shipped permanent smashable flags become active Linux state.
    shipped_smashes = {smash for smash in bridge.SMASHABLES
                       if shipped.get(bridge.smash_field(smash)) == ('bool', 'true')}
    assert shipped_smashes == bridge.SMASHABLES
    assert {'Z:' + smash for smash in shipped_smashes} <= set(target.read_text().splitlines())
    grass_smash = 'Game/XmlFiles/levels/grass_exploration_1.xml'
    smash_spawn = '-909.65625 -600 25 100'
    smash_start = root / 'smash-start.dl5'
    smash_start.write_text('DL5\n' + grass_smash + '\n' + smash_spawn + '\nB:0\nU:kDownwardDash\n')
    smash_run = 0
    def smash_trace(save, script, output=None):
        global smash_run
        smash_run += 1
        working = root / f'smash-run-{smash_run}.dl5'
        working.write_bytes(save.read_bytes())
        args = ['port/build/deathleak-linux', '--load-save', str(working), '--script', script,
                '--fixed-dt', '0.0166667', '--simulate-only']
        if output: args += ['--save-file', str(output)]
        return subprocess.run(args, capture_output=True, text=True, check=True).stdout
    normal = smash_trace(smash_start, '.:29')
    assert 'Smashed ' not in normal and 'grounded=1' in normal, normal
    early = smash_trace(smash_start, 'M:1,.:28')
    assert 'Smashed ' not in early, early  # air-time gate rejects immediate dash
    no_unlock = root / 'smash-no-unlock.dl5'
    no_unlock.write_text(smash_start.read_text().replace('U:kDownwardDash\n', ''))
    assert 'Smashed ' not in smash_trace(no_unlock, '.:20,M:1,.:8')
    smash_saved = root / 'smash-earned.dl5'
    dashed = smash_trace(smash_start, '.:20,M:1,.:8', smash_saved)
    assert 'Smashed ' + grass_smash + ' at frame 21' in dashed, dashed
    assert 'grounded=0' in dashed and 'focus=51.5' in dashed, dashed
    assert 'Z:' + grass_smash in smash_saved.read_text().splitlines()
    # Dynamic smash rewards have a source 0.01..0.109 s spawn lockout. They
    # credit only the balance when caught, never an authored persistent orb ID.
    delayed = smash_trace(smash_start, '.:20,M:1,.:2')
    assert 'spawned_orbs=3' in delayed and 'orbs=9' in delayed and 'pending_smash_orbs=3' in delayed and 'balance=9' in delayed, delayed
    assert 'orbs=9' in dashed and 'pending_smash_orbs=2' in dashed and 'balance=10' in dashed, dashed
    assert 'B:10' in smash_saved.read_text().splitlines()
    assert len([line for line in smash_saved.read_text().splitlines() if line.startswith(grass_smash + '#')]) == 9
    resumed_reward = smash_trace(smash_saved, '.:1')
    assert 'Smashed ' not in resumed_reward and 'pending_smash_orbs=0' in resumed_reward and 'balance=10' in resumed_reward, resumed_reward
    assert 'Smashed ' not in smash_trace(smash_saved, '.:20,M:1,.:8')
    smash_export = root / 'smash-earned.xml'
    run('export', smash_saved, smash_export, '--discard-runtime')
    assert bridge.windows_fields(smash_export)[bridge.smash_field(grass_smash)] == ('bool', 'true')
    imported_smash = root / 'smash-import.dl5'
    imported_lines = target.read_text().splitlines()
    imported_lines[1] = grass_smash; imported_lines[2] = smash_spawn
    imported_smash.write_text('\n'.join(imported_lines) + '\n')
    imported_pass = smash_trace(imported_smash, '.:29')
    assert 'Smashed ' not in imported_pass and 'grounded=0' in imported_pass, imported_pass
    # Connected normal-control playtest from the authored village-central
    # spawn: clear solid #38, jump above stone #95, then dash onto its top.
    village = 'Game/XmlFiles/levels/village_central.xml'
    village_start = root / 'village-smash-start.dl5'
    village_start.write_text('DL5\n' + village + '\n7470.876953 -1538.940674 25 100\nB:0\nU:kDownwardDash\n')
    village_route = 'R:65,E:40,R:50,E:35,M:1,.:240'
    village_saved = root / 'village-smash-earned.dl5'
    village_result = smash_trace(village_start, village_route, village_saved)
    assert 'Smashed ' + village + ' at frame 190 spawned_orbs=8' in village_result, village_result
    assert 'village_central.xml player=(8915.14,-1223.31) orbs=10' in village_result and 'pending_smash_orbs=0' in village_result, village_result
    assert 'health=25' in village_result and 'balance=18' in village_result, village_result
    village_lines = village_saved.read_text().splitlines()
    assert 'Z:' + village in village_lines and 'B:18' in village_lines
    assert sum(line.startswith(village + '#') for line in village_lines) == 10
    village_resume = smash_trace(village_saved, '.:120')
    assert 'Smashed ' not in village_resume and 'pending_smash_orbs=0' in village_resume and 'balance=18' in village_resume, village_resume
    village_export = root / 'village-smash-earned.xml'
    run('export', village_saved, village_export, '--discard-runtime')
    assert bridge.windows_fields(village_export)[bridge.smash_field(village)] == ('bool', 'true')
    bad_smash = root / 'smash-bad.dl5'
    bad_smash.write_text(target.read_text().replace('Z:' + grass_smash + '\n', ''))
    run('export', bad_smash, root / 'smash-bad.xml', '--discard-runtime', success=False)
    bad_smash.write_text(target.read_text() + 'Z:' + grass_smash + '\n')
    run('export', bad_smash, root / 'smash-bad.xml', '--discard-runtime', success=False)
    shipped_papers = [value for kind, value in shipped['paper_pickups_collected'][1]]
    assert {'P:' + paper for paper in shipped_papers} <= set(target.read_text().splitlines())
    # Original grass-3 PaperPickup #103 uses its description loc_id as the
    # permanent key. Interaction should collect once, then hide on reload.
    paper_spawn = '329.692871 -1335 25 100'
    paper_fresh = root / 'paper-fresh.dl5'
    paper_fresh.write_text('DL5\n' + level + '\n' + paper_spawn + '\nB:0\n')
    paper_saved = root / 'paper-collected.dl5'
    collected = subprocess.run(['port/build/deathleak-linux', '--load-save', str(paper_fresh),
                                '--save-file', str(paper_saved), '--script', 'I:5',
                                '--fixed-dt', '0.0166667', '--simulate-only'],
                               capture_output=True, text=True, check=True)
    paper_id = 'paper_pickup_gametip_projectile'
    assert 'Collected paper ' + paper_id + ' at frame 2' in collected.stdout, collected.stdout
    assert 'P:' + paper_id in paper_saved.read_text().splitlines()
    assert 'Opened paper modal title="GAME TIP" description="Hold the RIGHT THUMBSTICK' in collected.stdout
    # The modal freezes world simulation, ignores dismissal before one second,
    # then resumes normal movement after an explicit later dismissal.
    def paper_modal_trace(script, locale=None):
        args = ['port/build/deathleak-linux', '--load-save', str(paper_fresh),
                '--save-file', str(root / ('paper-modal-' + str(len(list(root.iterdir()))) + '.dl5')),
                '--script', script, '--fixed-dt', '0.0166667', '--simulate-only']
        if locale: args += ['--language', locale]
        return subprocess.run(args, capture_output=True, text=True, check=True).stdout
    opened = paper_modal_trace('I:3')
    frozen = paper_modal_trace('I:3,R:30,X:1,.:65')
    dismissed = paper_modal_trace('I:3,R:30,X:1,.:65,X:1,R:30')
    position = lambda trace: trace.split(' player=(', 1)[1].split(')', 1)[0]
    assert position(opened) == position(frozen), (opened, frozen)
    assert 'Dismissed paper modal' not in frozen
    assert 'Dismissed paper modal at frame 99' in dismissed and position(dismissed) != position(frozen)
    # strings.xml currently contains only en-gb; selected pt-br falls back to
    # that actual authored string while retaining the selected locale.
    localized = paper_modal_trace('I:3', 'pt-br')
    assert 'locale=pt-br' in localized and 'title="GAME TIP" description="Hold the RIGHT THUMBSTICK' in localized
    paper_repeat = subprocess.run(['port/build/deathleak-linux', '--load-save', str(paper_saved),
                                   '--script', 'I:5', '--fixed-dt', '0.0166667', '--simulate-only'],
                                  capture_output=True, text=True, check=True)
    assert 'Collected paper ' not in paper_repeat.stdout, paper_repeat.stdout
    paper_windows = root / 'paper-export.xml'
    run('export', paper_saved, paper_windows, '--discard-runtime')
    assert bridge.windows_fields(paper_windows)['paper_pickups_collected'] == ('vector', [('string', paper_id)])
    # An imported paper is active immediately; revisit cannot recollect it.
    paper_import = root / 'paper-import-location.dl5'
    paper_lines = target.read_text().splitlines()
    paper_lines[2] = paper_spawn
    paper_import.write_text('\n'.join(paper_lines) + '\n')
    imported_paper = subprocess.run(['port/build/deathleak-linux', '--load-save', str(paper_import),
                                     '--script', 'I:5', '--fixed-dt', '0.0166667', '--simulate-only'],
                                    capture_output=True, text=True, check=True)
    assert 'Collected paper ' not in imported_paper.stdout, imported_paper.stdout
    # HealthDevil is level-keyed in Windows permanent data. A grounded held
    # interaction grants one +20 health cap in this bounded Linux path; health
    # itself is not healed, and save/revisit cannot grant it again.
    hot = 'Game/XmlFiles/levels/hot_springs.xml'
    hot_start = root / 'devil-fresh.dl5'
    hot_start.write_text('DL5\n' + hot + '\n-1234.3479 450 25 100\nB:0\n')
    hot_earned = root / 'devil-earned.dl5'
    earned = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_start),
                             '--save-file', str(hot_earned), '--script', 'I:120',
                             '--fixed-dt', '0.0166667', '--simulate-only'],
                            capture_output=True, text=True, check=True)
    assert 'Health devil reward ' + hot + ' at frame 20 count=1 max_health=45' in earned.stdout, earned.stdout
    assert 'health=25' in earned.stdout and 'max_health=45' in earned.stdout, earned.stdout
    assert {'V:' + hot, 'J:1', 'H:45'} <= set(hot_earned.read_text().splitlines())
    repeated = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_earned),
                               '--script', 'I:120', '--fixed-dt', '0.0166667', '--simulate-only'],
                              capture_output=True, text=True, check=True)
    assert 'Health devil reward ' not in repeated.stdout and 'max_health=45' in repeated.stdout, repeated.stdout
    hot_export = root / 'devil-export.xml'
    run('export', hot_earned, hot_export, '--discard-runtime')
    assert bridge.windows_fields(hot_export)['health_devil_reward_XmlFiles-levels-hot_springs.xml'] == ('bool', 'true')
    assert bridge.windows_fields(hot_export)['health_devil_reward_count'] == ('int', '1')
    # Shipped flags/count become active, and the third authored devil can
    # extend the migrated cap/count without losing original typed fields.
    target_lines = target.read_text().splitlines()
    assert {'J:2', 'V:' + hot, 'V:Game/XmlFiles/levels/waterfall_cave.xml'} <= set(target_lines)
    imported_hot = root / 'devil-import-location.dl5'
    imported_lines = target_lines.copy()
    imported_lines[1] = hot
    imported_lines[2] = '-1234.3479 450 25 100'
    imported_hot.write_text('\n'.join(imported_lines) + '\n')
    already = subprocess.run(['port/build/deathleak-linux', '--load-save', str(imported_hot),
                              '--script', 'I:120', '--fixed-dt', '0.0166667', '--simulate-only'],
                             capture_output=True, text=True, check=True)
    assert 'Health devil reward ' not in already.stdout and 'max_health=65' in already.stdout, already.stdout
    cliff = 'Game/XmlFiles/levels/cliff.xml'
    third_start = root / 'devil-third.dl5'
    third_lines = target_lines.copy(); third_lines[1] = cliff; third_lines[2] = '-2097.966797 -500 25 100'
    third_start.write_text('\n'.join(third_lines) + '\n')
    third_earned = root / 'devil-third-earned.dl5'
    third = subprocess.run(['port/build/deathleak-linux', '--load-save', str(third_start),
                            '--save-file', str(third_earned), '--script', 'I:120',
                            '--fixed-dt', '0.0166667', '--simulate-only'],
                           capture_output=True, text=True, check=True)
    assert 'Health devil reward ' + cliff + ' at frame 21 count=3 max_health=85' in third.stdout, third.stdout
    third_win = root / 'devil-third.xml'
    run('export', third_earned, third_win, '--discard-runtime')
    third_fields = bridge.windows_fields(third_win)
    assert third_fields['health_devil_reward_XmlFiles-levels-cliff.xml'] == ('bool', 'true')
    assert third_fields['health_devil_reward_count'] == ('int', '3')
    assert third_fields['player_max_health'] == ('int', '85')
    for name, value in shipped.items():
        if name not in ('player_max_health', 'health_devil_reward_count'): assert third_fields[name] == value
    bad_reward = root / 'devil-bad.dl5'
    bad_reward.write_text(target.read_text().replace('V:' + hot + '\n', ''))
    run('export', bad_reward, root / 'devil-bad.xml', '--discard-runtime', success=False)
    bad_reward.write_text(target.read_text().replace('J:2', 'J:1'))
    run('export', bad_reward, root / 'devil-bad.xml', '--discard-runtime', success=False)
    for movement_unlock in ('kDownwardDash', 'kRoll'):
        assert 'U:' + movement_unlock in target.read_text().splitlines()
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
    assert {'P:' + paper for paper in shipped_papers} <= set(resumed.read_text().splitlines())
    assert {'Z:' + smash for smash in shipped_smashes} <= set(resumed.read_text().splitlines())
    assert 'H:65' in resumed.read_text().splitlines()
    assert 'U:kSpeedIncrease' in resumed.read_text().splitlines()
    for movement_unlock in ('kDownwardDash', 'kRoll'):
        assert 'U:' + movement_unlock in resumed.read_text().splitlines()
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
    # Imported permanent unlocks must change real movement and focus, while
    # their absent-mirror baseline keeps the old behavior. The open authored
    # floor fixture exercises the existing swept floor collision on dash landing.
    def movement_state(lines, position, script):
        path = root / ('motion-' + str(len(list(root.glob('motion-*.dl5')))) + '.dl5')
        data = lines.copy()
        data[1] = 'port/tests/fixtures/encounter_open.xml'
        data[2] = position
        path.write_text('\n'.join(data) + '\n')
        result = subprocess.run(['port/build/deathleak-linux', '--load-save', str(path),
                                 '--script', script, '--fixed-dt', '0.0166667', '--simulate-only'],
                                capture_output=True, text=True, check=True)
        match = re.search(r'player=\(([-0-9.]+),([-0-9.]+)\).*focus=([-0-9.]+).*grounded=([01]).*dash_active=([01]) roll_active=([01])', result.stdout)
        assert match, result.stdout
        return tuple(float(match.group(i)) for i in range(1, 7))
    no_motion = [line for line in upgraded if line not in ('U:kDownwardDash', 'U:kRoll')]
    dash = movement_state(upgraded, '0 600 25 100', '.:15,M:1')
    ordinary_dash = movement_state(no_motion, '0 600 25 100', '.:15,M:1')
    assert dash[1] < ordinary_dash[1] - 15 and 50 < dash[2] < 51 and ordinary_dash[2] == 100, (dash, ordinary_dash)
    early = movement_state(upgraded, '0 600 25 100', 'M:1')
    assert early[2] == 100 and early[4] == 0, early
    landing = movement_state(upgraded, '0 600 25 100', '.:15,M:45')
    assert landing[3] == 1 and landing[4] == 0 and landing[2] > 50, landing
    roll = movement_state(upgraded, '0 107.44937 25 100', '.:1,O:10')
    ordinary_roll = movement_state(no_motion, '0 107.44937 25 100', '.:1,O:10')
    assert roll[0] > ordinary_roll[0] + 50 and roll[2] > 98 and roll[2] < 100 and roll[5] == 1, (roll, ordinary_roll)
    assert ordinary_roll[5] == 0 and ordinary_roll[2] == 100, ordinary_roll
    buffered = movement_state(upgraded, '0 200 25 100', 'O:1,.:15')
    assert buffered[0] > 50 and buffered[5] == 1, buffered
    wall_path = root / 'roll-wall.dl5'
    wall_lines = upgraded.copy()
    wall_lines[1] = 'port/tests/fixtures/slope_wall.xml'
    wall_lines[2] = '0 107.44937 25 100'
    wall_path.write_text('\n'.join(wall_lines) + '\n')
    wall_result = subprocess.run(['port/build/deathleak-linux', '--load-save', str(wall_path),
                                  '--script', '.:1,O:30', '--fixed-dt', '0.0166667', '--simulate-only'],
                                 capture_output=True, text=True, check=True)
    assert 'player=(235.52,' in wall_result.stdout and 'vx=0 ' in wall_result.stdout, wall_result.stdout

    # A low-focus attempt never starts either movement or spends focus.
    assert movement_state(upgraded, '0 600 25 40', '.:15,M:1')[2] > 40
    assert movement_state(upgraded, '0 107.44937 25 1', '.:1,O:1')[5] == 0
    restored = root / 'restored.xml'
    run('export', resumed, restored, '--discard-runtime')
    # Windows defaults absent completion count to zero; export writes it explicitly.
    assert bridge.windows_fields(restored) == (shipped | {'num_times_game_completed': ('int', '0')})
    conflict = root / 'conflict.dl5'
    conflict.write_text(resumed.read_text().replace('H:65', 'H:25'))
    run('export', conflict, root / 'conflict.xml', '--discard-runtime', success=False)
    assert not (root / 'conflict.xml').exists()
    conflict.write_text(resumed.read_text().replace('P:' + shipped_papers[0] + '\n', ''))
    run('export', conflict, root / 'conflict.xml', '--discard-runtime', success=False)
    conflict.write_text(resumed.read_text().replace('U:kSpeedIncrease\n', ''))
    run('export', conflict, root / 'conflict.xml', '--discard-runtime', success=False)
    for unlock in ('kProjectileDamageIncrease', 'kProjectileSpeedIncrease', 'kDownwardDash', 'kRoll'):
        conflict.write_text(resumed.read_text().replace('U:' + unlock + '\n', ''))
        run('export', conflict, root / 'conflict.xml', '--discard-runtime', success=False)
    # An imported permanent record survives a Linux resave, but EndStory must
    # wipe it before the same-process menu starts a genuinely fresh game.
    ending = root / 'ending.dl5'
    ending_lines = target.read_text().splitlines()
    ending_lines[1] = 'Game/XmlFiles/levels/sea_2.xml'
    ending_lines[2] = '0 0 25 100'
    ending.write_text('\n'.join(ending_lines) + '\n')
    ended = subprocess.run(['port/build/deathleak-linux', '--load-save', str(ending),
                            '--menu-loop', '--script', '.:2101,S:1,.:1',
                            '--fixed-dt', '0.0166667', '--simulate-only'],
                           capture_output=True, text=True, check=True)
    assert 'Menu New Game at frame 2101' in ended.stdout, ended.stdout
    fresh_lines = ending.read_text().splitlines()
    assert fresh_lines[0] == 'DL5' and not any(line.startswith(('U:', 'W1:', 'H:', 'P:', 'V:', 'J:', 'Z:')) for line in fresh_lines), fresh_lines
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
