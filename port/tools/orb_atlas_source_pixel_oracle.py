#!/usr/bin/env python3
"""Bounded float32 UV/source-pixel oracle for the rejected flipped orb page.

This is a coordinate/filter diagnostic, not a substitute for exact physical GL pixels.
"""
import argparse
import json
import struct
from pathlib import Path
from PIL import Image


def f(x):
    return struct.unpack('f', struct.pack('f', x))[0]


def uv_texel(local, span, page_span, offset):
    # SDL/GL atlas quad has normalized endpoint coordinates; interpolation uses
    # float32 here to expose the page-dimension precision change.
    a = f(f(offset) / f(page_span))
    b = f(f(offset + span) / f(page_span))
    return f(f(a + f(f(b - a) * f(local))) * f(page_span)) - .5


def bilinear(pixels, width, height, x, y):
    x = max(0., min(width - 1., x))
    y = max(0., min(height - 1., y))
    x0, y0 = int(x), int(y)
    x1, y1 = min(width - 1, x0 + 1), min(height - 1, y0 + 1)
    u, v = x - x0, y - y0
    p00, p10 = pixels[x0, y0], pixels[x1, y0]
    p01, p11 = pixels[x0, y1], pixels[x1, y1]
    return tuple(round((1-u)*(1-v)*p00[c] + u*(1-v)*p10[c] + (1-u)*v*p01[c] + u*v*p11[c]) for c in range(4))


def run(atlas_dir, samples):
    manifest = json.loads((atlas_dir / 'atlas_manifest.json').read_text())
    assert manifest['full_rgba'] and len(manifest['textures']) == 10
    page_w, page_h = manifest['page_dimensions'][0]
    page = Image.open(atlas_dir / manifest['pages'][0]).convert('RGBA')
    assert page.size == (page_w, page_h)
    summary = []
    for entry in manifest['textures']:
        source = Image.open(entry['source']).convert('RGBA')
        width, height = source.size
        x0, y0, w, h = entry['rect']
        assert (w, h) == source.size and entry['trim_offset'] == [0, 0]
        assert page.crop((x0, y0, x0+w, y0+h)).tobytes() == source.tobytes()
        src, atlas = source.load(), page.load()
        for phase in samples:
            # 167x255 -> 32x49 physical pixels, minified and vertically flipped.
            # Phase models camera/world subpixel placement at four visible sites.
            different, max_delta, max_texel_error = 0, 0, 0.
            for py in range(49):
                ly = 1. - f(f(py + .5 - phase) / f(49.))
                if not 0. <= ly <= 1.: continue
                for px in range(32):
                    lx = f(f(px + .5 - phase) / f(32.))
                    if not 0. <= lx <= 1.: continue
                    dx = uv_texel(lx, w, w, 0)
                    dy = uv_texel(ly, h, h, 0)
                    ax = uv_texel(lx, w, page_w, x0)
                    ay = uv_texel(ly, h, page_h, y0)
                    max_texel_error = max(max_texel_error, abs(ax-x0-dx), abs(ay-y0-dy))
                    a = bilinear(src, w, h, dx, dy)
                    b = bilinear(atlas, page_w, page_h, ax, ay)
                    if a != b:
                        different += 1
                        max_delta = max(max_delta, *(abs(a[c]-b[c]) for c in range(4)))
            summary.append({'frame': Path(entry['source']).name, 'phase': phase,
                            'rgba_mismatches': different, 'max_channel_delta': max_delta,
                            'max_source_texel_coordinate_delta': round(max_texel_error, 8)})
    return summary


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--atlas-dir', type=Path, required=True)
    args = parser.parse_args()
    result = run(args.atlas_dir, [0., .25, .5, .75])
    print(json.dumps(result, indent=2))
