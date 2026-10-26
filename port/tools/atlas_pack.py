#!/usr/bin/env python3
"""Deterministic, densely packed RGBA atlases with a 4096px page ceiling.

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


def intersects(a, b):
    return a[0] < b[0] + b[2] and b[0] < a[0] + a[2] and a[1] < b[1] + b[3] and b[1] < a[1] + a[3]


def contains(a, b):
    return a[0] <= b[0] and a[1] <= b[1] and a[0] + a[2] >= b[0] + b[2] and a[1] + a[3] >= b[1] + b[3]


def place(free, used):
    """Split intersected free rectangles, then remove contained duplicates."""
    remaining = []
    ux, uy, uw, uh = used
    for x, y, w, h in free:
        if not intersects((x, y, w, h), used):
            remaining.append((x, y, w, h))
            continue
        if ux > x:
            remaining.append((x, y, ux - x, h))
        if ux + uw < x + w:
            remaining.append((ux + uw, y, x + w - ux - uw, h))
        if uy > y:
            remaining.append((x, y, w, uy - y))
        if uy + uh < y + h:
            remaining.append((x, uy + uh, w, y + h - uy - uh))
    return [rect for i, rect in enumerate(remaining)
            if not any(i != j and contains(other, rect) for j, other in enumerate(remaining))]


def pack(paths, output):
    entries = []
    for path in sorted(set(paths), key=lambda p: p.as_posix()):
        with Image.open(path) as source:
            width, height = source.size
        if width + 2 * PAD > PAGE or height + 2 * PAD > PAGE:
            raise ValueError(f"too large for padded atlas: {path} ({width}x{height})")
        entries.append((path, width, height))
    entries.sort(key=lambda item: (-(item[1] + 2 * PAD) * (item[2] + 2 * PAD),
                                   -max(item[1], item[2]), item[0].as_posix()))
    pages = []
    placements = []
    for path, width, height in entries:
        outer_w, outer_h = width + 2 * PAD, height + 2 * PAD
        selected = None
        for index, page in enumerate(pages):
            for x, y, w, h in page['free']:
                if outer_w <= w and outer_h <= h:
                    score = (index, min(w - outer_w, h - outer_h), w * h - outer_w * outer_h, y, x)
                    if selected is None or score < selected[0]:
                        selected = (score, index, x, y)
        if selected is None:
            pages.append({'free': [(0, 0, PAGE, PAGE)], 'width': 0, 'height': 0, 'items': []})
            selected = ((len(pages) - 1, 0, 0, 0, 0), len(pages) - 1, 0, 0)
        _, index, x, y = selected
        page = pages[index]
        page['free'] = place(page['free'], (x, y, outer_w, outer_h))
        page['width'] = max(page['width'], x + outer_w)
        page['height'] = max(page['height'], y + outer_h)
        page['items'].append((path, x, y, width, height))
        placements.append({'source': path.as_posix(), 'sha256': digest(path),
                           'page': index, 'rect': [x + PAD, y + PAD, width, height]})
    output.mkdir(parents=True, exist_ok=True)
    page_names = []
    page_dimensions = []
    for index, page in enumerate(pages):
        image_page = Image.new('RGBA', (page['width'], page['height']))
        for path, x, y, width, height in page['items']:
            with Image.open(path) as source:
                image = source.convert('RGBA')
                image_page.paste(image, (x + PAD, y + PAD))
                image_page.paste(image.crop((0, 0, 1, height)).resize((PAD, height)), (x, y + PAD))
                image_page.paste(image.crop((width - 1, 0, width, height)).resize((PAD, height)), (x + PAD + width, y + PAD))
                image_page.paste(image.crop((0, 0, width, 1)).resize((width, PAD)), (x + PAD, y))
                image_page.paste(image.crop((0, height - 1, width, height)).resize((width, PAD)), (x + PAD, y + PAD + height))
                for cx, cy, sx, sy in ((x, y, 0, 0), (x + PAD + width, y, width - 1, 0),
                                       (x, y + PAD + height, 0, height - 1),
                                       (x + PAD + width, y + PAD + height, width - 1, height - 1)):
                    image_page.paste(image.getpixel((sx, sy)), (cx, cy, cx + PAD, cy + PAD))
        name = f'atlas_{index:03}.png'
        image_page.save(output / name, optimize=True)
        page_names.append(name)
        page_dimensions.append([page['width'], page['height']])
    manifest = {'format': 'deathleak-atlas-v2', 'max_page_size': PAGE, 'padding': PAD,
                'pages': page_names, 'page_dimensions': page_dimensions,
                'textures': sorted(placements, key=lambda p: p['source'])}
    (output / 'atlas_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    # The first runtime family intentionally accepts only a single page. Keep
    # its lightweight lookup file generated from the same verified placements.
    if len(page_names) == 1:
        (output / 'atlas_regions.tsv').write_text(''.join(
            f"{entry['source']}\t" + '\t'.join(map(str, entry['rect'])) + '\n'
            for entry in manifest['textures']))
    return manifest


def verify(manifest, output):
    pages = [Image.open(output / name).convert('RGBA') for name in manifest['pages']]
    try:
        for index, page in enumerate(pages):
            if list(page.size) != manifest['page_dimensions'][index]:
                raise ValueError(f"page dimensions mismatch: {manifest['pages'][index]}")
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
    print(f"packed and verified {len(manifest['textures'])} images into "
          f"{len(manifest['pages'])} page(s): {manifest['page_dimensions']}")


if __name__ == '__main__':
    main()
