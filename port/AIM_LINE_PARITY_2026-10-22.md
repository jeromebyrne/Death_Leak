# Runtime player aim-line parity — 2026-10-22

## Source-backed implementation

Windows `Player::AddAimLineSprite` creates a non-XML `Media\\aim_line.png` child at `kPlayer`, 400×400 world units, attached at projectile offset `(5,25)` and initially transparent. `Player::FireWeapon` refreshes its 0.05-second full-opacity hold **before** the cooldown/burst rejection; `Player::Update` then fades at 10 alpha units/second. Direction changes the horizontal attachment sign and vertical flip. See `Game/src/GameObjects/Player.cpp:345-365,388-415,634-697` and `Game/src/GameObjects/GameObjectManager.cpp:438-450`.

The Linux scene now loads this shipped PNG once per player level, inserts a separate child after the authored player at `kPlayer` depth, keeps it out of the XML sprite count, draws it after the body in stable painter order, and applies the same ordinary sprite texture/postprocess path. The current horizontal-only input chooses the right/left attachment and source-equivalent vertical flip; no diagonal aim is invented. Its alpha is refreshed on an eligible fire attempt, including a cooldown-rejected attempt. The firing arm remains keyed to actual projectile creation, so the two effects are independent. Movement/roll/water/melee/death guards use the port's available motion/combat state; `WallSide()==0` is the current approximation of the source's object-side collision guard.

No authored XML, PNG, global blending, ID37/ID15, or unrelated files were changed.

## Verification

- Docker bookworm build: `make -C port -j2` passed in `deathleak-linux-build:bookworm`.
- Focused `port/tests/aim_line_parity_test.cpp` passed in Docker: idle suppression, 400×400 projected size, `(±5,+25)` attachment, right/left UV orientation, stable post-body draw order, cooldown attempt refresh without a successful shot, 0.05-second hold, and 10/s fade/removal.
- Physical Xorg probe passed on `SDL video_driver=x11`, OpenGL `accelerated=1 software=0`, nouveau/NVE7; log: `port/build/physical-xorg-route/probe.log`.
- Physical NVE7 one-frame diagnostic captures with only the aim-line XML diagnostic ID enabled: `port/build/aim-line-gpu/idle.bmp` versus `fire.bmp`, both 1280×720, identical player position `(-43.0146,3384.32)`. Fire created one projectile; full-frame difference bounding box `(605,340)-(749,368)` contains 1,216 changed pixels. Captures and logs are ignored build artifacts, not committed. The difference includes projectile/other fire-dependent pixels, so this is hardware rendering evidence, not a pixel-isolated Windows parity measurement.

## Limit / acceptance

This fixes a source-semantic omission. The texture's sparse low-alpha footprint is small, and a matched Windows camera/input/time capture was not available. The visual acceptance decision remains Jerome's. Analog diagonal aim and strafing visibility remain outside this horizontal-input slice.
