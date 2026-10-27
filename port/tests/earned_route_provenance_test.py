#!/usr/bin/env python3
"""Audit ordinary authored pickups and conservation in integration route saves.

Run after integration.sh from the repository root. This audits earned-state
provenance, not real-time survival, physical input or GPU acceptance.
"""
from pathlib import Path
import xml.etree.ElementTree as ET

objects = {}
for filename, count, balance, keys in (
    ('earned-purchase-save.txt', 375, 0, set()),
    ('earned-deflect-save.txt', 383, 8, set()),
    ('earned-dojo-hut-save.txt', 388, 13, {'dojo', 'village_hut'}),
    ('earned-full-preboat.txt', 406, 31,
     {'granary', 'dojo', 'village_hut', 'has_repair_tools'}),
):
    lines = (Path('port/build') / filename).read_text().splitlines()
    assert lines[0] == 'DL5', filename
    ids = [x for x in lines if x.startswith('Game/XmlFiles/levels/') and '#' in x]
    assert len(ids) == len(set(ids)) == count, filename
    assert [x for x in lines if x.startswith('B:')] == [f'B:{balance}'], filename
    assert count - balance == 375, filename
    assert 'U:kDeflection' in lines, filename
    assert {x[2:] for x in lines if x.startswith('K:')} == keys, filename
    for pickup in ids:
        level, object_id = pickup.rsplit('#', 1)
        if level not in objects:
            objects[level] = {o.get('id'): o for o in ET.parse(level).getroot()}
        obj = objects[level].get(object_id)
        assert obj is not None and obj.tag == 'CurrencyOrb', pickup
    print(f'PASS: {filename}: {count} unique authored CurrencyOrb IDs, '
          f'balance {balance}, one 375-orb debit, {len(keys)} earned keys')
