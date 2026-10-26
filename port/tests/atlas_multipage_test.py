#!/usr/bin/env python3
"""Packer/runtime lookup format contract for legacy and multi-page atlases."""
import tempfile
from pathlib import Path
import sys

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import atlas_pack


def run():
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        atlas_pack.PAGE = 64
        sources = []
        for index in range(3):
            path = root / f"sprite_{index}.png"
            Image.new("RGBA", (40, 40), (index + 1, 20, 30, 255)).save(path)
            sources.append(path)
        multi = root / "multi"
        manifest = atlas_pack.pack(sources, multi, full_rgba=True)
        atlas_pack.verify(manifest, multi)
        assert len(manifest["pages"]) == 3
        for row in (multi / "atlas_regions.tsv").read_text().splitlines():
            fields = row.split("\t")
            assert len(fields) == 10
            entry = next(item for item in manifest["textures"] if item["source"] == fields[0])
            assert int(fields[1]) == entry["page"]
            assert list(map(int, fields[2:])) == entry["rect"] + entry["original_size"] + entry["trim_offset"]
        single = root / "single"
        manifest = atlas_pack.pack(sources[:1], single, full_rgba=True)
        atlas_pack.verify(manifest, single)
        assert len((single / "atlas_regions.tsv").read_text().splitlines()[0].split("\t")) == 9
    print("atlas multi-page lookup: PASS")


if __name__ == "__main__":
    run()
