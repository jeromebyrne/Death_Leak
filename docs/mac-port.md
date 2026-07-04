# Death Leak Mac Port

This port starts from a Mac-buildable scaffold while preserving the existing Windows Direct3D 10 project.

## Current Rule

Do not rewrite the shipped Windows path just to make Mac progress. Add platform-neutral interfaces first, then implement new Mac backends beside the existing Win32, Direct3D, XInput, Steam, and irrKlang code.

## First Build Target

`DeathLeakMacSmoke` is a small CMake target that validates the new platform, input, audio, renderer, filesystem, time, and Steam service contracts without linking any Windows-only dependency. It also exercises the first concrete Mac backends, `MacFileSystem`, `MacSdlPlatformApp`, `MacSdlInputSystem`, `MacSdlRenderer`, `MacNullAudioSystem`, and `MacNullSteamService`.

SDL2 is currently discovered through `pkg-config`.

The CMake build also compiles `DeathLeakPortableCore`, a first slice of existing legacy code that is now AppleClang-compatible:

- `Logger`
- `DataValue`
- `Timing`
- `Vector2`
- `Vector3`
- `Vector4`
- `XmlDocument`
- `XmlUtilities`
- vendored `tinyxml`
- `Settings`
- `SineWave`
- `SaveManager`
- `PlayerLevelManager`
- `FeatureUnlockManager`
- `LevelProperties`
- `Animation`
- `AnimationPart`
- `AnimationSequence`
- `AnimationSkeleton`
- `Material`
- `MaterialManager`
- `StringManager`
- `LevelObjectRecord`

The smoke app now builds as a macOS bundle at:

`build/mac/DeathLeakMacSmoke.app`

The bundle copies `Game/Media`, `Game/XmlFiles`, and `Game/ShaderFiles` into `Contents/Resources`, and `MacFileSystem` prefers that bundle resource tree before falling back to the repo checkout.

The frame cadence now runs through `GameLoopController`, which centralises the platform pump, audio update, Steam update, and fixed-step timing around injected services.

The smoke app reads the real settings XML and writes a temporary encrypted save file at:

`~/Library/Application Support/Death Leak/Saves/mac_smoke_save.xml`

Build it with:

```sh
cmake -S . -B build/mac
cmake --build build/mac --target DeathLeakMacSmoke
open build/mac/DeathLeakMacSmoke.app
```

The smoke target also writes a texture audit report to:

`build/mac/texture-report.txt`

Current unresolved asset references in that report are the test/demo files in `Game/XmlFiles/levels/test.xml` plus the saw references in the shipped level data.

## Next Steps

1. Move Windows startup code behind the same service interfaces.
2. Port the real game boot flow onto the Mac service layer instead of the smoke preview.
3. Keep splitting remaining platform-specific code out of the shared logic so gameplay files only see abstract services.
4. Clean up the remaining asset mismatches or add explicit compatibility aliases where the shipped content expects older names.
5. Replace the temporary smoke harness with the actual game loop once the portable service layer is complete enough to support it.
