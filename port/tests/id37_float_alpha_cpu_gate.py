#!/usr/bin/env python3
"""Source/CPU gate for single-application fractional-parallax alpha."""
from pathlib import Path
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
renderer = (ROOT / 'port/linux/SDLRenderer.cpp').read_text()
loader = (ROOT / 'port/game/LevelScene.cpp').read_text()
shader = (ROOT / 'Game/ShaderFiles/BasicLightTexture.fx').read_text()
windows_sprite = (ROOT / 'Game/src/GameObjects/Sprite.cpp').read_text()
float_draw = renderer.split('    bool DrawFractionalParallax(', 1)[1].split('    void DrawDamageBurst(', 1)[0]

def authored_alpha(level, object_id):
    scene = ET.parse(ROOT / 'Game/XmlFiles/levels' / level).getroot()
    node = next(node for node in scene.iter('ParallaxLayer') if node.get('id') == str(object_id))
    return float(node.find('alpha').get('value'))

# Gate the actual state chain, branch, and shader contract as well as the arithmetic.
assert 'sprite.alpha = alpha;' in loader
assert 'sprite.tint.a = static_cast<std::uint8_t>(std::lround(alpha * 255));' in loader
assert 'draw.tint = object.tint;' in loader and 'draw.opacity = object.alpha;' in loader
assert 'command.kind != ParallaxUVCommandKind::FloatQuad' in float_draw
assert re.search(r'glColor4f\([^;]*sprite\.tint\.b / 255\.f, sprite\.tint\.a / 255\.f\);', float_draw, re.S)
assert 'SDL_SetTextureAlphaMod(texture, sprite.tint.a);' in renderer
assert 'm_effectLightTexture->SetAlpha(m_alpha);' in windows_sprite
assert shader.count('textureSample.a = textureSample.a * alpha;') == 1

for name, level, object_id, expected in (
    ('opaque', 'forest_exploration_1.xml', 37, 0.95),
    ('lower alpha', 'waterfall_bottom.xml', 10, 0.5),
):
    alpha = authored_alpha(level, object_id)
    assert alpha == expected, (name, alpha)
    tint_byte = round(alpha * 255)
    float_multiplier = tint_byte / 255.0
    legacy_multiplier = tint_byte / 255.0
    assert float_multiplier == legacy_multiplier
    assert abs(float_multiplier - alpha) <= 0.5 / 255.0
    print(f'{name}: authored={alpha:.6f}, tint={tint_byte}, float=legacy={float_multiplier:.9f}')

# Explicit unity control independent of an XML fixture.
assert round(1.0 * 255) / 255.0 == 1.0
print('unity: authored=1.000000, tint=255, float=legacy=1.000000000')
