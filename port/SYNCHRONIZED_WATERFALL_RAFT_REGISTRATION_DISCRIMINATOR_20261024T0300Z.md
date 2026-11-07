# Synchronized Waterfall raft-registration discriminator — 2026-10-24 03:00Z

## Decision

No distinct original-versus-port defect is proven by the synchronized active Waterfall packet, and no production patch is justified. The highest-impact visible candidate remains the broad *composition/framing* difference at the paired rafts versus Steam screenshot 16, but its three static landmarks move together and the original screenshot has no camera or presentation-state sidecar. This report adds an exact **within-port temporal transform oracle** to distinguish camera framing from a future raft-transform error; it is independent of the earlier ID22 ground-blur A/B. It does not assert parity.

## Evidence and exact oracle

The 0252Z packet's X11grab/present-clock alignment maps 18 s to active `waterfall_top.xml` present 655, camera X = -411.001, player X = -291.015; 20 s maps to active present 711, camera X = 36.9856, player X = 156.985. Camera Y remains 3000. The camera moved **+447.9866 world units**. `port/game/LevelScene.cpp` draws ordinary ground sprites at `640 + ((worldX-cameraX)*layerX - width/2)*2/3`; ground has layerX = 1. Thus an unchanged authored ground landmark must move **-298.6577 screen pixels** between these two port frames (allowing subpixel filtering/feature-location uncertainty). `Game/XmlFiles/levels/waterfall_top.xml` IDs 32 and 33 are fixed, non-scrolling kGround raft sprites at X=-92.014648 and X=803.985352. The predicted displacement is identical for both despite their differing rotation; their 896-world-unit X-center separation remains 597.333 screen pixels. This is an executable discriminator for later image registration, not a measurement extracted from the compressed video.

At 20 s the paired raft tips and upper ledge are visible; at 18 s only the first raft is entering. The prior read-only review estimated all three 20 s features about +175–190 pixels right of the screenshot-16 features after resizing the 1920×1080 JPEG to 1280×720. A common offset of that size is consistent with about 263–285 world units of *unknown original camera-X difference*, so changing the XML coordinates, rotation, flip, blur, or HUD based on that comparison would conflate unlike camera states. The screenshot's 24-orb HUD versus the port's 10-orb state also confirms the frames are not gameplay-state matched. Waterfall-bottom stills at 22/24 s cannot register screenshot 16's Waterfall-top rafts.

## Reopen threshold

Acquire one original *active* Waterfall-top frame with camera XY, world/player XY, 1920×1080 backbuffer (or exact crop/scale), gameplay clock, film/fog phase, and original D3D10 post-transform corners/draw order for IDs 32/33 and the upper ledge. Render the port serially at that same state; register the static raft/ledge landmarks first. Only a residual non-common displacement, distorted corners, wrong alpha/order, or pixel residual tied to a first divergent source field would justify a scoped fix. If an original command packet cannot be obtained, screenshot 16 remains a composition reference only.

## Scope and release

Read-only source/sidecar/previous-gate inspection plus this one new report. No source, test, asset, renderer, status, or shared checkpoint edit; no build/capture/container started. No production software change to commit or push. Existing dirty files preserved. Exclusive physical Xorg :0/GPU/build lane released. User visual review remains pending; no parity claim.
