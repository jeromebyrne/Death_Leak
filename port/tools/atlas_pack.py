#!/usr/bin/env python3
"""Deterministic RGBA atlases with per-sprite transparent-margin trimming.

A two-pixel transparent guard is retained where available for filtering, then
two edge pixels are extruded. Original size and trim offset restore placement.
Repeating textures and shader paths require virtual-UV handling before use.
"""
import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

PAGE = 4096
PAD = 2
GUARD = 2


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


def pack(paths, output, full_rgba=False):
    entries = []
    for path in sorted(set(paths), key=lambda p: p.as_posix()):
        with Image.open(path) as source:
            image = source.convert('RGBA')
            width, height = image.size
            # Invisible RGB bytes are not sampled by the alpha-blended sprite.
            bounds = (0, 0, width, height) if full_rgba else (image.getchannel('A').getbbox() or (0, 0, 1, 1))
            left = max(0, bounds[0] - (0 if full_rgba else GUARD))
            top = max(0, bounds[1] - (0 if full_rgba else GUARD))
            right = min(width, bounds[2] + (0 if full_rgba else GUARD))
            bottom = min(height, bounds[3] + (0 if full_rgba else GUARD))
            trim = (left, top, right - left, bottom - top)
        if trim[2] + 2 * PAD > PAGE or trim[3] + 2 * PAD > PAGE:
            raise ValueError(f"too large for padded atlas: {path} ({trim[2]}x{trim[3]})")
        entries.append((path, width, height, trim))
    entries.sort(key=lambda item: (-(item[3][2] + 2 * PAD) * (item[3][3] + 2 * PAD),
                                   -max(item[3][2], item[3][3]), item[0].as_posix()))
    pages = []
    placements = []
    for path, original_w, original_h, trim in entries:
        left, top, width, height = trim
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
        page['items'].append((path, x, y, trim))
        placements.append({'source': path.as_posix(), 'sha256': digest(path),
                           'page': index, 'rect': [x + PAD, y + PAD, width, height],
                           'original_size': [original_w, original_h], 'trim_offset': [left, top]})
    output.mkdir(parents=True, exist_ok=True)
    page_names = []
    page_dimensions = []
    for index, page in enumerate(pages):
        image_page = Image.new('RGBA', (page['width'], page['height']))
        for path, x, y, trim in page['items']:
            left, top, width, height = trim
            with Image.open(path) as source:
                image = source.convert('RGBA').crop((left, top, left + width, top + height))
                image_page.paste(image, (x + PAD, y + PAD))
                image_page.paste(image.crop((0, 0, 1, height)).resize((PAD, height), resample=Image.Resampling.NEAREST), (x, y + PAD))
                image_page.paste(image.crop((width - 1, 0, width, height)).resize((PAD, height), resample=Image.Resampling.NEAREST), (x + PAD + width, y + PAD))
                image_page.paste(image.crop((0, 0, width, 1)).resize((width, PAD), resample=Image.Resampling.NEAREST), (x + PAD, y))
                image_page.paste(image.crop((0, height - 1, width, height)).resize((width, PAD), resample=Image.Resampling.NEAREST), (x + PAD, y + PAD + height))
                for cx, cy, sx, sy in ((x, y, 0, 0), (x + PAD + width, y, width - 1, 0),
                                       (x, y + PAD + height, 0, height - 1),
                                       (x + PAD + width, y + PAD + height, width - 1, height - 1)):
                    image_page.paste(image.getpixel((sx, sy)), (cx, cy, cx + PAD, cy + PAD))
        name = f'atlas_{index:03}.png'
        image_page.save(output / name, optimize=True)
        page_names.append(name)
        page_dimensions.append([page['width'], page['height']])
    manifest = {'format': 'deathleak-atlas-v3', 'max_page_size': PAGE, 'padding': PAD,
                'transparent_guard': 0 if full_rgba else GUARD, 'full_rgba': full_rgba,
                'pages': page_names, 'page_dimensions': page_dimensions,
                'textures': sorted(placements, key=lambda p: p['source'])}
    (output / 'atlas_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    # Preserve the legacy single-page layout; multi-page rows add page index.
    (output / 'atlas_regions.tsv').write_text(''.join(
        f"{entry['source']}\t" + '\t'.join(map(str,
            ([entry['page']] if len(page_names) > 1 else []) +
            entry['rect'] + entry['original_size'] + entry['trim_offset'])) + '\n'
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
                original = image.convert('RGBA')
                ow, oh = texture['original_size']
                left, top = texture['trim_offset']
                if original.size != (ow, oh):
                    raise ValueError(f"source dimensions changed: {source}")
                # The bilinear footprint can touch the two-pixel padding.
                # Verify actual edge texels, not just the source rectangle.
                page = pages[texture['page']]
                if x < PAD or y < PAD or x + width + PAD > page.width or y + height + PAD > page.height:
                    raise ValueError(f"missing atlas padding: {source}")
                top_edge = page.crop((x, y, x + width, y + 1))
                bottom_edge = page.crop((x, y + height - 1, x + width, y + height))
                left_edge = page.crop((x, y, x + 1, y + height))
                right_edge = page.crop((x + width - 1, y, x + width, y + height))
                for pad in range(1, PAD + 1):
                    if page.crop((x, y - pad, x + width, y - pad + 1)).tobytes() != top_edge.tobytes() or \
                       page.crop((x, y + height - 1 + pad, x + width, y + height + pad)).tobytes() != bottom_edge.tobytes() or \
                       page.crop((x - pad, y, x - pad + 1, y + height)).tobytes() != left_edge.tobytes() or \
                       page.crop((x + width - 1 + pad, y, x + width + pad, y + height)).tobytes() != right_edge.tobytes():
                        raise ValueError(f"non-exact atlas edge extrusion: {source}")
                reconstructed = Image.new('RGBA', (ow, oh))
                reconstructed.paste(pages[texture['page']].crop((x, y, x + width, y + height)), (left, top))
                # Hidden RGB in transparent source pixels may be nonzero; only
                # alpha and RGB of visible pixels are a rendering contract.
                original_bytes = original.tobytes()
                restored_bytes = reconstructed.tobytes()
                for offset in range(0, len(original_bytes), 4):
                    if manifest.get('full_rgba') and original_bytes[offset:offset+4] != restored_bytes[offset:offset+4]:
                        raise ValueError(f"RGBA-byte mismatch: {source}")
                    if original_bytes[offset + 3] != restored_bytes[offset + 3] or (
                        original_bytes[offset + 3] and
                        original_bytes[offset:offset + 3] != restored_bytes[offset:offset + 3]
                    ):
                        raise ValueError(f"visible-pixel mismatch: {source}")
    finally:
        for page in pages:
            page.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--full-rgba', action='store_true', help='retain all source RGBA bytes, including invisible RGB')
    parser.add_argument('png', nargs='+', type=Path)
    args = parser.parse_args()
    for path in args.png:
        if not path.is_file() or path.suffix.lower() != '.png':
            parser.error(f'not a PNG file: {path}')
    manifest = pack(args.png, args.output, args.full_rgba)
    verify(manifest, args.output)
    print(f"packed and verified {len(manifest['textures'])} images into "
          f"{len(manifest['pages'])} page(s): {manifest['page_dimensions']}")


if __name__ == '__main__':
    main()
