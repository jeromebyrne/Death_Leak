#!/usr/bin/env python3
"""Windows bargon.xml bridge with explicit preservation of unported permanent data."""
import argparse
import base64
import binascii
import os
from pathlib import Path
import re
import tempfile
import xml.etree.ElementTree as ET

ABC = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 '
KEY = 'babson'
KEYS = {'granary', 'dojo', 'village_hut', 'has_repair_tools'}
DOORS = {'barn_entrance', 'dojo', 'village_hut_entrance'}
LOCALES = {'en-gb', 'pt-br', 'de'}
# Source SaveManager stores these permanently. Linux does not yet implement their effects.
OPAQUE_BOOL = {
    'health_devil_reward_XmlFiles-levels-hot_springs.xml',
    'health_devil_reward_XmlFiles-levels-waterfall_cave.xml',
    'smashable_XmlFiles-levels-grass_exploration_1.xml',
    'smashable_XmlFiles-levels-village_central.xml',
}
OPAQUE_INT = {'health_devil_reward_count', 'player_max_health'}
PAPERS = {
    'paper_pickup_gametip_death', 'paper_pickup_gametip_projectile',
    'paper_pickup_strange_stones_desc', 'paper_pickup_desc_test',
    'dojo_closed_desc', 'paper_pickup_cave_clue_desc', 'suicide_note_desc',
    'save_shrine_hint_desc', 'paper_pickup_desc_granary_key_hint',
}


def validate_opaque(fields):
    for name, (kind, value) in fields.items():
        if name == 'gameplay_features_unlocked':
            if kind != 'vector' or any(t != 'int' or not re.fullmatch('[1-8]', v) for t, v in value):
                raise ValueError('unmapped feature in migration extension')
        elif name == 'paper_pickups_collected':
            if kind != 'vector' or any(t != 'string' or v not in PAPERS for t, v in value):
                raise ValueError('unmapped paper pickup in migration extension')
        elif name in OPAQUE_BOOL:
            if kind != 'bool' or value not in ('true', 'false'):
                raise ValueError('invalid migration boolean ' + name)
        elif name in OPAQUE_INT:
            if kind != 'int' or not re.fullmatch(r'\d+', value) or int(value) >= (1000001 if name == 'player_max_health' else 1000000000) or (name == 'player_max_health' and int(value) < 1):
                raise ValueError('invalid migration integer ' + name)
        else:
            raise ValueError('unmapped permanent Windows field: ' + name)


def field_xml(fields):
    root = ET.Element('save_data')
    for name, (kind, value) in fields.items():
        node = ET.SubElement(root, name, {'type': kind})
        if kind == 'vector':
            for i, (child_kind, child_value) in enumerate(value, 1):
                ET.SubElement(node, f'e_{i}', {'type': child_kind, 'value': child_value})
        else:
            node.set('value', value)
    return ET.tostring(root, encoding='utf-8')


def extension_fields(encoded):
    if len(encoded) > 65533:
        raise ValueError('migration extension too long')
    raw = base64.b64decode(encoded, validate=True)
    if len(raw) > 49152:
        raise ValueError('migration extension too large')
    root = ET.fromstring(raw)
    if root.tag != 'save_data':
        raise ValueError('invalid migration extension root')
    # Reuse the strict typed parser without trusting arbitrary XML payloads.
    wrapper = ET.Element('a', {'z': cipher(base64.b64encode(raw).decode())})
    fields = windows_fields_xml(ET.tostring(wrapper))
    validate_opaque(fields)
    return fields


def windows_fields_xml(payload):
    wrapper = ET.fromstring(payload)
    return parse_windows_wrapper(wrapper)


def cipher(value, decrypt=False):
    sign = -1 if decrypt else 1
    return ''.join(ABC[(ABC.index(c) + sign * ABC.index(KEY[i % len(KEY)])) % len(ABC)]
                   if c in ABC else c for i, c in enumerate(value))


def windows_fields(path):
    return parse_windows_wrapper(ET.parse(path).getroot())


def parse_windows_wrapper(wrapper):
    if wrapper.tag != 'a' or 'z' not in wrapper.attrib or set(wrapper.attrib) - {'z', 'content'} or list(wrapper):
        raise ValueError('expected encrypted bargon.xml wrapper')
    root = ET.fromstring(base64.b64decode(cipher(wrapper.get('z'), True), validate=True))
    if root.tag != 'save_data':
        raise ValueError('expected save_data')
    fields = {}
    for node in root:
        if node.tag in fields:
            raise ValueError('duplicate Windows field ' + node.tag)
        if node.get('type') == 'vector' and set(node.attrib) == {'type'}:
            value = []
            for i, child in enumerate(node, 1):
                if child.tag != f'e_{i}' or set(child.attrib) != {'type', 'value'} or list(child):
                    raise ValueError('invalid feature vector')
                value.append((child.get('type'), child.get('value')))
        elif node.get('type') in {'bool', 'int', 'string'} and set(node.attrib) == {'type', 'value'} and not list(node):
            value = node.get('value')
        else:
            raise ValueError('unsupported typed field ' + node.tag)
        fields[node.tag] = (node.get('type'), value)
    return fields


def windows_to_dl5(fields, level):
    fields = dict(fields)
    def take(name, kind):
        item = fields.pop(name, None)
        if item and item[0] != kind:
            raise ValueError('wrong type for ' + name)
        return None if item is None else item[1]
    count = take('num_times_game_completed', 'int')
    if count is None: count = '0'
    if not re.fullmatch(r'\d+', count) or int(count) >= 1000000000:
        raise ValueError('invalid completion count')
    locale = take('language_locale_key', 'string')
    if locale is not None and locale not in LOCALES:
        raise ValueError('unmapped language')
    features = take('gameplay_features_unlocked', 'vector')
    if features is not None:
        fields['gameplay_features_unlocked'] = ('vector', features)
    keys, doors = [], []
    for key in sorted(KEYS):
        value = take(key, 'bool')
        if value not in (None, 'true', 'false'):
            raise ValueError('invalid key boolean')
        if value == 'false': raise ValueError('explicit false key cannot be represented: ' + key)
        if value == 'true': keys.append(key)
    for door in sorted(DOORS):
        value = take(door + '_unlocked', 'bool')
        if value not in (None, 'true', 'false'):
            raise ValueError('invalid door boolean')
        if value == 'false': raise ValueError('explicit false door cannot be represented: ' + door)
        if value == 'true': doors.append(door)
    validate_opaque(fields)
    max_health = fields.get('player_max_health', ('int', '25'))[1]
    speed_unlocked = features is not None and ('int', '5') in features
    positions = ET.parse(level).getroot().findall('.//Player/position')
    if len(positions) != 1:
        raise ValueError('level needs exactly one authored Player spawn')
    x, y = (float(positions[0].get(axis)) for axis in ('x', 'y'))
    lines = ['DL5', str(level), f'{x:.9g} {y:.9g} 25 100', 'B:0', 'C:' + count]
    if locale: lines.append('G:' + locale)
    lines += ['K:' + key for key in keys] + ['D:' + door for door in doors]
    if features is not None and ('int', '4') in features: lines.append('U:kDeflection')
    if speed_unlocked: lines.append('U:kSpeedIncrease')
    for name, feature_id in (('kProjectileDamageIncrease', '6'), ('kProjectileSpeedIncrease', '7')):
        if features is not None and ('int', feature_id) in features: lines.append('U:' + name)
    if 'player_max_health' in fields: lines.append('H:' + max_health)
    if fields: lines.append('W1:' + base64.b64encode(field_xml(fields)).decode())
    return lines


def dl5_to_windows(path):
    lines = Path(path).read_text().splitlines()
    if len(lines) < 2 or lines[0] not in ('DL5', 'DL6'):
        raise ValueError('expected DL5 or DL6 save')
    if lines[0] == 'DL5':
        if len(lines) < 4 or not Path(lines[1]).is_file() or len(lines[2].split()) != 4:
            raise ValueError('invalid DL5 header')
        data = lines[3:]
    elif lines[1] == 'MENU': data = lines[2:]
    else: raise ValueError('invalid DL6 MENU save')
    fields, keys, doors, runtime = {}, set(), set(), []
    migration = None
    for line in data:
        if line.startswith('C:'):
            if 'C' in fields or not re.fullmatch(r'\d+', line[2:]) or int(line[2:]) >= 1000000000:
                raise ValueError('invalid/duplicate completion count')
            fields['C'] = line[2:]
        elif line.startswith('G:'):
            if 'G' in fields or line[2:] not in LOCALES: raise ValueError('invalid/duplicate locale')
            fields['G'] = line[2:]
        elif line.startswith('K:'):
            if line[2:] not in KEYS or line[2:] in keys: raise ValueError('unmapped/duplicate key')
            keys.add(line[2:])
        elif line.startswith('D:'):
            if line[2:] not in DOORS or line[2:] in doors: raise ValueError('unmapped/duplicate door')
            doors.add(line[2:])
        elif line in ('U:kDeflection', 'U:kSpeedIncrease', 'U:kProjectileDamageIncrease', 'U:kProjectileSpeedIncrease'):
            unlock = line[2:]
            if unlock in fields: raise ValueError('duplicate unlock')
            fields[unlock] = True
        elif line.startswith('H:'):
            if 'H' in fields or not re.fullmatch(r'[1-9]\d*', line[2:]) or int(line[2:]) > 1000000:
                raise ValueError('invalid/duplicate max health')
            fields['H'] = line[2:]
        elif line.startswith('W1:'):
            if migration is not None or lines[0] != 'DL5': raise ValueError('duplicate/invalid migration extension')
            migration = extension_fields(line[3:])
        elif line.startswith(('B:', 'N:')) or '#' in line: runtime.append(line)
        else: raise ValueError('unmapped Linux line: ' + line)
    if lines[0] == 'DL6' and (keys or doors or runtime or fields.get('kDeflection') or fields.get('kSpeedIncrease') or fields.get('kProjectileDamageIncrease') or fields.get('kProjectileSpeedIncrease') or fields.get('H') or migration):
        raise ValueError('DL6 MENU cannot carry progression')
    if lines[0] == 'DL5' and len([v for v in runtime if v.startswith('B:')]) != 1:
        raise ValueError('DL5 requires one balance')
    root = ET.Element('save_data')
    def scalar(name, kind, value):
        ET.SubElement(root, name, {'type': kind, 'value': str(value)})
    scalar('num_times_game_completed', 'int', fields.get('C', '0'))
    if 'G' in fields: scalar('language_locale_key', 'string', fields['G'])
    for key in sorted(keys): scalar(key, 'bool', 'true')
    for door in sorted(doors): scalar(door + '_unlocked', 'bool', 'true')
    if fields.get('H'):
        if migration is None: migration = {}
        previous = migration.get('player_max_health')
        if previous and previous != ('int', fields['H']):
            raise ValueError('max health migration conflicts with Linux value')
        migration['player_max_health'] = ('int', fields['H'])
    for name, feature_id in (('kDeflection', '4'), ('kSpeedIncrease', '5'), ('kProjectileDamageIncrease', '6'), ('kProjectileSpeedIncrease', '7')):
        if fields.get(name):
            if migration is None: migration = {}
            features = migration.get('gameplay_features_unlocked', ('vector', []))[1]
            if ('int', feature_id) not in features:
                migration['gameplay_features_unlocked'] = ('vector', features + [('int', feature_id)])
    if migration:
        for name, feature_id in (('kDeflection', '4'), ('kSpeedIncrease', '5'), ('kProjectileDamageIncrease', '6'), ('kProjectileSpeedIncrease', '7')):
            if not fields.get(name) and ('int', feature_id) in migration.get('gameplay_features_unlocked', ('vector', []))[1]:
                raise ValueError(name + ' migration conflicts with Linux unlock')
        if not fields.get('H') and 'player_max_health' in migration:
            raise ValueError('max health migration conflicts with Linux value')
        for node in ET.fromstring(field_xml(migration)):
            root.append(node)
    xml = ET.tostring(root, encoding='utf-8')
    wrapper = ET.Element('a', {'z': cipher(base64.b64encode(xml).decode())})
    return ET.tostring(wrapper, encoding='utf-8', xml_declaration=True) + b'\n', len(runtime)


def write_new(path, payload):
    target = Path(path)
    if target.exists(): raise ValueError('output already exists: ' + str(target))
    with tempfile.NamedTemporaryFile(dir=target.parent, prefix=target.name + '.', delete=False) as tmp:
        temp = Path(tmp.name)
        try:
            tmp.write(payload); tmp.flush(); os.fsync(tmp.fileno())
        except BaseException:
            temp.unlink(missing_ok=True); raise
    try: os.link(temp, target)
    finally: temp.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    actions = parser.add_subparsers(dest='action', required=True)
    imp = actions.add_parser('import')
    imp.add_argument('source'); imp.add_argument('output'); imp.add_argument('--level', required=True)
    imp.add_argument('--fresh-session', action='store_true', required=True,
                     help='acknowledge Windows runtime state/position is not persisted')
    exp = actions.add_parser('export')
    exp.add_argument('source'); exp.add_argument('output')
    exp.add_argument('--discard-runtime', action='store_true', required=True,
                     help='acknowledge Linux position, balance, orbs, cooldowns cannot export')
    args = parser.parse_args()
    try:
        if args.action == 'import':
            lines = windows_to_dl5(windows_fields(args.source), args.level)
            write_new(args.output, ('\n'.join(lines) + '\n').encode())
            print('Imported permanent subset into a fresh DL5 session; no Windows temporary state was present.')
        else:
            payload, count = dl5_to_windows(args.source)
            write_new(args.output, payload)
            print(f'Exported permanent subset; discarded session position and {count} Linux runtime lines.')
    except (ValueError, binascii.Error, ET.ParseError, OSError) as exc:
        parser.exit(2, f'save bridge: {exc}\n')


if __name__ == '__main__': main()
