# Raft WRAP draw-site preflight — 2026-10-23

## Decision

The smallest source-correct correction is **two draw-site enum replacements**, exclusively inside the dirty, opt-in `DrawRaftMipExperiment`: change `GL_CLAMP_TO_EDGE` to `GL_REPEAT` for `GL_TEXTURE_WRAP_S` and `GL_TEXTURE_WRAP_T` at `port/linux/SDLRenderer.cpp:307-308`. Do not remove the assignments: the private texture is cached and texture-object sampler parameters can persist across draws; `glPushAttrib`/`glPopAttrib` (`:303,342`) are not a reliable restoration of those object parameters. Do not edit the uploader, UVs, flip, raster transform, blend, painter routing, or default SDL path. This report makes **no renderer edit**.

## Source evidence and scope

- The original `Game/ShaderFiles/BasicLightTexture.fx:10-16` specifies `MIN_LINEAR_MAG_POINT_MIP_LINEAR`, `AddressU = WRAP`, and `AddressV = WRAP`; its vertex shader passes UV unchanged (`:37-46`) and pixel shader samples that sampler (`:53-59`). Forest-1 XML IDs 11, 57, 58 use `Media/objects/raft.png` with `effectlighttexture`, `repeatX=false`, `repeatY=false`; IDs 11/57 flip horizontally while 58 does not (`Game/XmlFiles/levels/forest_exploration_1.xml:191-203,928-955`). No-repeat constrains mesh UVs, not the effect sampler.
- The Linux private branch is selected only for exact `DEATHLEAK_RAFT_MIP_EXPERIMENT=on`, scene sampling, GL context, raft-path suffix and other eligibility checks (`port/linux/SDLRenderer.cpp:228-230,274-289`); if private allocation fails, ordinary SDL copy remains the fallback (`:297-298,266-270`). Selection is path-based, **not** an XML-ID assertion. The branch executes in the sprite's existing painter slot (`:228-230`).
- `WobbleMipTexture` caches the private texture by `TextureId` (`:789-796`) and initially sets S/T to `GL_REPEAT` with mip-linear minification (`:875-881`). After `glBindTexture(GL_TEXTURE_2D, name)` (`:306`), however, each actual private raft draw overwrites both wrap axes to `GL_CLAMP_TO_EDGE` (`:307-308`) before the quad (`:330-336`). Thus uploader-only REPEAT does not establish draw-time state. The quad's U/V endpoints remain 0/1, with X flip and PNG-row V orientation (`:324-329`); in-range linear edge footprints can still depend on wrap mode.

## Potential effects and limits

- Correcting S/T affects only successful opted-in private raft draws as currently routed. The GL object retains REPEAT after the call; the previous CLAMP also persisted. Because the object is cached and can be reused by other private paths, inspect any subsequent use of the same `TextureId` before calling this side-effect-free. The surrounding private draw rebinds the old texture and restores active texture/program/matrix/attribute state (`:299-305,337-343`), but that is distinct from restoring the private object's parameters. Other GL private paths explicitly set their own wrap state (`:418-420,448-451,562-565,597-600`); ordinary SDL textures are not this private GL name.
- A source-state mismatch does **not** prove a changed Forest-1 pixel or a Windows framebuffer match. `port/RAFT_WRAP_CPU_PIXEL_FIXTURE_2026-10-23.md` and `port/RAFT_WRAP_CPU_ORACLE_IMPLEMENTATION_2026-10-23.md` show a tracked-asset BOX-mip CPU oracle: transparent boundary texels make a composited difference unlikely at nominal mip levels 1/2, while levels 3/4 have address-sensitive alpha probes. BOX texels are not measured `glGenerateMipmap` output or actual LOD. `port/RAFT_WRAP_PIXEL_IMPACT_GATE_2026-10-23.md` likewise leaves fragment coverage and final pixels unresolved. Linux GL state and pixels require an opted-in live draw; Windows parity requires separate bound-sampler and matched-frame evidence.
- The dirty renderer has a larger uncommitted raft experiment (`git diff --numstat -- port/linux/SDLRenderer.cpp` observed 76 added, 1 removed), so the two-token change should be owned and verified against current diff, not applied from an old patch. `port/RAFT_WRAP_RENDERER_OWNER_PREFLIGHT_2026-10-23.md` did not establish exclusive renderer/GPU owner release. Preserve concurrent work and have the coordinator verify ownership before assigning an editor or GPU lane.

## Next verification

After exclusive renderer ownership and the two-line correction, from repository root run:

```sh
python3 -m unittest port/tests/raft_wrap_cpu_pixel_test.py
```

The existing CPU oracle has four independent pixel/source-context tests and one draw-site source-state test (`port/tests/raft_wrap_cpu_pixel_test.py:62-179`). The latter scopes between `bool DrawRaftMipExperiment(` and `DrawFractionalParallax`, then between the private `glBindTexture(..., name)` and `glBegin(GL_QUADS)`; it requires exactly one S/REPEAT and one T/REPEAT assignment, so the uploader cannot falsely satisfy it. On the inspected tree it fails at S because `:307` is CLAMP (T at `:308` is also CLAMP). After editing, also run `git diff --check -- port/linux/SDLRenderer.cpp` and inspect `git diff -- port/linux/SDLRenderer.cpp` to confirm only the intended two tokens were added to the preexisting dirty renderer diff. These CPU/static gates do not replace a later GL state assertion immediately before the actual ID11/57/58 quad or a frozen-state CLAMP/REPEAT lossless capture.

Scope of this preflight: read-only source/report inspection; this report is the sole write. No code/test edits, build, GPU/game run, commit, or shared checkpoint/status write.
