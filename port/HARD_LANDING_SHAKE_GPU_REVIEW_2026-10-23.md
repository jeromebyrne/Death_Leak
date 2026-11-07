# Hard-landing shake physical-GPU A/B gate — 2026-10-23

**Verdict: Linux shake rendered and passed the bounded temporal discriminator; original-Windows visual parity is not established.** The physical Xorg `:0` / direct `nouveau NVE7` lane and isolated build lane are now **released**. No source, shared status, or unrelated dirty file was edited.

## Ownership and identities

At entry `HEAD=1fa54c332deaeedc9dc94aa50699167a87040b51`; `CHECKPOINT_STATUS.md`, `port/COMPOSITION_DIAGNOSTIC_NOTES.md`, `port/linux/SDLRenderer.cpp`, two test files and numerous reports were already dirty/untracked. `PORT_WORK_STATUS.md` explicitly reserved this GPU gate after the HUD-bars owner released `:0`/build; the concurrent Forest worker was read-only. No competing game/build/capture process was observed. All capture/build outputs are private under `/srv/openclaw/workspace/death_leak_analysis/shake-gpu-2026-10-23/out/`; the shared `port/build/deathleak-linux` was untouched.

A clean detached worktree at `05cc746^ = c0c87ff` and another at `05cc746` were serially built with the same `deathleak-linux-build:bookworm` image (`sha256:73cf3ceea9cb72793e400e8d3bc5a029195626a2ff2082d12edfc4d81bfda3e7`) and Makefile flags to private targets. Binary SHA-256: A `0370c58e6c89ddbf057837647d23f906a3ebd193e64f04ffe08e1a9048640f61`; B `3d58632964c1a5a6a770638b1bdecbb7913777c03de38dacf8f7e430237efa91`. A private build from the integrated current tree (HEAD above plus dirty renderer) also passed; binary `1fce8f00fe3ef50808df7da403a9c708c392e29f0f912d7bb5f3f9c6f16dcf78`. The relevant dirty source diff hash at build was `55fb964584e54a3cf8a55aa6ccb534059107afcb343b5f6c54cc7873568c215e` (`git diff --binary` over main/SDLRenderer/PlayerMotion/CameraShake). The clean A/B is the causal commit comparison; the integrated capture is corroboration, **not** a clean causal control. The first read-only source mount build failed only because Makefile `mkdir -p build` needs a writable private worktree; the rerun used the isolated worktree and passed.

Canonical test save SHA-256 `952bb2304a897feb5aaf04a7c2672e4d953470ae62a39e334319e3352e26d836`; forest XML `6d36a837708a732ac7d2a085010a3d22229e29618b8594b5c2576d491c4161c5`. Fixture: DL5 forest-1 at `(-500,3811)`, health 25/focus 100, zero orbs. Each ordinal used a fresh fixture copy because `--load-save` rewrites it. Runs used `--script '.:100' --fixed-dt 0.0166667 --frames N --capture ...` for N=27,28,29,30,32,36,44,46,48, separate processes, **active loaded gameplay**, never menu/preview/simulation-only. Exact reproduction scripts are `out/capture.sh` and `out/capture-current.sh`; 27 original BMPs, corresponding logs, and `out/bmp-sha256.txt` are retained.

All 27 logs passed the explicit check for `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, GPU Sepia initialization, `Loaded 71 original level sprites; 0 missing or unsupported assets`, and forest-1 gameplay completion. Audio was SDL dummy; this gate does not test sound. All variants report airborne `player=(-500,3114.41)` at N=27 and grounded `(-500,3108.38)` at N=28, then the same grounded endpoint through N=48. The 702.62-unit apex-to-landing drop and committed CPU event test support a hard-impact trigger; there is no separate runtime event trace, so its exact internal firing is inferred from the causal pixel response and source code.

## Rendered discriminator

Lossless A/B BMPs are **pixel-identical over all 1280×720 pixels at N=27 and N=28**. Thus no premature/contact-frame shake displacement was detected. From N=29 onward, the same-ordinal A/B scenes differ. A static branch patch `(260,280)–(475,350)` was horizontally registered by minimum mean absolute RGB error over integer X shifts ±20; a static bridge `(780,478)–(1100,540)` and tree trunk `(800,160)–(880,420)` independently corroborate it. Reported positive displacement is B landmark position relative to A, in **screen pixels**, not world units:

| Rendered ordinal | branch | bridge | trunk | Interpretation |
|---|---:|---:|---:|---|
| 27 / 28 | 0 | 0 | 0 | pixel-exact airborne/contact negative controls |
| 29 | +9 | +9 | +7 | first post-contact world displacement |
| 30 | 0 | 0 | 0 | temporal jitter/follow can transiently cancel |
| 32 | +15 | +15 | +12 | world displacement persists |
| 36 | +12 | +12 | +9 | active shake/follow |
| 44 | +13 | +13 | +10 | active shake/follow |
| 46 | +10 | +10 | +8 | decay/residual follow |
| 48 | +9 | +7–9 | +7 | residual follow; not pixel equality |

The registration uncertainty is roughly ±1–2 px for the bridge at some ordinals because foliage/film/animated details alter the patch score; the static branch has a clear integer minimum at the primary N=29/N=32 discriminators. Depth/parallax explains unequal trunk movement. Scanning branch vertical offsets ±3 at N=29 and N=32 found **Y=0** as the minimum after X alignment, consistent with forest-1 follow-Y=false. The opaque red health bar's pixel bounding box remains exactly `(43,46)–(90,51)` in both variants at N=27/28/29/32/36/48; HUD remains screen-anchored even though the larger HUD patch includes changing world pixels behind it. This is stronger than a whole-frame difference alone: contact and precontact images are identical, then rigid world geometry moves while HUD geometry does not.

The integrated current-tree branch registration against clean A reproduced 0 px at N=27/28, +9 at N=29, 0 at N=30, +15 at N=32, +12 at N=36, +13 at N=44, +10 at N=46 and +9 at N=48. This verifies the effect survives the current integrated build at these frames; its extra HUD/renderer changes prevent treating it as the sole-feature A/B.

## Limits and release

The fixture is an artificial airborne start, not a traversed route. Integer patch registration is a visual effect test, not direct camera-state telemetry. Residual X offset at N=48 is allowed by follow lag and does not prove retrigger; no sustained-contact repeat was seen in this bounded sequence. No matched original Windows active forest frame/trace or Jerome visual review was available; therefore **do not claim original visual parity, full gameplay acceptance, or release acceptance**. The clean A/B excludes today's integrated changes, while the integrated run corroborates only this event's temporal behavior.

All Docker game/build processes exited; final process check found no Death Leak, make, or FFmpeg process and no remaining game Docker container. Physical GPU `:0`, `/dev/dri`, and build/capture ownership are explicitly **released**. The two detached worktrees and private evidence remain for reproduction; unrelated dirty source/status is preserved.
