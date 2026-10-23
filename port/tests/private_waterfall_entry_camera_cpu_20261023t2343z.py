#!/usr/bin/env python3
"""Source-anchored CPU discriminator for the Forest -> Waterfall entry camera."""
from pathlib import Path
import re
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[2]
source = (root / 'port/linux/main.cpp').read_text()
transition = source[source.index('nextScene.ApplyCollected('):source.index('motion.Reset(worldX, worldY, true); combat.ClearLevel();')]
assert re.search(r'worldX\s*=\s*scene\.PlayerX\(\);\s*worldY\s*=\s*scene\.PlayerY\(\);', transition)
assert re.search(r'cameraX\s*=\s*scene\.CameraFollowX\(\)\s*\?\s*worldX\s*:\s*scene\.CameraX\(\);', transition)
assert re.search(r'cameraY\s*=\s*scene\.CameraFollowY\(\)\s*\?\s*worldY\s*:\s*scene\.CameraY\(\);', transition)
assert source.index('FollowCameraWithTarget({') > source.index('nextScene.ApplyCollected(')

level = ET.parse(root / 'Game/XmlFiles/levels/waterfall_top.xml').getroot()
camera = level.find('./LevelProperties/camera_properties')
bounds = level.find('./LevelProperties/level_bounds')
entry = next(e for e in level.findall('LevelEntry') if e.find('level_came_from').get('value').replace('\\', '/').endswith('/forest_exploration_1.xml'))
player = level.find('Player')
assert camera.get('followx') == 'true' and camera.get('followy') == 'false'
entry_x = float(entry.find('position').get('x'))
xml_camera_x = float(camera.get('posx'))
assert entry_x < -2000 and xml_camera_x == 0
initial_x = entry_x if camera.get('followx') == 'true' else xml_camera_x
initial_y = float(entry.find('position').get('y')) if camera.get('followy') == 'true' else float(camera.get('posy'))
assert initial_x == entry_x and initial_y == float(camera.get('posy'))

# The first follow tick applies authored left bounds and then X lag; no
# permanent snap or bound mutation. Use measured packet player X at frame 424.
left = float(bounds.get('left'))
player_x_424 = -2139.01465
clamped_x = max(initial_x, left + 960.)
follow_x = clamped_x + (player_x_424 - clamped_x) / float(camera.get('xLag'))
assert clamped_x == -1340 and player_x_424 < follow_x < clamped_x
width = float(player.find('dimensions').get('width'))
scale = 2. / 3.
body_left = 640. + ((player_x_424 - follow_x) - width / 2.) * scale
body_right = body_left + width * scale
old_camera_424 = -1082.38098
old_body_right = 640. + ((player_x_424 - old_camera_424) + width / 2.) * scale
assert 0 < body_left < body_right < 1280
assert old_body_right < 0
print(f'PASS entry snap={initial_x:.3f}, Y preserved={initial_y:.1f}, bound={clamped_x:.1f}, lagged camera={follow_x:.3f}, body X={body_left:.1f}..{body_right:.1f}; old right={old_body_right:.1f}')
