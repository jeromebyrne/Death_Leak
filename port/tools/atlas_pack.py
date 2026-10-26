#!/usr/bin/env python3
"""Deterministic 4096px RGBA atlas builder for explicitly selected PNGs.

The manifest's rect is the unpadded source content; two extruded edge pixels
on every side protect linear filtering. Selection is explicit because repeating
textures and shader paths need virtual-UV handling before runtime integration.
"""
import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

PAGE = 4096
PAD = 2


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pack(paths, output):
    entries = []
    for path in sorted(set(paths), key=lambda p: p.as_posix()):
        with Image.open(path) as source:
            width, height = source.size
        if width + 2 * PAD > PAGE or height + 2 * PAD > PAGE:
            raise ValueError(f"too large for padded atlas: {path} ({width}x{height})")
        entries.append((path, width, height))
    entries.sort(key=lambda item: (-item[2], -item[1], item[0].as_posix()))
    pages = []
    placements = []
    for path, width, height in entries:
        outer_w, outer_h = width + 2 * PAD, height + 2 * PAD
        selected = None
        for page in pages:
            x, y, row_height = page['cursor']
            if x + outer_w > PAGE:
                x, y, row_height = 0, y + row_height, 0
            if y + outer_h <= PAGE:
                selected = (page, x, y, row_height)
                break
        if selected is None:
            page = {'image': Image.new('RGBA', (PAGE, PAGE)), 'cursor': (0, 0, 0)}
            pages.append(page)
            selected = (page, 0, 0, 0)
        page, x, y, row_height = selected
        with Image.open(path) as source:
            image = source.convert('RGBA')
            page['image'].paste(image, (x + PAD, y + PAD))
            page['image'].paste(image.crop((0, 0, 1, height)).resize((PAD, height)), (x, y + PAD))
            page['image'].paste(image.crop((width - 1, 0, width, height)).resize((PAD, height)), (x + PAD + width, y + PAD))
            page['image'].paste(image.crop((0, 0, width, 1)).resize((width, PAD)), (x + PAD, y))
            page['image'].paste(image.crop((0, height - 1, width, height)).resize((width, PAD)), (x + PAD, y + PAD + height))
            for cx, cy, sx, sy in ((x, y, 0, 0), (x + PAD + width, y, width - 1, 0),
                                   (x, y + PAD + height, 0, height - 1),
                                   (x + PAD + width, y + PAD + height, width - 1, height - 1)):
                page['image'].paste(image.getpixel((sx, sy)), (cx, cy, cx + PAD, cy + PAD))
        page['cursor'] = (x + outer_w, y, max(row_height, outer_h))
        placements.append({'source': path.as_posix(), 'sha256': digest(path),
                           'page': pages.index(page), 'rect': [x + PAD, y + PAD, width, height]})
    output.mkdir(parents=True, exist_ok=True)
    page_names = []
    for index, page in enumerate(pages):
        name = f'atlas_{index:03}.png'
        page['image'].save(output / name, optimize=True)
        page_names.append(name)
    manifest = {'format': 'deathleak-atlas-v1', 'page_size': PAGE, 'padding': PAD,
                'pages': page_names, 'textures': sorted(placements, key=lambda p: p['source'])}
    (output / 'atlas_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    return manifest


def verify(manifest, output):
    pages = [Image.open(output / name).convert('RGBA') for name in manifest['pages']]
    try:
        for texture in manifest['textures']:
            source = Path(texture['source'])
            if digest(source) != texture['sha256']:
                raise ValueError(f"source changed: {source}")
            x, y, width, height = texture['rect']
            with Image.open(source) as image:
                if pages[texture['page']].crop((x, y, x + width, y + height)).tobytes() != image.convert('RGBA').tobytes():
                    raise ValueError(f"pixel mismatch: {source}")
    finally:
        for page in pages:
            page.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('png', nargs='+', type=Path)
    args = parser.parse_args()
    for path in args.png:
        if not path.is_file() or path.suffix.lower() != '.png':
            parser.error(f'not a PNG file: {path}')
    manifest = pack(args.png, args.output)
    verify(manifest, args.output)
    print(f"packed and verified {len(manifest['textures'])} images into {len(manifest['pages'])} 4096x4096 page(s)")


if __name__ == '__main__':
    main()
