#!/usr/bin/env python3
"""Connected, deterministic Forest-1 -> Waterfall loading-fade regression."""

import argparse
import re
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
IMAGE = "deathleak-linux-build:bookworm"
ROW = re.compile(
    r"FADE_OBSERVER frame=(\d+) entry=(\d+) exit=(\d+) gameplay=(\d+) "
    r"weather=(\d+) scene=(\d+) player=(\d+) alpha=(-?\d+) "
    r"hold_exit=(\d+) time=([\d.e+-]+)"
)


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def run(binary=None):
    with tempfile.TemporaryDirectory(prefix="loading-fade-cpu-", dir=ROOT / "port/build") as tmp:
        output = Path(tmp)
        if binary is None:
            build = [
                "docker", "run", "--rm", "--network", "host",
                "--mount", f"type=bind,src={ROOT},dst=/src,readonly",
                "--mount", f"type=bind,src={output},dst=/out",
                "-w", "/src", IMAGE, "make", "-e", "-C", "port",
                "TARGET=/out/deathleak-observer",
                "CXXFLAGS=-O2 -g -std=c++17 -Wall -Wextra -Wpedantic "
                "-DDEATHLEAK_TEST_LOADING_FADE_OBSERVER",
            ]
            subprocess.run(build, check=True)
        else:
            shutil.copy2(Path(binary), output / "deathleak-observer")
        command = [
            "docker", "run", "--rm", "--network", "host",
            "--mount", f"type=bind,src={ROOT},dst=/src,readonly",
            "--mount", f"type=bind,src={output},dst=/out,readonly",
            "-e", "SDL_VIDEODRIVER=dummy",
            "-e", "SDL_RENDER_DRIVER=software",
            "-e", "SDL_AUDIODRIVER=dummy",
            "-e", "DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1",
            "-w", "/src", IMAGE, "/out/deathleak-observer",
            "--play", "Game/XmlFiles/levels/forest_exploration_1.xml",
            "--script", "B:1800", "--frames", "426",
            "--fixed-dt", "0.016666667",
        ]
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, check=True)
    verify(result.stdout)


def verify(text):
    require("Trigger at frame 398" in text, "authored Forest trigger did not fire")
    require(text.count("Entered Game/XmlFiles/levels/waterfall_top.xml") == 1,
            "destination scene was not installed exactly once")
    rows = []
    for line in text.splitlines():
        match = ROW.fullmatch(line)
        if match:
            values = [int(value) for value in match.groups()[:-1]]
            rows.append((*values, float(match.group(10))))
    require(rows, "passive production-path observer emitted no frames")
    by_frame = {row[0]: row for row in rows}
    require(len(by_frame) == len(rows), "duplicate observer frame")
    # Phases: None=0, FadeIn=1, OpaqueHold=2, FadeOut=3.
    start = next((row for row in rows if row[1:3] == (0, 1)), None)
    require(start is not None and start[3:7] == (1, 1, 1, 1),
            f"trigger-start gameplay did not finish: {start}")
    for row in rows:
        if row[1] in (1, 2):
            require(row[3:7] == (0, 0, 0, 0),
                    f"loading entry did not pause gameplay/visuals: {row}")
    install = next((row for row in rows if row[1:3] == (1, 2)), None)
    require(install is not None and install[7] == 255,
            f"destination installation was not opaque: {install}")
    before_install = by_frame.get(install[0] - 1)
    require(before_install is not None and before_install[1:3] == (1, 1)
            and before_install[7] == 255,
            f"FadeIn did not respect the strict >0.20 boundary: {before_install}")
    exit_hold = next((row for row in rows if row[1:3] == (2, 3)), None)
    require(exit_hold is not None and exit_hold[8] == 1 and exit_hold[7] == 255,
            f"hold-exit draw was not opaque: {exit_hold}")
    require(exit_hold[3:7] == (0, 0, 0, 0),
            f"hold-exit frame resumed gameplay too soon: {exit_hold}")
    before_exit = by_frame.get(exit_hold[0] - 1)
    require(before_exit is not None and before_exit[1:3] == (2, 2)
            and before_exit[7] == 255,
            f"hold did not remain opaque until its strict boundary: {before_exit}")
    first_fade_out = by_frame.get(exit_hold[0] + 1)
    require(first_fade_out is not None and first_fade_out[1] == 3
            and first_fade_out[3:7] == (1, 1, 1, 1)
            and 0 <= first_fade_out[7] < 255
            and first_fade_out[9] > exit_hold[9],
            f"FadeOut did not advance gameplay under the revealing splash: {first_fade_out}")
    fading = [row for row in rows if row[1] == 3]
    require(fading and all(row[3:7] == (1, 1, 1, 1) for row in fading),
            f"FadeOut gameplay/visual update gap: {fading}")
    retired = next((row for row in fading if row[2] == 0), None)
    require(retired is not None and retired[7] == -1 and retired[3:7] == (1, 1, 1, 1),
            f"FadeOut retirement did not retain gameplay and remove splash: {retired}")
    before_retirement = by_frame.get(retired[0] - 1)
    require(before_retirement is not None and before_retirement[1:3] == (3, 3)
            and before_retirement[7] == 0,
            f"FadeOut retired before its strict >0.20 boundary: {before_retirement}")
    print(f"PASS connected loading fade: hold-exit frame {exit_hold[0]}, "
          f"first gameplay FadeOut frame {first_fade_out[0]}, "
          f"retirement frame {retired[0]}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--observer-binary", type=Path)
    parser.add_argument("--observer-log", type=Path)
    args = parser.parse_args()
    if args.observer_log:
        verify(args.observer_log.read_text())
    else:
        run(args.observer_binary)
