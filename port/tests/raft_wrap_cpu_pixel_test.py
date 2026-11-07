"""CPU-only source-state and BOX-asset oracle for the private raft mip draw.

BOX texels are a deterministic fixture, not a model of driver-generated mipmaps.
No SDL/GL context, renderer, game process, or display is used here.
"""

import hashlib
import math
from pathlib import Path
import re
import unittest
import xml.etree.ElementTree as ET

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
RAFT = ROOT / "Game/Media/objects/raft.png"
EXPECTED_SHA256 = "d319f5d1cc292565df20111a67529feb8b4ff3434ff2176fa3d7ddf5f420d031"


def box_pyramid(image, last_level=4):
    levels = [image]
    for _ in range(last_level):
        width, height = levels[-1].size
        levels.append(levels[-1].resize((max(1, width // 2), max(1, height // 2)), Image.Resampling.BOX))
    return levels


def sample(image, u, v, address="WRAP", filtering="linear"):
    """Normalized-coordinate nearest or bilinear RGBA texel sample in floats."""
    if address not in ("WRAP", "CLAMP") or filtering not in ("linear", "nearest"):
        raise ValueError("unsupported CPU oracle mode")
    width, height = image.size

    def index(i, length):
        return i % length if address == "WRAP" else min(length - 1, max(0, i))

    if filtering == "nearest":
        return tuple(float(c) for c in image.getpixel((index(math.floor(u * width), width),
                                                        index(math.floor(v * height), height))))

    x, y = u * width - 0.5, v * height - 0.5
    x0, y0 = math.floor(x), math.floor(y)
    fx, fy = x - x0, y - y0
    pixels = [
        (image.getpixel((index(ix, width), index(iy, height))), weight)
        for ix, iy, weight in ((x0, y0, (1-fx)*(1-fy)),
                               (x0+1, y0, fx*(1-fy)),
                               (x0, y0+1, (1-fx)*fy),
                               (x0+1, y0+1, fx*fy))
    ]
    return tuple(sum(pixel[channel] * weight for pixel, weight in pixels)
                 for channel in range(4))


def over_opaque(rgba, background=(20.0, 100.0, 210.0)):
    alpha = rgba[3] / 255.0
    return tuple(rgba[i] * alpha + background[i] * (1-alpha) for i in range(3))


class RaftPixelOracle(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assert_asset_hash = hashlib.sha256(RAFT.read_bytes()).hexdigest()
        if cls.assert_asset_hash != EXPECTED_SHA256:
            raise AssertionError("raft asset hash changed; numerical BOX probes need revalidation")
        cls.levels = box_pyramid(Image.open(RAFT).convert("RGBA"))

    def test_forest_raft_objects_and_flip_edge_mapping(self):
        root = ET.parse(ROOT / "Game/XmlFiles/levels/forest_exploration_1.xml").getroot()
        sprites = {int(node.attrib["id"]): node for node in root.iter("Sprite")}
        for sprite_id, flipped in ((11, True), (57, True), (58, False)):
            with self.subTest(sprite_id=sprite_id):
                node = sprites[sprite_id]
                self.assertEqual(node.find("texture").attrib["filename"], "Media/objects/raft.png")
                self.assertEqual(node.find("effect").attrib["name"], "effectlighttexture")
                dims = node.find("dimensions").attrib
                self.assertEqual((dims["repeatX"], dims["repeatY"]), ("false", "false"))
                self.assertEqual((float(dims["width"]), float(dims["height"])),
                                 (458.298584, 125.316017))
                self.assertEqual(float(node.find("alpha").attrib["value"]), 1.0)
                self.assertEqual(node.find("horizontalflip").attrib["value"], str(flipped).lower())
                self.assertEqual(node.find("verticalflip").attrib["value"], "false")
                # DrawRaftMipExperiment maps the physical left/right quad edges this way.
                self.assertEqual((int(flipped), 1-int(flipped)),
                                 (1, 0) if sprite_id in (11, 57) else (0, 1))
        self.assertEqual(sprites[14].find("texture").attrib["filename"], "Media/sun_shaft_2.png")

    def test_synthetic_two_axis_address_and_nearest_controls(self):
        image = Image.new("RGBA", (2, 2))
        image.putdata([(255, 0, 0, 255), (0, 255, 0, 255),
                       (0, 0, 255, 255), (255, 255, 0, 255)])
        for u, v in ((0, .25), (1, .25), (.25, 0), (.25, 1)):
            with self.subTest(u=u, v=v):
                self.assertNotEqual(sample(image, u, v, "WRAP"),
                                    sample(image, u, v, "CLAMP"))
        for uv in ((.25, .25), (.75, .75)):
            self.assertEqual(sample(image, *uv, "WRAP"), sample(image, *uv, "CLAMP"))
        self.assertEqual(sample(image, .25, .25, "WRAP", "nearest"), (255., 0., 0., 255.))

    def test_raft_box_mip_edge_bands_and_interior(self):
        self.assertEqual([im.size for im in self.levels],
                         [(1024, 280), (512, 140), (256, 70), (128, 35), (64, 17)])
        for level, image in enumerate(self.levels):
            width, height = image.size
            for axis in ("u", "v"):
                # Texel centers are interior controls; the half-texel bands alone
                # are address-sensitive for normalized coordinates within [0,1].
                for coordinate, should_differ in ((0, True), (1/(4*(width if axis == "u" else height)), True),
                                                  (1/(2*(width if axis == "u" else height)), False),
                                                  (.5, False),
                                                  (1-1/(2*(width if axis == "u" else height)), False),
                                                  (1-1/(4*(width if axis == "u" else height)), True),
                                                  (1, True)):
                    u, v = (coordinate, 4.5/height) if axis == "u" else (.5, coordinate)
                    a, b = sample(image, u, v, "CLAMP"), sample(image, u, v, "WRAP")
                    with self.subTest(level=level, axis=axis, coordinate=coordinate):
                        if not should_differ:
                            self.assertEqual(a, b)
                        else:
                            length = width if axis == "u" else height
                            first_uv = (.5/width, v) if axis == "u" else (u, .5/height)
                            last_uv = (1-.5/width, v) if axis == "u" else (u, 1-.5/height)
                            first = sample(image, *first_uv, "CLAMP")
                            last = sample(image, *last_uv, "CLAMP")
                            position = coordinate * length - .5
                            if position < 0:
                                expected_clamp = first
                                expected_wrap = tuple(first[i]*(position+1) - last[i]*position
                                                      for i in range(4))
                            else:
                                expected_clamp = last
                                expected_wrap = tuple(last[i]*(length-position) +
                                                      first[i]*(position-length+1)
                                                      for i in range(4))
                            for channel in range(4):
                                self.assertAlmostEqual(a[channel], expected_clamp[channel], places=9)
                                self.assertAlmostEqual(b[channel], expected_wrap[channel], places=9)
                        # Equal edge texels can hide a real address-mode difference.
            for u in (0, 1):
                v = (4.5 if level == 3 else 2.5 if level == 4 else .5) / height
                clamp, wrap = sample(image, u, v, "CLAMP"), sample(image, u, v, "WRAP")
                with self.subTest(level=level, u=u):
                    if level <= 2:
                        self.assertEqual((clamp[3], wrap[3]), (0.0, 0.0))
                        self.assertEqual(over_opaque(clamp), over_opaque(wrap))
                    else:
                        self.assertNotEqual(over_opaque(clamp), over_opaque(wrap))

    def test_exact_box_edge_probes(self):
        for level, row, first, half in ((3, 4, (25., 25., 25., 10.), (12.5, 12.5, 12.5, 5.)),
                                        (4, 2, (8., 8., 8., 62.), (4., 4., 4., 31.))):
            image = self.levels[level]
            width, height = image.size
            v = (row + .5) / height
            self.assertEqual(sample(image, 0, v, "CLAMP"), first)
            self.assertEqual(sample(image, 0, v, "WRAP"), half)
            self.assertEqual(sample(image, 1, v, "CLAMP"), (0., 0., 0., 0.))
            self.assertEqual(sample(image, 1, v, "WRAP"), half)
            self.assertEqual(sample(image, .5/width, v, "CLAMP"), first)
            self.assertEqual(sample(image, .5/width, v, "WRAP"), first)


class RaftDrawSiteSourceState(unittest.TestCase):
    def test_private_draw_sets_repeat_on_both_axes(self):
        source = (ROOT / "port/linux/SDLRenderer.cpp").read_text()
        begin = source.index("bool DrawRaftMipExperiment(")
        end = source.index("DrawFractionalParallax", begin)
        method = source[begin:end]
        bound = method.index("glBindTexture(GL_TEXTURE_2D, name)")
        quad = method.index("glBegin(GL_QUADS)", bound)
        draw_state = method[bound:quad]
        for axis in ("S", "T"):
            assignments = re.findall(
                rf"glTexParameteri\s*\(\s*GL_TEXTURE_2D\s*,\s*GL_TEXTURE_WRAP_{axis}\s*,\s*(GL_\w+)\s*\)\s*;",
                draw_state)
            self.assertEqual(assignments, ["GL_REPEAT"], f"draw-site WRAP_{axis} must be GL_REPEAT")
        self.assertNotIn("GL_CLAMP_TO_EDGE", draw_state)


if __name__ == "__main__":
    unittest.main()
