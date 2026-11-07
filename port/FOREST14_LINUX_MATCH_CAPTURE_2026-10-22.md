# Forest screenshot 14 — physical-Linux composition capture (2026-10-22)

## Evidence and scope

Steam reference: `../store_media/screenshots/14.jpg` (1920×1080 JPEG; SHA-256 `d7326063f5f9b163e9c6513ad2b68a27dd92f833e379a9049d9bc4b2b08dc3ab`). New Linux capture: `port/build/forest14-linux-match-2026-10-22/linux-r110.png` (1280×720 lossless PNG; SHA-256 `04f055a3ac3dec8a8c670b24ea1520573901a1ff8899b7f7c91a4703c0e817a4`). Original SDL BMP and full stdout/stderr are adjacent as `linux-r110.bmp` and `linux-r110.log`. The build used was repo HEAD `8a6dc85`; this task made no source, asset, or XML edits.

This is a *qualitative composition match* only. Steam screenshot 14 has no recorded Windows camera/player coordinates, viewport/backbuffer provenance, animation frame, elapsed time, collected-orb state or weather/particle seed. The approximate camera X≈700 from `FOREST_REFERENCE_AUDIT.md` was a search seed, not an exact Windows value. No pixel/color-difference metric or exact-frame parity is asserted.

## Reproduction and physical-GPU proof

From `/srv/openclaw/workspace/death_leak_analysis/source`, the capture used this exact invocation (with its stdout/stderr redirected to the named log):

```sh
mkdir -p port/build/forest14-linux-match-2026-10-22
timeout --signal=TERM --kill-after=10s 90s docker run --rm --name deathleak-forest14-match --network host --hostname jerome-Linux-MacBookPro --device /dev/dri --mount type=bind,src=/srv/openclaw/workspace/death_leak_analysis/source,dst=/src --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy -w /src deathleak-linux-build:bookworm port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:110 --frames 110 --fixed-dt 0.0166667 --capture port/build/forest14-linux-match-2026-10-22/linux-r110.bmp > port/build/forest14-linux-match-2026-10-22/linux-r110.log 2>&1
python3 -c 'from PIL import Image; from pathlib import Path; p=Path("port/build/forest14-linux-match-2026-10-22/linux-r110.bmp"); Image.open(p).save(p.with_suffix(".png"))'
```

The run entered active forest-1 gameplay, not preview/menu. The log reports `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, initialized Sepia.fx GPU postprocess, 71 original sprites with zero missing/unsupported assets, and two authored WAV ambience streams (audio output itself was dummy). Final state: player `(808.986,3135.97)`, 3 collected orbs, health 25, grounded, no enemies/projectiles; 110 fixed 1/60-s steps, reported elapsed wall time 2.40601 s. The log does **not** report camera coordinates, so no exact Linux camera value is claimed. The named Docker container had exited and `docker ps --filter name=deathleak-forest14-match` was empty after capture: GPU lane released.

## Visual comparison

After normalizing both 16:9 images for visual inspection, the important landmarks occupy the same broad composition: the low three-piece raft spans the left foreground; player is near its right end beneath the long mid-height branch; a diagonal sword/scroll pickup protrudes from that branch; two orbs hang beside the right trunk above the branch, one is left of the branch, and two low orbs lie near the player/trunk. The top branch and large right trunk also line up in kind and layering. This supports the forest-1 identity described in `FOREST_REFERENCE_AUDIT.md`; it is not a random similar forest screen.

Remaining visible differences:

- The Linux scene landmarks (player, pickup and right trunk) sit roughly **50–70 pixels left** of the reference when the 1920×1080 reference is scaled to 1280×720. This is an approximate screen observation, not a solved camera delta; the Windows camera state is unknown.
- The raft's left portion is visibly disjoint/steeper in the Linux capture: a down-sloping section reaches a seam near x≈200, then the middle section runs nearly level before the right descent. The reference reads as a smoother shallow arch from the left edge through the middle to the player. It is not yet possible to attribute this to a renderer defect versus unmatched camera, authored piece transforms/occlusion, or different capture state. The previously documented radians-to-degrees rotation correction is present in this build; the branch pickup is diagonal, not nearly vertical.
- The reference is softer and hazier around the lower raft/ground and distant trees. Linux bark/raft edges and foreground silhouettes are more starkly defined. The source screenshot is JPEG and unpaired in time/phase, so this observation alone does not prove a specific fog, color, shader or particle defect.
- Reference HUD reads 13 orbs and reduced health; Linux has 3 orbs and full health. The player pose and exact orb/effect phases also differ. The Linux capture has an extra low orb at the left edge of the raft region that is not comparably visible in the reference.

No new source-backed renderer discrepancy can be isolated from this unmatched still. The **next worker** can investigate the raft-piece projection/occlusion using the authored IDs 11/57/58 and original renderer transforms, but this report does not recommend an asset/XML change. A defensible exact visual comparison still needs a lossless Windows forest-1 frame with actual camera X/Y, player/state, viewport, zoom, frame/phase and weather/particles recorded, then an equivalent Linux capture.
