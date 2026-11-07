# Matched original-runtime discriminator — 2026-10-24 00:12Z

## Decision

**Blocked at executable access, before capture.** There is no currently reachable Windows/D3D10 node and no identified runnable original Death Leak executable in the accessible tree. This is a host/access blocker, not a renderer parity finding. A screenshot, Steam JPEG, Linux BMP/MP4, or SDK tool EXE cannot satisfy the original same-render-ordinal packet.

## Fresh discriminating check

- A live `openclaw nodes list --json` returned `{"pending":[],"paired":[]}`; the subsequent `openclaw nodes status --json` returned `{"ts":1792800769078,"nodes":[]}`. Thus there is no node ID to target with a Windows capture command now. This checks the Gateway reachable by this session, not every privately owned host.
- Prior inventory stopped at archive *names*. I inspected the central Steamworks SDK ZIP **member lists without extracting or executing them**. `steamworks_sdk_155/sdk/tools/SteamPipeGUI.zip` has two members and its sole EXE is `SteamPipeGUI.exe`; `ContentPrep.zip` has eight members and no EXE. The identical SDK archives copied under `port/build` have the same result. No member name includes Death Leak or app 995990. The one archived Windows EXE is a Steam depot GUI utility, not the game, so unzipping the known archive cannot produce a candidate original runtime. This resolves the previous archive-content caveat, within filename/member-list scope.
- `/proc/mounts` exposes no mount path named Steam, Windows, `/mnt/c`, games, media or `/run/host`; `/mnt/c/Windows/System32/cmd.exe`, `/home/openclaw/.steam`, and `/home/openclaw/.local/share/Steam` are absent. These negative checks are bounded to this sandbox and do not prove that Jerome lacks a Windows install. The inaccessible `/root` home was not inspected.

## Single next action / executable route

**On an authorized D3D10-capable Windows PC, connect one OpenClaw node to this Gateway and return a qualification receipt before commissioning any frame capture:** node ID/status, the actual runnable Death Leak game EXE full path and SHA-256 (`Get-FileHash -Algorithm SHA256 -LiteralPath <actual-game-exe>` in PowerShell), whether that EXE is the shipped Steam **app 995990** or an isolated build of checked-in `DeathLeak.sln`, and the running game's native backbuffer size/format. If no such EXE exists, report that precise failure; do not substitute `SteamPipeGUI.exe`. The Gateway address and Windows EXE path are not present in this session, so a remotely executable invocation cannot honestly be filled in here. This is the first missing transaction, not a request to rerun the Linux GPU lane.

Once that receipt exists, the already specified route is one active **Forest-1 bridge/ID117** capture: quick-play `a. Forest Exploration 1`; hold the left-raft/right-branch view for two updates without modal/shake; assign an ordinal before original `Render`; read the native D3D10 backbuffer after `UIManager::Draw` and before Present; write lossless image and synchronized drawn-state/asset-identity sidecar under the same ordinal, image first/JSON last. `port/FOREST1_MATCHED_FRAME_CAPTURE_PACKET_2026-10-23.md` defines the exact camera, player, HUD, effect-time, input/history and hash acceptance fields. A source-built EXE needs the hook in its own Windows build; a shipped EXE needs a validated state hook as well as frame-debugger pixels. Only after that packet validates should a separate Linux same-state capture be considered. Neither a Windows frame nor pixel parity was obtained here.

## Scope

Only this unique report was written. No source, build, GPU/capture, shared status/checkpoint, node pairing, archive extraction, staging, or unrelated change was made.
