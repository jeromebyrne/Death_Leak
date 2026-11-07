# Original-Windows forest-1 bridge frame: local feasibility (2026-10-22)

## Verdict

**Not acquirable from the current local materials/tools.** A lossless, state-matched *original Windows renderer* frame is absent, and there is no runnable Death Leak Windows EXE or Wine runtime in this execution environment. The assets and source make the scene and a future capture seam identifiable, but cannot produce pixels from the shipped Windows executable by themselves. A newly built Windows executable could be a useful **source-build** reference, not proof of the shipped binary's exact output. The historical Steam screenshot 14 is a lossy, state-unknown JPEG and cannot be made into a matched frame by guessing its camera.

## Read-only checks (from repository root)

```sh
find /srv/openclaw/workspace/death_leak_analysis -type f \( -iname '*.exe' -o -iname '*.pdb' \) -print
# no output; Game/x64/Debug has only a DeathLeakGame.exe.recipe, and x64/Release has .iobj/.ipdb, not an EXE/PDB
for x in wine wine64 wineserver Xvfb ffmpeg x86_64-w64-mingw32-g++ clang-cl msbuild dotnet; do command -v "$x" || true; done
# no output for these commands (python3 and docker do exist)
docker image ls --format '{{.Repository}}:{{.Tag}}' | grep -Ei 'wine|windows|proton|steam|deathleak'
# deathleak-capture:post-foliage, deathleak-video-capture:bookworm,
# deathleak-linux-build:bookworm, cm2network/steamcmd:latest; no Wine/Proton image identified
```

The `steamworks_sdk_155/sdk/tools/ContentBuilder/content/windows_content/` tree contains XML, textures, shaders and DLLs, but no EXE. `Game/steam_api64.dll`, `irrKlang.dll`, `ikpMP3.dll` and `D3DX10d_43.dll` are dependencies, not the game program. `Game/Game.vcxproj` declares v143, Windows SDK 10.0.19041.0 and D3D9/D3D10/D3DX link dependencies; those are not installed build tools here. The retained `.exe.recipe`, `.iobj` and `.ipdb` are build artifacts, not a runnable executable. A cached SteamCMD image is not evidence that the app depot or a runnable game is installed; no network/download or build attempt was made.

`Game/src/UI/UIQuickPlayScreen.cpp:60` selects `XmlFiles\\levels\\forest_exploration_1.xml`; `Game/src/Core/Graphics.cpp:257-260` presents the swap chain in `Graphics::SwapBuffers`. A backbuffer read immediately before Present is a plausible lossless capture seam **once a runnable Windows build exists**. No D3D10-under-Wine compatibility, display/headless capture, or backbuffer readback has been demonstrated locally, so having Docker alone is not a proved capture setup. The existing `port/build/forest14-linux-match-2026-10-22/linux-r110.png` is a Linux renderer capture, not a Windows bridge image.

The existing `port/WINDOWS_FOREST_FRAME_ROUTE_2026-10-22.md`, `port/WINDOWS_FOREST_MATCH_ROUTE_2026-10-22.md` and `port/FOREST14_LINUX_MATCH_CAPTURE_2026-10-22.md` already establish screenshot 14's landmark identity and its JPEG/state limitations. This check did not repeat speculative screenshot matching. No saved lossless original-Windows forest-1 frame with a paired state record was found in the project materials.

## Same-frame sidecar feasibility

**Feasible in an instrumented Windows source build; not currently evidenced for the shipped binary.** Log the effective rendered camera transform/position (including shake and follow lag), viewport/backbuffer dimensions, player pose/velocity, health/orbs and relevant object IDs, simulation/shader time, animation/weather/particle phase or RNG state, EXE and asset hashes, and capture stage/color format. Associate the sidecar with the image by a common rendered-frame index and image hash, written during the same frame. `Camera2D::Position()` exists in source, but player X, XML initial camera, or an external screen grab cannot substitute for the effective render transform. A shipped EXE without symbols/instrumentation would need a validated external capture-and-state extraction method; no such method is present. Capturing a PNG without the sidecar would be lossless visually but still **not state-matched**.

## Next bounded step

Acquire an authorized, hashable **shipped Death Leak Windows EXE/depot for app 995990** (preferred), or separately identify a Windows machine with the declared VS/SDK dependencies for a clearly labeled source-build reference. First do a small smoke test: launch forest-1, confirm Direct3D rendering and a lossless native-size backbuffer readback, and prove one image plus same-frame sidecar. If using Wine, install/identify Wine and test D3D10 and the display/capture path before planning a full comparison. Only after that proof, reproduce the recorded camera/state in the Linux port and compare static landmarks. Do not claim recovery of the exact historic Steam JPEG moment.

Scope: read-only inspection only; no source, tests, build, GPU, status files, network download, commit or push changed for this audit.
