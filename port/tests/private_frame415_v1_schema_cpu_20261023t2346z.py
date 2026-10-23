#!/usr/bin/env python3
"""CPU-only regression for the saved frame-415 v1 packet contract."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / 'port/build/private-watchdog-linux-frame415-atomic-20261023t2322z/frame415.json'
REGRESSED = ROOT / 'port/build/private-watchdog-postqa-frame-packet-gpu-20261023t2340z/frame415.json'
POSTFADE = ROOT / 'port/build/private-waterfall-entry-camera-fix-20261023t2343z/frame424.json'
SOURCE = ROOT / 'port/linux/main.cpp'
EXTRA = {'firstPostfadeOrdinal', 'loadingSplashSubmitted'}


def shape(value):
    if isinstance(value, dict):
        return {key: shape(item) for key, item in value.items()}
    if isinstance(value, list):
        return ('array', tuple(shape(item) for item in value))
    return type(value).__name__


base, regressed, postfade = [json.loads(p.read_text()) for p in (BASE, REGRESSED, POSTFADE)]
assert base['schema'] == regressed['schema'] == 'deathleak-linux-frame415-v1'
assert postfade['schema'] == 'deathleak-linux-postfade-waterfall-v1'
assert EXTRA.isdisjoint(base['transition'])
assert set(regressed['transition']) - set(base['transition']) == EXTRA
assert set(base['transition']) - set(regressed['transition']) == set()
assert type(regressed['transition']['firstPostfadeOrdinal']) is int
assert type(regressed['transition']['loadingSplashSubmitted']) is bool
normalized = json.loads(json.dumps(regressed))
for key in EXTRA:
    del normalized['transition'][key]
assert shape(normalized) == shape(base), 'a difference beyond the two transition keys exists'
assert 'preHudImage' not in base and 'preHudImage' not in regressed
assert 'sceneDrawIdentifierScope' not in base['dynamicDraw']
assert 'sceneDrawIdentifierScope' not in regressed['dynamicDraw']
assert isinstance(base['dynamicDraw']['orderedSceneDraws'], dict)
assert isinstance(regressed['dynamicDraw']['orderedSceneDraws'], dict)
assert type(postfade['transition']['firstPostfadeOrdinal']) is int
assert type(postfade['transition']['loadingSplashSubmitted']) is bool
assert isinstance(postfade['preHudImage'], dict)
assert isinstance(postfade['dynamicDraw']['sceneDrawIdentifierScope'], str)
assert isinstance(postfade['dynamicDraw']['orderedSceneDraws'], list)

# Source seam assertion: the only writes of these two JSON members are both
# within the postfadePacket branch before common transition fields resume.
source = SOURCE.read_text()
start = source.index('<< "  \\"transition\\": {\\"activeLevel\\": "')
end = source.index('<< ", \\"loadingPhaseAtDraw\\": "', start)
seam = source[start:end]
assert seam.count('if (postfadePacket)') == 1
assert seam.count('"firstPostfadeOrdinal\\": "') == 1
assert seam.count('"loadingSplashSubmitted\\": "') == 1
branch = seam.index('if (postfadePacket)')
first = seam.index('firstPostfadeOrdinal', branch)
second = seam.index('loadingSplashSubmitted', first)
common = seam.index('packet << ", \\"loadingPhaseOnEntry\\": "', second)
assert branch < first < second < common
assert source.count('"firstPostfadeOrdinal\\": "') == 1
assert source.count('"loadingSplashSubmitted\\": "') == 1
print('PASS: frame-415 saved-v1 shape and postfade-only serializer seam')
