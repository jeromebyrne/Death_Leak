# Private sword-pull final-composite GPU discriminator — 2026-10-23

## Verdict

**The bounded N=180 same-process gate passes on the Linux port.** A and A2 (independent copies of the same post-update scene) are byte-identical across the full 1280×720 BMP, as are A and C (explicit 5/1 alpha recomputation). Suppressing only the intro-spray draw produces 7,001 changed RGB pixels inside the same-process dilated raw-spray/interior mask. Deliberately wrong .1/.8 alpha produces 8,109 changed pixels there. This proves a visible final-composite contribution and 5/1 draw-alpha parity for this Linux state. It **does not** prove a matched Windows visual/composition result.

| Comparison | Interior mask changed pixels | RGB absolute sum | Inclusive bbox | Full-frame changed pixels / RGB sum |
|---|---:|---:|---|---:|
| A–A2 | 0 | 0 | none | 0 / 0 |
| A–C | 0 | 0 | none | 0 / 0 |
| A–B | 7,001 | 68,330 | (538,416)–(658,546) | 7,002 / 68,331 |
| D–C | 8,109 | 289,251 | (538,416)–(658,548) | 8,133 / 289,309 |

The same-process black-backed raw-spray pass has 7,304 nonblack pixels; its 2-pixel-dilated mask intersects 10,384 interior pixels (y=100..619). The small full-frame excess over masked deltas is outside that strict mask and does not affect the gate.

## Frozen execution and reproducibility

The source tree had unrelated pre-existing edits, including `port/linux/main.cpp`, `port/game/ParticleSpray.h`, renderer, tests, and shared status. None were modified. I copied only build inputs to ignored `port/build/private-final-composite-20261023/port-snapshot`, then instrumented **that copy only** with `instrument.py`. The diagnostic mode was placed at `LevelScene::Draw`'s intro spray player seam. B suppresses those two draws, C recomputes each live particle's alpha with the actual update formula at 5/1, and D recomputes alpha at .1/.8. It changes no particle position, size, brightness, texture, flip, emission, update or RNG. Each of A/B/C/D/A2 gets a distinct `LevelScene` copy after the one update/shake sample. Each replays BeginFrame, SetWorldZoom, scene draw with rebuilt painter items, sepia, frozen film index, bars, HUD and SaveFrame. Only one EndFrame and shader-clock RenderCompleted follow the passes. Raw surface is a separate scene copy after these passes. `analyze.py` and `analysis-180.json` preserve the measurement method/results; `run-180.log` preserves runtime provenance.

Built from the snapshot using image `deathleak-linux-build:bookworm` (`sha256:73cf3ceea9cb72793e400e8d3bc5a029195626a2ff2082d12edfc4d81bfda3e7`), with unique `/out`, without touching the shared build. The single rendered process used `--fresh-save-intro --play Game/XmlFiles/levels/forest_exploration_1.xml --script I:220 --fixed-dt 0.0166667 --frames 180 --capture /out/gate-180 --final-burst-gate`. Runtime logged pull at frame 165, x11/opengl accelerated=1 software=0 strict accelerated, `nouveau/NVE7`, GPU Sepia initialized, 71 original sprites and zero missing assets. Physical Xorg `:0` and active LightDM Xauthority were mounted into the Docker process; no software/offscreen fallback. `run-180.log` ended normally at forest-1. A competing sea-2 gdb capture briefly existed at preflight but had finished before this GPU run; no overlapping capture was performed.

### SHA-256 of unchanged BMP artifacts

- A: `9fe21363897e48d55cee78d3d6ac32f95edfd5e13245c4a36b84d254aa542880`
- A2: `9fe21363897e48d55cee78d3d6ac32f95edfd5e13245c4a36b84d254aa542880`
- B: `a9a96654f4af484a0a934e3d63dba19713610cbd292d89534dbb309c387433c6`
- C: `9fe21363897e48d55cee78d3d6ac32f95edfd5e13245c4a36b84d254aa542880`
- D: `fd73e9a9fa7a285f5edca97561afdbd6221c1ed922073827bbeb15691cbea37d`
- Raw: `4ff3872f76b1bc306f9cd49c783d6387b499a408be8e0aef691f3621907a7368`

All BMPs, frozen diagnostic binary, snapshot-only patch source, run log, analyzer and JSON are under `port/build/private-final-composite-20261023/`. The artifacts are ignored/private and not a shipping source change. There is no scoped software change to commit or push. **Exclusive physical Xorg :0 / NVE7 GPU, Docker build and capture ownership is released.**
