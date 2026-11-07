# Raft WRAP CPU oracle implementation (2026-10-23)

Implemented only `port/tests/raft_wrap_cpu_pixel_test.py` plus this handoff. The independent `unittest` fixture verifies the tracked raft PNG SHA-256 before numerical use, Forest-1 XML IDs 11/57/58 and flip/UV-edge mapping, and CPU nearest/bilinear WRAP-versus-CLAMP addressing on a synthetic unequal-edge RGBA image. It builds Pillow BOX mips 0–4 from the tracked PNG; samples U and V endpoints, near-endpoint half-texel bands, and interior controls; checks exact level-3/4 U-edge RGBA probes; and compares straight-alpha compositing against a fixed opaque background. BOX values are fixture data, not a prediction of GPU mip generation or actual LOD. The source-state test isolates `DrawRaftMipExperiment` and requires exactly one `GL_REPEAT` assignment for each S/T axis between the private texture bind and quad draw; uploader state cannot satisfy it.

Verification from repository root:

- `python3 -m unittest port.tests.raft_wrap_cpu_pixel_test.RaftPixelOracle` — **4 tests pass**.
- `python3 -m unittest port/tests/raft_wrap_cpu_pixel_test.py` — **5 run, 1 expected failure**: current dirty renderer draw-site `GL_TEXTURE_WRAP_S` is `GL_CLAMP_TO_EDGE`; source-state requires `GL_REPEAT`. T is also currently CLAMP, but the first assertion stops the test at S.
- `git diff --check -- port/tests/raft_wrap_cpu_pixel_test.py` — no whitespace errors (new untracked file, so this does not validate its contents).

No renderer, other test, shared checkpoint/status, build, game process, display, GPU capture, or existing dirty file was changed. No commit was made. The CPU oracle demonstrates mathematical and tracked-asset sensitivity only; it does not establish fragment coverage, GPU mip texels/LOD, compositing in the renderer, or Windows parity. The source-state failure is intentionally pending the separate renderer owner's two-axis draw-site correction.
