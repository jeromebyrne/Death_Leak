# Death Leak Mac Port

This port starts from a Mac-buildable scaffold while preserving the existing Windows Direct3D 10 project.

## Current Rule

Do not rewrite the shipped Windows path just to make Mac progress. Add platform-neutral interfaces first, then implement new Mac backends beside the existing Win32, Direct3D, XInput, Steam, and irrKlang code.

## First Build Target

`DeathLeakMacSmoke` is a small CMake target that validates the new platform, input, audio, renderer, filesystem, time, and Steam service contracts without linking any Windows-only dependency. It also exercises the first concrete Mac backends, `MacFileSystem`, `MacSdlPlatformApp`, `MacSdlInputSystem`, and `MacSdlRenderer`.

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

Build it with:

```sh
cmake -S . -B build/mac
cmake --build build/mac --target DeathLeakMacSmoke
./build/mac/DeathLeakMacSmoke
```

## Next Steps

1. Move Windows startup code behind the same service interfaces.
2. Add a real macOS platform backend, likely SDL-based.
3. Add a stub renderer that can boot the game loop before porting sprite rendering.
4. Compile gameplay files in batches and remove Windows/D3D dependencies as each batch exposes them.
