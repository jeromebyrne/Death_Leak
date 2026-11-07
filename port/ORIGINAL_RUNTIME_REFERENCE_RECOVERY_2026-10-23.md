# Original runtime reference recovery — bounded local inventory (2026-10-23)

## Finding

**No original Windows Death Leak executable or matched forest-1 original-renderer gameplay frame/state can be recovered from the currently accessible project files.** This confirms the prerequisite identified in `WINDOWS_FOREST_MATCHED_FRAME_EXECUTION_2026-10-23.md`; it is not a new pixel-parity verdict. No similarly scoped *active* worker was visible in this child's session scope, but session visibility is tree-restricted, so activity outside that scope cannot be ruled out.

## Distinct evidence from this pass

- A read-only `find /srv/openclaw/workspace/death_leak_analysis -type f` search for `.exe`, `.zip`, `.7z`, `.rar`, `.msi`, `.iso`, and `.vhd*` found **no `.exe`**. The only matching archives were `steamworks_sdk_155/sdk/tools/{SteamPipeGUI,ContentPrep}.zip` and copies of those two SDK archives under Linux-port experiment `port/build/.../{fix-src,control-src,pre-fix-src}`. They are Steamworks SDK tool packages, not identified Death Leak game packages; archive payloads were not inspected, so this is a filename/path conclusion, not a complete archive-content proof.
- The ContentBuilder's top-level `windows_content/` has assets/runtime DLLs and `steam_appid.txt`, but no game EXE. Its available build scripts are `simple_app_build.vdf`, `app_build_1000.vdf`, `depot_build_1001.vdf`, and `depot_build_1002.vdf`. `app_build_1000.vdf` explicitly uses placeholder AppID `1000`, “Your build description here,” and depots `1001`/`1002`; `depot_build_1001.vdf` calls `1001` “your assigned depot ID.” These are SDK example scripts, **not** evidence of a staged app-995990 release executable or a recoverable release depot.
- `../store_media/` contains store JPEG screenshots and a 30-second trailer/contact-frame set; the previously established closest forest-1 reference is `screenshots/14.jpg` (`MATCHED_FOREST_REFERENCE_SEARCH_2026-10-22.md`). That JPEG lacks the original camera/player/effect-clock sidecar and is compressed; the trailer has no matching forest-1 scene. Neither is a synchronized original gameplay frame/state.

## Limit and next action

The scan covers this project tree and documented adjacent `store_media`, not a user's separate Steam library, Windows machine, or online entitlement. Prior reports already document the absent local Wine/Windows runtime and the original-source build dependencies; those checks were not repeated here. **Next:** on a Windows D3D10-capable machine with authorized access to Steam app 995990, first verify that the *game* EXE exists and record its SHA-256 plus content identity. Then use the established `FOREST1_MATCHED_FRAME_CAPTURE_PACKET_2026-10-23.md` contract to capture one native lossless post-HUD forest-1 frame and synchronized same-render-ordinal state sidecar. If only a screenshot is possible, label it qualitative, not matched-state evidence.

Only this report was written. No source, build, GPU/Xorg, commit, or shared status file was touched.
