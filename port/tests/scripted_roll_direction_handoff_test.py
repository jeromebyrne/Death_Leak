"""Static call-site guard for scripted leftward Roll material spray.

The separate LevelScene CPU fixture proves a supplied -1 DTO emits at x-12.
This guard checks that main actually supplies that DTO during scripted playback.
"""
from pathlib import Path
import re
import unittest

MAIN = Path(__file__).resolve().parents[1] / "linux" / "main.cpp"


class ScriptedRollDirectionHandoffTest(unittest.TestCase):
    def test_left_script_direction_survives_roll_and_reaches_visual_update(self):
        source = MAIN.read_text()
        self.assertRegex(source, r"command == 'L'.*command == 'N'.*command == 'Q'.*command == 'Z'")
        self.assertIn("if (direction != 0) facing = movementDirection = direction < 0 ? -1 : 1;", source)
        self.assertIn("const bool roll = !script.empty() ? command == 'O'", source)

        update = source.index("motion.Update(scene, dt,")
        handoff = source.index("strafeVisual.movementDirection = movementDirection;", update)
        controller_only = source.index("if (script.empty()) strafeVisual =", update)
        visual = source.index("scene.UpdatePlayerVisual(dt,", controller_only)
        self.assertLess(update, handoff)
        self.assertLess(handoff, controller_only)
        self.assertLess(controller_only, visual)
        self.assertRegex(source[visual:visual + 330], r"playerEligibleFireAttempt, strafeVisual\)")
        # With L then O, the last horizontal direction remains -1. The DTO
        # assignment above must occur outside the interactive-only branch.
        movement_direction = 1
        for command in "LO":
            direction = int(command in "RBEY") - int(command in "LNQZ")
            if direction:
                movement_direction = -1 if direction < 0 else 1
        self.assertEqual(movement_direction, -1)


if __name__ == "__main__":
    unittest.main()
