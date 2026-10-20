#!/usr/bin/env python3
"""Rendered player-pose regression; run from repository root after building."""
import os
import struct
import subprocess
import tempfile
from pathlib import Path

BINARY = "port/build/deathleak-linux"
LEVEL = "Game/XmlFiles/levels/forest_exploration_1.xml"
CROP = (550, 360, 680, 540)  # player near center after camera settles


def capture(script, frame, destination):
    env = os.environ.copy()
    env.update(SDL_VIDEODRIVER="offscreen", SDL_RENDER_DRIVER="opengl", SDL_AUDIODRIVER="dummy")
    result = subprocess.run(
        [BINARY, "--play", LEVEL, "--script", script, "--fixed-dt", "0.0166667",
         "--frames", str(frame), "--capture", str(destination)],
        env=env, capture_output=True, text=True, check=True,
    )
    assert "accelerated=1 software=0" in result.stderr, result.stderr


def pixels(path):
    data = Path(path).read_bytes()
    assert data[:2] == b"BM"
    offset, width, height, bits = (struct.unpack_from("<I", data, 10)[0],
                                   struct.unpack_from("<i", data, 18)[0],
                                   struct.unpack_from("<i", data, 22)[0],
                                   struct.unpack_from("<H", data, 28)[0])
    assert bits == 32 and width == 1280 and abs(height) == 720
    stride = width * 4
    left, top, right, bottom = CROP
    rows = []
    for y in range(top, bottom):
        row = abs(height) - 1 - y if height > 0 else y
        start = offset + row * stride + left * 4
        rows.append(data[start:start + (right - left) * 4])
    return b"".join(rows)


def changed(a, b):
    return sum(a[i:i + 4] != b[i:i + 4] for i in range(0, len(a), 4))


with tempfile.TemporaryDirectory(prefix="deathleak-player-visual-") as tmp:
    paths = {}
    for name, script, frame in (("idle120", ".:120", 120), ("idle130", ".:130", 130),
                                ("melee127", ".:120,M:40", 127),
                                ("melee145", ".:120,M:40", 145)):
        path = Path(tmp) / (name + ".bmp")
        capture(script, frame, path)
        paths[name] = pixels(path)
    idle = changed(paths["idle120"], paths["idle130"])
    melee = changed(paths["melee127"], paths["melee145"])
    # Static-first-frame gameplay produced fewer than 700 changed pixels in
    # this region (background/orbs). Authored body animation changes thousands.
    assert idle > 1500, f"idle sequence did not visibly advance: {idle} pixels"
    assert melee > 1500, f"melee sequence did not visibly advance: {melee} pixels"
    print(f"PASS: accelerated player body animation idle={idle} melee={melee} changed pixels")
