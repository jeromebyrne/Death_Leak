#!/usr/bin/env python3
"""Isolated no-pixel connected Forest->Waterfall loading-fade gate.

Builds only into /tmp from a read-only source mount; no shared build output,
physical display, Xorg, or GPU. Requires the pre-existing local build image.
"""
from pathlib import Path
import subprocess
import tempfile

from private_loading_fade_cpu_regression_20261023 import verify

ROOT = Path(__file__).resolve().parents[2]
IMAGE = "deathleak-linux-build:bookworm"
SOURCE_FILES = [
    "port/linux/main.cpp", "port/linux/SDLSoundPlayer.cpp",
    "port/game/LevelScene.cpp", "port/game/Combat.cpp",
    "port/game/PlayerMotion.cpp", "port/game/Hud.cpp",
    "Game/tinyxml/tinyxml.cpp", "Game/tinyxml/tinyxmlerror.cpp",
    "Game/tinyxml/tinyxmlparser.cpp", "Game/tinyxml/tinystr.cpp",
]


def main():
    with tempfile.TemporaryDirectory(prefix="deathleak-fast-fade-") as output:
        mount = [
            "docker", "run", "--rm", "--network", "none",
            "--mount", f"type=bind,src={ROOT},dst=/src,readonly",
            "--mount", f"type=bind,src={output},dst=/out", "-w", "/src", IMAGE,
        ]
        compile_command = [
            *mount, "sh", "-c",
            "g++ -O1 -std=c++17 -DDEATHLEAK_TEST_LOADING_FADE_OBSERVER "
            "-include port/tests/private_fast_fade_observer_cpu_renderer_20261023t2255z.h "
            "-I. -Iport/compat $(pkg-config --cflags sdl2 libpng) "
            + " ".join(SOURCE_FILES)
            + " -o /out/fast-fade-observer $(pkg-config --libs sdl2 libpng)",
        ]
        subprocess.run(compile_command, check=True, timeout=180)
        run_command = [
            "docker", "run", "--rm", "--network", "none",
            "--mount", f"type=bind,src={ROOT},dst=/src,readonly",
            "--mount", f"type=bind,src={output},dst=/out",
            "-e", "SDL_AUDIODRIVER=dummy", "-w", "/src", IMAGE,
            "/out/fast-fade-observer", "--play",
            "Game/XmlFiles/levels/forest_exploration_1.xml", "--script", "B:1800",
            "--frames", "426", "--fixed-dt", "0.016666667",
            "--settings-file", "/out/settings.xml",
        ]
        result = subprocess.run(run_command, check=True, timeout=40,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        verify(result.stdout)


if __name__ == "__main__":
    main()
