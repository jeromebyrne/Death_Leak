# Waterfall-top fixed-state camera-X sweep — 2026-10-23

## Result

A camera registration, not an authored-object translation, explains the three selected landmarks in the unmatched original screenshot. At fixed camera **(315, 3000)**, zoom 1 and 1280×720 viewport, the left raft tip, right raft tip and upper-right ledge tip fit `../store_media/screenshots/16.jpg` after resizing it to 1280×720 with one common translation of **(-1.67, -1.00) screen pixels**. The three-point 2D residual RMS is **2.49 px** (manual tip-pick uncertainty approximately 3–5 px). There is no stable differential displacement large enough to justify changing raft/ledge XML or production rendering from this screenshot. This is geometric registration only, not a matched Windows frame or shader/color parity claim.

## Reproduction and state

- Source is clean archived HEAD `5c2d335f77bb44499c955bfa933a5718a7640383`, isolated at `/tmp/deathleak-camera-x-sweep-5c2d335` from `/tmp/deathleak-head-5c2d335`; unrelated dirty shared-tree files were excluded. Only the isolated `port/linux/main.cpp` received a diagnostic `DEATHLEAK_DIAG_FIXED_CAMERA_X` override after camera follow and a capture-state log. The exact patch is in ignored `port/build/waterfall-camera-x-sweep-2026-10-23/isolated-main.patch`; diagnostic binary SHA-256 `a5039a2ed32513d2b1e194fc212062e4d43c4c77f0a9e159a8def578f353ecd4`. No production source or authored XML was edited.
- `capture.sh` launches actual `Game/XmlFiles/levels/waterfall_top.xml` for each isolated `--script '.:1' --frames 1 --fixed-dt 0.016666667 --capture` run. It sweeps camera X **-100, 0, 100, 200, 300, 315, 400, 500**; X=500 to X=0 is a 500-world-unit leftward shift. Camera Y is 3000 on every capture, XML zoom is 1, and the renderer viewport is 1280×720. All BMPs are physical Xorg `:0` accelerated GPU captures, not software renders. Each log says `SDL ... accelerated=1 software=0 ... mode=strict-accelerated`, `OpenGL ... nouveau/NVE7`, 81 original sprites and zero missing/unsupported assets. The capture process and Xorg GPU lane have exited.
- For registered X=315, `x315.log` records frame 0, player `(818.985, 3233.32)`, orb balance **0**, shader total/effect time **0.0166667 s**, dt **0.0166667 s**. Pose is the first-frame waterfall-top starting pose, visibly on/near the upper-right branch. These state/time fields are identical across sweep logs except camera X. The original JPEG has orb count **24**, a different airborne player pose, and unknown camera/effect time; it is explicitly unmatched. The comparison does not pretend otherwise.
- Evidence includes lossless `x*.bmp`, viewable `x*.png`, resized `reference16-1280.png`, `registration-annotated.png`, per-run logs, capture script, patch and `capture-sha256.txt`, all under ignored `port/build/waterfall-camera-x-sweep-2026-10-23/`.

## Source identity/order and projected corners

`waterfall_top.xml` XML order is **32, 33, 52** for the three objects. IDs **32/33** are `kGround` `Media/objects/raft.png`, centers `(-92.014648,2790)` and `(803.985352,2813)`, both size `674.042114×184.308380`, rotations `+0.180110` and `-0.279275` radians. ID **52** is `kGroundFront` `Media/objects/branch_3.png`, center `(797.086548,3245.201172)`, size `547.613953×199.370834`, rotation 0. Stable depth sorting draws IDs 32 then 33 at kGround, and ID52 later at kGroundFront. Their texture alpha contours, rather than bare rectangle corners, define the visible tips.

At camera (315,3000), using source projection `(640+(worldX-cameraX)*2/3, 360+(cameraY-worldY)*2/3)` and authored rotation, nominal transformed quad corners (TL, TR, BR, BL), in screen pixels, are:

| ID | Corners at X=315 (rounded 0.1 px) |
|---|---|
| 32 | (158.6,399.3), (600.7,479.8), (578.7,600.7), (136.6,520.2) |
| 33 | (733.1,487.5), (1165.0,363.7), (1198.9,481.8), (767.0,605.7) |
| 52 | (778.9,130.1), (1143.9,130.1), (1143.9,263.0), (778.9,263.0) |

## Three-point translation fit

The JPEG is 1920×1080 and was uniformly resized 2/3 to 1280×720. Points below are hand-selected corresponding *visible opaque tip* pixels in the resized reference and registered `x315.png`; they are not nominal rectangle corners. The annotated two-panel evidence shows every pick.

| Landmark | Source ID | Reference (x,y) | X=315 GPU (x,y) | Reference minus GPU | Residual after common translation |
|---|---:|---:|---:|---:|---:|
| Left inward raft tip | 32 | (571,558) | (576,559) | (-5,-1) | (-3.33,0.00) |
| Right inward raft tip | 33 | (747,507) | (747,509) | (0,-2) | (+1.67,-1.00) |
| Upper-right ledge tip | 52 | (783,165) | (783,165) | (0,0) | (+1.67,+1.00) |

Least-squares translation is componentwise mean `(-1.67,-1.00)` px, with residual magnitude RMS `sqrt(mean(dx_resid²+dy_resid²)) = 2.49` px; maximum point residual is 3.33 px. The +315 camera is ~315 world units to the right of XML initial camera X=0; equivalently, moving left by 315 from X=315 to X=0 shifts all three grounded objects right by 210 display pixels. The sweep also spans the requested 300–500-world-unit leftward interval. The visible shaft/foreground and player/orbs remain state-dependent, so this result only rules out a raft/ledge placement patch from screenshot 16; it does not establish full-frame or temporal parity.

No production software change was verified or committed/pushed. The GPU lane is released.
