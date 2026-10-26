#!/usr/bin/env python3
"""Deterministic lossless atlas for the audited ordinary cart/track Sprite family."""
import hashlib
import json
from pathlib import Path
from PIL import Image

SOURCES = ('Game/Media/objects/cart.png', 'Game/Media/objects/cart_dark.png',
           'Game/Media/cart_tracks.png')
PAD = 2
OUTPUT = Path('port/assets/cart_props')

def generate(output=OUTPUT):
    entries = []
    for name in SOURCES:
        path = Path(name)
        with Image.open(path) as image:
            rgba = image.convert('RGBA')
            entries.append((name, rgba.copy(), hashlib.sha256(path.read_bytes()).hexdigest()))
    # First two 512-square props side by side, track below; retain hidden RGB.
    positions = ((PAD, PAD), (512 + 3 * PAD, PAD), (PAD, 512 + 3 * PAD))
    width = max(x + image.width + PAD for (_, image, _), (x, y) in zip(entries, positions))
    height = max(y + image.height + PAD for (_, image, _), (x, y) in zip(entries, positions))
    assert width <= 4096 and height <= 4096
    page = Image.new('RGBA', (width, height))
    manifest = {'format': 'deathleak-atlas-v3', 'max_page_size': 4096,
                'padding': PAD, 'transparent_guard': 0,
                'pages': ['atlas_000.png'], 'page_dimensions': [[width, height]], 'textures': []}
    for (name, image, sha), (x, y) in zip(entries, positions):
        w, h = image.size
        page.paste(image, (x, y))
        page.paste(image.crop((0, 0, 1, h)).resize((PAD, h)), (x-PAD, y))
        page.paste(image.crop((w-1, 0, w, h)).resize((PAD, h)), (x+w, y))
        page.paste(image.crop((0, 0, w, 1)).resize((w, PAD)), (x, y-PAD))
        page.paste(image.crop((0, h-1, w, h)).resize((w, PAD)), (x, y+h))
        for px, py, sx, sy in ((x-PAD,y-PAD,0,0),(x+w,y-PAD,w-1,0),
                               (x-PAD,y+h,0,h-1),(x+w,y+h,w-1,h-1)):
            page.paste(image.getpixel((sx, sy)), (px, py, px+PAD, py+PAD))
        assert page.crop((x,y,x+w,y+h)).tobytes() == image.tobytes()
        manifest['textures'].append({'source': name, 'sha256': sha, 'page': 0,
                                     'rect': [x,y,w,h], 'original_size': [w,h], 'trim_offset': [0,0]})
    manifest['textures'].sort(key=lambda e: e['source'])
    output.mkdir(parents=True, exist_ok=True)
    page.save(output / 'atlas_000.png', optimize=True)
    (output / 'atlas_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    (output / 'atlas_regions.tsv').write_text(''.join(
        e['source'] + '\t' + '\t'.join(map(str, e['rect']+e['original_size']+e['trim_offset'])) + '\n'
        for e in manifest['textures']))
    print(f'exact-RGBA atlas: {width}x{height}, {len(entries)} sources')

if __name__ == '__main__':
    generate()
