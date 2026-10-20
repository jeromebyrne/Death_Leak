#!/usr/bin/env python3
"""Integration coverage for the explicit permanent-save subset and isolation."""
import hashlib
import importlib.util
import sys
sys.dont_write_bytecode = True
from pathlib import Path
import subprocess
import tempfile
import re
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
    # interaction starts authored VO; its completion spawns a separate pickup.
    # Collection alone raises the cap; health is not healed or duplicated.
    hot = 'Game/XmlFiles/levels/hot_springs.xml'
    hot_start = root / 'devil-fresh.dl5'
    hot_start.write_text('DL5\n' + hot + '\n-1234.3479 450 25 100\nB:0\n')
    before_voice = root / 'devil-dialog-pending.dl5'
    pending = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_start),
                              '--save-file', str(before_voice), '--script', 'I:120',
                              '--fixed-dt', '0.0166667', '--simulate-only'],
                             capture_output=True, text=True, check=True)
    assert 'Health devil dialog ' + hot in pending.stdout
    assert 'Health devil reward spawned ' not in pending.stdout
    assert 'max_health=25' in pending.stdout
    assert not any(line.startswith(('V:', 'J:', 'H:')) for line in before_voice.read_text().splitlines())
    # HealthDevil::Update is manager-culled by its drawable bounds. The voice
    # can finish while the player/camera are far away, but GiveReward must wait
    # until the devil is visited again; remaining nearby still rewards on time.
    zone_fixture = 'port/tests/fixtures/health_devil_update_zone.xml'
    zone_cmd = ['port/build/deathleak-linux', '--play', zone_fixture,
                '--fixed-dt', '0.0166667', '--simulate-only']
    far_dialog = subprocess.run(zone_cmd + ['--script', 'I:4,R:400,.:400'],
                                capture_output=True, text=True, check=True)
    assert f'Health devil dialog {zone_fixture} at frame 2' in far_dialog.stdout, far_dialog.stdout
    assert 'Health devil reward spawned ' not in far_dialog.stdout, far_dialog.stdout
    assert 'max_health=25' in far_dialog.stdout, far_dialog.stdout
    return_dialog = subprocess.run(zone_cmd + ['--script', 'I:4,R:400,.:400,L:400,.:20'],
                                   capture_output=True, text=True, check=True)
    assert f'Health devil reward spawned {zone_fixture} at frame 959 count=1' in return_dialog.stdout, return_dialog.stdout
    assert 'Health upgrade collected ' not in return_dialog.stdout and 'max_health=25' in return_dialog.stdout, return_dialog.stdout
    near_dialog = subprocess.run(zone_cmd + ['--script', 'I:4,.:800'],
                                 capture_output=True, text=True, check=True)
    assert f'Health devil reward spawned {zone_fixture} at frame 749 count=1' in near_dialog.stdout, near_dialog.stdout
    # The shipped HealthDevils explicitly set always_update=true: unlike the
    # non-always-update fixture above, their finished voice may reward off-zone.
    always_devil = root / 'always-health-devil.xml'
    always_devil.write_text(Path(zone_fixture).read_text().replace(
        '<HealthDevil id="3">', '<HealthDevil id="3" updateable="true" always_update="true">'))
    always_result = subprocess.run(['port/build/deathleak-linux', '--play', str(always_devil),
                                    '--script', 'I:4,R:400,.:400', '--fixed-dt', '0.0166667',
                                    '--simulate-only'], capture_output=True, text=True, check=True)
    assert f'Health devil reward spawned {always_devil} at frame 749 count=1' in always_result.stdout, always_result.stdout
    assert 'Health upgrade collected ' not in always_result.stdout, always_result.stdout
    # A camera initially far from the authored spawn leaves two overlapping
    # interactables outside the manager update zone. Held I cannot pre-charge
    # either object; each starts its own 0.05-second hold when visited.
    authored_zone = 'port/tests/fixtures/authored_interactable_update_zone.xml'
    zone_args = ['port/build/deathleak-linux', '--play', authored_zone,
                 '--fixed-dt', '0.0166667', '--simulate-only']
    before_visit = subprocess.run(zone_args + ['--script', 'I:4'],
                                  capture_output=True, text=True, check=True)
    assert 'Collected key ' not in before_visit.stdout and 'Collected paper ' not in before_visit.stdout, before_visit.stdout
    visited_pair = subprocess.run(zone_args + ['--script', 'I:15'],
                                  capture_output=True, text=True, check=True)
    assert 'Collected key zone_key at frame 7' in visited_pair.stdout, visited_pair.stdout
    assert 'Collected paper paper_pickup_gametip_projectile at frame 10' in visited_pair.stdout, visited_pair.stdout
    # always_update bypasses the rectangle, while updateable=false excludes
    # the key entirely, matching GameObjectManager's two early branches.
    zone_xml = Path(authored_zone).read_text()
    always_key = root / 'always-key.xml'
    always_key.write_text(zone_xml.replace('id="3" updateable="true" always_update="false"',
                                           'id="3" updateable="true" always_update="true"'))
    always_key_result = subprocess.run(['port/build/deathleak-linux', '--play', str(always_key),
                                        '--script', 'I:4', '--fixed-dt', '0.0166667',
                                        '--simulate-only'], capture_output=True, text=True, check=True)
    assert 'Collected key zone_key at frame 2' in always_key_result.stdout, always_key_result.stdout
    disabled_key = root / 'disabled-key.xml'
    disabled_key.write_text(zone_xml.replace('id="3" updateable="true" always_update="false"',
                                               'id="3" updateable="false" always_update="true"'))
    disabled_result = subprocess.run(['port/build/deathleak-linux', '--play', str(disabled_key),
                                     '--script', 'I:15', '--fixed-dt', '0.0166667',
                                     '--simulate-only'], capture_output=True, text=True, check=True)
    assert 'Collected key ' not in disabled_result.stdout, disabled_result.stdout
    assert 'Collected paper paper_pickup_gametip_projectile at frame 7' in disabled_result.stdout, disabled_result.stdout
    # Game::Update calls Camera2D::FollowTargetObjectWithLag before manager
    # visitation. A disabled axis stays at its authored initial position;
    # an enabled axis advances by the authored per-update divisor (not dt).
    camera_fixture = Path('port/tests/fixtures/camera_follow_update_zone.xml')
    camera_xml = camera_fixture.read_text()
    def camera_trace(xml):
        variant = root / 'camera-follow-variant.xml'
        variant.write_text(xml)
        return subprocess.run(['port/build/deathleak-linux', '--play', str(variant),
                               '--script', 'I:8', '--fixed-dt', '0.0166667',
                               '--simulate-only'], capture_output=True, text=True, check=True).stdout
    fixed_camera = camera_trace(camera_xml)
    assert 'Collected key camera_follow_key' not in fixed_camera, fixed_camera
    followed_camera = camera_trace(camera_xml.replace('followx="false"', 'followx="true"'))
    assert 'Collected key camera_follow_key at frame 2' in followed_camera, followed_camera
    slow_camera = camera_trace(camera_xml.replace('followx="false"', 'followx="true"').replace('xLag="2"', 'xLag="300"'))
    assert 'Collected key camera_follow_key' not in slow_camera, slow_camera
    offset_camera = camera_trace(camera_xml.replace('followx="false"', 'followx="true"').replace('xOffset="0"', 'xOffset="-2500"'))
    assert 'Collected key camera_follow_key' not in offset_camera, offset_camera
    # Camera2D::CheckBoundaryCollisions runs before and after follow; its
    # viewport dimensions are the unzoomed 1920x1080 backbuffer. A left
    # boundary can make an otherwise culled key manager-visible before follow.
    left_bounded = camera_xml.replace('followx="false"', 'followx="false"').replace(
        '</LevelProperties>', '<level_bounds left="2000" right="8000" top="2000" bottom="-2000"/></LevelProperties>')
    assert 'Collected key camera_follow_key at frame 2' in camera_trace(left_bounded)
    # Conversely, post-follow right clamping prevents a key at x=6000 from
    # entering the update zone even with instantaneous follow.
    distant = camera_xml.replace('2500', '6000').replace('followx="false"', 'followx="true"').replace('xLag="2"', 'xLag="1"')
    assert 'Collected key camera_follow_key at frame 2' in camera_trace(distant)
    right_bounded = distant.replace('</LevelProperties>',
        '<level_bounds left="-8000" right="3000" top="2000" bottom="-2000"/></LevelProperties>')
    assert 'Collected key camera_follow_key' not in camera_trace(right_bounded)
    # Source Game::Update follows the player's position from the preceding
    # update, before the manager advances this frame's player. Frame 0 builds
    # real rightward momentum and the test-only camera trace fixes the prior
    # camera at x=0. On frame 1 the key's drawable edge is just outside the
    # zone with the previous position, but inside if follow uses new motion.
    timing_camera = root / 'timing-camera.csv'
    timing_camera.write_text('0,0,107.44937\n')
    timing = subprocess.run(['port/build/deathleak-linux', '--play',
                             'port/tests/fixtures/camera_follow_timing.xml',
                             '--camera-script', str(timing_camera),
                             '--script', 'R:1,I:6', '--fixed-dt', '0.0166667',
                             '--simulate-only'], capture_output=True, text=True, check=True)
    assert 'Collected key camera_timing_key at frame 4' in timing.stdout, timing.stdout
    assert 'Collected key camera_timing_key at frame 3' not in timing.stdout, timing.stdout
    # GameObject::UpdateInteractable is not called when its manager zone culls
    # the object. Its static lock and two-tick partial countdown survive even
    # when I is released off-zone; another visited object cannot steal the lock.
    # The simulation-only camera trace forces the otherwise-unreachable cull
    # while both key rectangles continue to overlap the stationary player.
    lock_fixture = 'port/tests/fixtures/interactable_lock_zone.xml'
    camera_trace = root / 'interaction-camera.csv'
    camera_trace.write_text(''.join(f'{at},2200,107.44937\n' for at in range(2, 6)) +
                            '6,100,107.44937\n')
    lock_args = ['port/build/deathleak-linux', '--play', lock_fixture,
                 '--fixed-dt', '0.0166667', '--simulate-only',
                 '--camera-script', str(camera_trace)]
    blocked = subprocess.run(lock_args + ['--script', 'I:6'],
                             capture_output=True, text=True, check=True)
    assert 'Collected key ' not in blocked.stdout, blocked.stdout
    locked = subprocess.run(lock_args + ['--script', 'I:2,.:4'],
                            capture_output=True, text=True, check=True)
    assert 'Collected key ' not in locked.stdout, locked.stdout
    resumed = subprocess.run(lock_args + ['--script', 'I:2,.:4,I:4'],
                             capture_output=True, text=True, check=True)
    assert 'Collected key lock_first at frame 6' in resumed.stdout, resumed.stdout
    assert 'Collected key lock_second at frame 9' in resumed.stdout, resumed.stdout
    reset_on_visit = subprocess.run(lock_args + ['--script', 'I:2,.:5,I:3'],
                                    capture_output=True, text=True, check=True)
    assert 'Collected key lock_first at frame 9' in reset_on_visit.stdout, reset_on_visit.stdout
    assert 'Collected key lock_second' not in reset_on_visit.stdout, reset_on_visit.stdout
    # Manager visitation is depth-descending. Momentum carries the player out
    # of the selected key's overlap on the first renewed I frame. A competing
    # earlier object has already run and misses that frame's hold; a later
    # object acquires immediately when the selected key releases its lock.
    release_fixture = Path('port/tests/fixtures/interactable_release_order.xml')
    release_camera = root / 'release-order-camera.csv'
    release_camera.write_text(''.join(f'{at},-2000,107.44937\n' for at in range(3)) +
                              '3,100,107.44937\n')
    def release_trace(fixture):
        return subprocess.run(['port/build/deathleak-linux', '--play', str(fixture),
                               '--script', 'I:2,R:1,I:5', '--fixed-dt', '0.0166667',
                               '--simulate-only', '--camera-script', str(release_camera)],
                              capture_output=True, text=True, check=True).stdout
    earlier = release_trace(release_fixture)
    assert 'Collected key candidate_earlier at frame 6' in earlier, earlier
    assert 'Collected key release_later' not in earlier, earlier
    later_fixture = root / 'interactable-release-later-candidate.xml'
    later_fixture.write_text(release_fixture.read_text().replace(
        'x="0" y="107.44937" depth_layer="kGroundFront"',
        'x="0" y="107.44937" depth_layer="kGround"'))
    later = release_trace(later_fixture)
    assert 'Collected key candidate_earlier at frame 5' in later, later
    assert 'Collected key release_later' not in later, later
    rejected_live_camera = subprocess.run(['port/build/deathleak-linux', '--play', lock_fixture,
                                           '--camera-script', str(camera_trace)],
                                          capture_output=True, text=True)
    assert rejected_live_camera.returncode == 2 and '--camera-script requires' in rejected_live_camera.stderr
    # Connected play: leave the devil during VO, observe the spawned object
    # without a cap increase, then return and physically collect it.
    uncollected = root / 'devil-uncollected.dl5'
    away = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_start),
                           '--save-file', str(uncollected),
                           '--script', 'I:25,L:100,.:750', '--fixed-dt', '0.0166667', '--simulate-only'],
                          capture_output=True, text=True, check=True)
    assert 'Health devil reward spawned ' + hot + ' at frame 767 count=1' in away.stdout
    assert 'Health upgrade collected ' not in away.stdout and 'max_health=25' in away.stdout
    # Pickup::OnCollision never resolves solids: after crossing the authored
    # ground it continues falling instead of settling at the old -97.3599 Y.
    fallen = re.search(r'Health upgrade pickup state x=([^\s]+) y=([^\s]+) vx=([^\s]+) vy=([^\s]+)', away.stdout)
    assert fallen and abs(float(fallen[1]) - (-935.589)) < .02 and float(fallen[2]) < -1000, away.stdout
    assert float(fallen[4]) == -20, away.stdout
    # The authored pickup launches from the devil's (-1234.3479, 24) at
    # (15,10). MovingSprite integrates old acceleration, then applies .8
    # gravity on ascent and 2.4 while descending above -15 speed.
    for idle, expected in ((650, (-1143.849, 97.2, 5.2)),
                           (660, (-1059.784, 95.6, -7.6))):
        flight = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_start),
                                 '--save-file', str(root / f'devil-flight-{idle}.dl5'),
                                 '--script', f'I:25,L:100,.:{idle}', '--fixed-dt', '0.0166667', '--simulate-only'],
                                capture_output=True, text=True, check=True)
        state = re.search(r'Health upgrade pickup state x=([^\s]+) y=([^\s]+) vx=([^\s]+) vy=([^\s]+)', flight.stdout)
        assert state, flight.stdout
        for actual, wanted in zip((float(state[1]), float(state[2]), float(state[4])), expected):
            assert abs(actual - wanted) < .02, flight.stdout
    # A source-style spawned pickup also passes through a static solid box
    # below the ground line, while the player remains on the line above it.
    pass_through = subprocess.run(['port/build/deathleak-linux', '--play',
                                   'port/tests/fixtures/health_pickup_pass_through.xml',
                                   '--script', 'I:4,.:800', '--fixed-dt', '0.0166667',
                                   '--simulate-only'], capture_output=True, text=True, check=True)
    assert 'Health devil reward spawned port/tests/fixtures/health_pickup_pass_through.xml at frame 749' in pass_through.stdout
    box_state = re.search(r'Health upgrade pickup state x=([^\s]+) y=([^\s]+) vx=([^\s]+) vy=([^\s]+)', pass_through.stdout)
    assert box_state and 0 < float(box_state[1]) < 400 and float(box_state[2]) < -235, pass_through.stdout
    assert 'max_health=25' in pass_through.stdout and 'grounded=1' in pass_through.stdout
    # GameObjectManager culls the spawned, updateable pickup against a
    # camera-centered 4000-square zone. Its native 50x50 art extends the
    # strict boundary by 25 units, so it receives the final downward step
    # before freezing outside the zone; more idle frames cannot advance it.
    frozen_states = []
    for idle in (900, 1100):
        far = subprocess.run(['port/build/deathleak-linux', '--play',
                              'port/tests/fixtures/health_pickup_pass_through.xml',
                              '--script', f'I:4,.:{idle}', '--fixed-dt', '0.0166667',
                              '--simulate-only'], capture_output=True, text=True, check=True)
        state = re.search(r'Health upgrade pickup state x=([^\s]+) y=([^\s]+) vx=([^\s]+) vy=([^\s]+)', far.stdout)
        assert state and 'max_health=25' in far.stdout, far.stdout
        frozen_states.append(tuple(map(float, state.groups())))
    assert frozen_states[0] == frozen_states[1], frozen_states
    assert abs(frozen_states[0][1] - (-1919.2)) < .02, frozen_states
    # AddGameObject re-sorts the live list. On a tie, XML objects keep their
    # earlier order; the overlapping authored key wins the first hold.
    # its removal allows a fresh 0.05-second hold on the still-overlapping pickup.
    arbitration_cmd = ['port/build/deathleak-linux', '--play',
                       'port/tests/fixtures/health_pickup_arbitration.xml',
                       '--fixed-dt', '0.0166667', '--simulate-only']
    key_first = subprocess.run(arbitration_cmd + ['--script', 'I:4,R:10,.:735,I:3'],
                               capture_output=True, text=True, check=True)
    assert 'Collected key pickup_overlap_key at frame 751' in key_first.stdout, key_first.stdout
    assert 'Health upgrade collected ' not in key_first.stdout and 'max_health=25' in key_first.stdout, key_first.stdout
    then_pickup = subprocess.run(arbitration_cmd + ['--script', 'I:4,R:10,.:735,I:8'],
                                 capture_output=True, text=True, check=True)
    assert 'Collected key pickup_overlap_key at frame 751' in then_pickup.stdout, then_pickup.stdout
    assert 'Health upgrade collected port/tests/fixtures/health_pickup_arbitration.xml at frame 754 max_health=45' in then_pickup.stdout, then_pickup.stdout
    # HealthDevil inserts its pickup during GameObjectManager::Update. The
    # std::list iterator stays at the devil through the live depth sort. A
    # kPlayer pickup inserted after a kGround devil is visited this frame;
    # one inserted before a kGroundFront devil is first visited next frame.
    for suffix, expected_x, expected_y, collected_frame in (
        ('later', 15, 30, 751), ('earlier', 0, 20, 752)):
        fixture = f'port/tests/fixtures/health_pickup_spawn_{suffix}.xml'
        args = ['port/build/deathleak-linux', '--play', fixture,
                '--fixed-dt', '0.0166667', '--simulate-only']
        spawn = subprocess.run(args + ['--script', 'I:4,.:746'],
                               capture_output=True, text=True, check=True)
        assert f'Health devil reward spawned {fixture} at frame 749' in spawn.stdout, spawn.stdout
        state = re.search(r'Health upgrade pickup state x=([^\s]+) y=([^\s]+) vx=([^\s]+) vy=([^\s]+)', spawn.stdout)
        assert state and abs(float(state[1]) - expected_x) < .01 and abs(float(state[2]) - expected_y) < .01, spawn.stdout
        collected = subprocess.run(args + ['--script', 'I:755'],
                                   capture_output=True, text=True, check=True)
        assert f'Health upgrade collected {fixture} at frame {collected_frame} max_health=45' in collected.stdout, collected.stdout
    # A visited pickup checks interaction in Sprite::Update before its
    # MovingSprite motion. At x=90 the authored player box misses the pickup
    # at its spawn x=0 by 0.52 units, but overlaps after the first x=15 move.
    # That first moved frame must not count toward the 0.05-second hold.
    edge = 'port/tests/fixtures/health_pickup_overlap_edge.xml'
    edge_cmd = ['port/build/deathleak-linux', '--play', edge,
                '--fixed-dt', '0.0166667', '--simulate-only']
    edge_before = subprocess.run(edge_cmd + ['--script', 'I:752'],
                                 capture_output=True, text=True, check=True)
    assert f'Health devil reward spawned {edge} at frame 749' in edge_before.stdout, edge_before.stdout
    assert 'Health upgrade collected ' not in edge_before.stdout and 'max_health=25' in edge_before.stdout, edge_before.stdout
    edge_after = subprocess.run(edge_cmd + ['--script', 'I:753'],
                                capture_output=True, text=True, check=True)
    assert f'Health upgrade collected {edge} at frame 752 max_health=45' in edge_after.stdout, edge_after.stdout
    # Conversely x=-85 overlaps before the spawn-frame move, then leaves the
    # pickup behind. A 0.05-second fixed step completes exactly one hold on
    # that visited spawn frame, proving pre-motion overlap is honored too.
    exit_edge = 'port/tests/fixtures/health_pickup_overlap_exit.xml'
    exit_run = subprocess.run(['port/build/deathleak-linux', '--play', exit_edge,
                               '--script', 'I:250', '--fixed-dt', '0.05',
                               '--simulate-only'], capture_output=True, text=True, check=True)
    assert f'Health devil reward spawned {exit_edge} at frame 249' in exit_run.stdout, exit_run.stdout
    assert f'Health upgrade collected {exit_edge} at frame 249 max_health=45' in exit_run.stdout, exit_run.stdout
    # AddGameObject re-sorts on insertion: its kPlayer pickup precedes a
    # kGroundFront key, even though the key is earlier in XML. Neither target
    # overlaps until the first post-spawn movement tick, so there is no prior
    # sCurrentInteractable lock obscuring the depth comparison.
    depth_cmd = ['port/build/deathleak-linux', '--play',
                 'port/tests/fixtures/health_pickup_depth_priority.xml',
                 '--fixed-dt', '0.0166667', '--simulate-only']
    depth_first = subprocess.run(depth_cmd + ['--script', 'I:4,.:745,L:2,I:3'],
                                 capture_output=True, text=True, check=True)
    assert 'Health upgrade collected port/tests/fixtures/health_pickup_depth_priority.xml at frame 753 max_health=45' in depth_first.stdout, depth_first.stdout
    assert 'Collected key pickup_overlap_key' not in depth_first.stdout, depth_first.stdout
    depth_second = subprocess.run(depth_cmd + ['--script', 'I:4,.:745,L:2,I:8'],
                                  capture_output=True, text=True, check=True)
    assert 'Collected key pickup_overlap_key at frame 756' in depth_second.stdout, depth_second.stdout
    assert {'V:' + hot, 'J:1'} <= set(uncollected.read_text().splitlines())
    assert 'H:45' not in uncollected.read_text().splitlines()
    returned = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_start),
                               '--save-file', str(root / 'devil-returned.dl5'),
                               '--script', 'I:25,L:20,.:700,R:20,I:35',
                               '--fixed-dt', '0.0166667', '--simulate-only'],
                              capture_output=True, text=True, check=True)
    assert 'Health devil reward spawned ' + hot + ' at frame 767 count=1' in returned.stdout
    assert 'Health upgrade collected ' + hot + ' at frame 770 max_health=45' in returned.stdout
    uncollected_reload = subprocess.run(['port/build/deathleak-linux', '--load-save', str(uncollected),
                                         '--script', 'I:800', '--fixed-dt', '0.0166667', '--simulate-only'],
                                        capture_output=True, text=True, check=True)
    assert 'Health devil dialog ' not in uncollected_reload.stdout
    assert 'max_health=25' in uncollected_reload.stdout
    # Overlap alone must not grant the upgrade: Pickup::OnInteracted owns it.
    untouched = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_start),
                                '--save-file', str(root / 'devil-untouched.dl5'),
                                '--script', 'I:25,.:775', '--fixed-dt', '0.0166667', '--simulate-only'],
                               capture_output=True, text=True, check=True)
    assert 'Health devil reward spawned ' + hot in untouched.stdout
    assert 'Health upgrade collected ' not in untouched.stdout and 'max_health=25' in untouched.stdout
    hot_earned = root / 'devil-earned.dl5'
    earned = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_start),
                             '--save-file', str(hot_earned), '--script', 'I:800',
                             '--fixed-dt', '0.0166667', '--simulate-only'],
                            capture_output=True, text=True, check=True)
    assert 'Health devil dialog ' + hot + ' at frame 20 voice=1' in earned.stdout, earned.stdout
    assert 'Health devil reward spawned ' + hot + ' at frame 767 count=1' in earned.stdout, earned.stdout
    assert 'Health upgrade collected ' + hot + ' at frame 770 max_health=45' in earned.stdout, earned.stdout
    assert 'health=25' in earned.stdout and 'max_health=45' in earned.stdout, earned.stdout
    assert {'V:' + hot, 'J:1', 'H:45'} <= set(hot_earned.read_text().splitlines())
    repeated = subprocess.run(['port/build/deathleak-linux', '--load-save', str(hot_earned),
                               '--script', 'I:120', '--fixed-dt', '0.0166667', '--simulate-only'],
                              capture_output=True, text=True, check=True)
    assert 'Health devil dialog ' not in repeated.stdout and 'max_health=45' in repeated.stdout, repeated.stdout
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
    assert 'Health devil dialog ' not in already.stdout and 'max_health=65' in already.stdout, already.stdout
    cliff = 'Game/XmlFiles/levels/cliff.xml'
    third_start = root / 'devil-third.dl5'
    third_lines = target_lines.copy(); third_lines[1] = cliff; third_lines[2] = '-2097.966797 -500 25 100'
    third_start.write_text('\n'.join(third_lines) + '\n')
    third_earned = root / 'devil-third-earned.dl5'
    third = subprocess.run(['port/build/deathleak-linux', '--load-save', str(third_start),
                            '--save-file', str(third_earned), '--script', 'I:800',
                            '--fixed-dt', '0.0166667', '--simulate-only'],
                           capture_output=True, text=True, check=True)
    assert 'Health devil dialog ' + cliff + ' at frame 21 voice=3' in third.stdout, third.stdout
    assert 'Health devil reward spawned ' + cliff + ' at frame 732 count=3' in third.stdout, third.stdout
    assert 'Health upgrade collected ' + cliff + ' at frame 734 max_health=85' in third.stdout, third.stdout
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
