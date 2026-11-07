# Dirty SDL renderer visual review (15:29, 2026-10-23)

## Verdict

**No demonstrated rendered-pixel regression from the dirty diff; one concrete experiment-scope mismatch.** The environment-gated branch is described and the proposed acceptance gate is framed as forest-1 raft IDs 11/57/58, but the code selects by texture-path suffix, not level or object ID. With `DEATHLEAK_RAFT_MIP_EXPERIMENT=on`, it can alter ordinary raft draws throughout the game. This is a source-backed coverage risk, not proof that any such pixels are wrong. With the switch unset, the new branch is inactive.

## Evidence

- `git diff -- port/linux/SDLRenderer.cpp` changes only `DrawSprite`'s branch at lines 225–230 and adds `RaftMipExperimentEnabled`/`DrawRaftMipExperiment` at lines 271–344. The predicate checks scene phase, exact env value `on`, current GL context, texture path ending in `Media/objects/raft.png`, and excludes source-rect/repeat/UV-shift/pixel-wobble/foliage/damage-burst cases. It has no level or XML-ID condition. The alternate GL quad uses a generated mip chain and replaces the SDL copy at that painter slot.
- A read-only XML inventory of `Game/XmlFiles/levels/*.xml` found **95 authored raft sprites in eight levels**, all with `repeatX=false`, `repeatY=false`, and authored alpha 1. Of these, **93 use `effectlighttexture`** and are candidates for the new ordinary branch under its runtime predicates; two first-ninja-arena entries (IDs 60/65) use `effectpixelwobble` and are excluded. Candidate counts by level: forest_exploration_1 **3**, cliff **3**, first_ninja_arena **6**, grass_exploration_2 **20**, grass_exploration_3 **8**, village_central **4**, village_shrine **47**, waterfall_top **2**. Examples outside forest: `Game/XmlFiles/levels/cliff.xml:1941,1956,2096`, `waterfall_top.xml:596,611`, `village_shrine.xml:968–1433`. These are authored candidates, not a claim that all are visible in one run.
- Their authored sizes vary across **14 width/height pairs**, unlike forest-1's common 458.298584 × 125.316017. Thus a forest-only off/on crop cannot certify the switch's complete enabled visual footprint: LOD and alpha-edge behavior can differ with scale/rotation. Original `Game/ShaderFiles/BasicLightTexture.fx:10–16,53–59` specifies mip-capable sampling and WRAP for the `effectlighttexture` path; it does not establish the actual Windows bound mip descriptors for any level.

## Distinct next action

Before calling this an ID11/57/58-only A/B or considering enabled-mode visual acceptance, **either constrain the experiment to those three draws at the scene submission point (where XML ID is available), or explicitly label it as an all-level raft experiment and include at least a differently scaled non-forest raft in the off/on visual gate**. Preserve the separate Windows bound-SRV and matched-state reference gate already specified in `RAFT_MIP_WINDOWS_BINDING_GATE_2026-10-23.md`; a Linux off/on difference alone is not a fidelity verdict.

This report intentionally does not repeat the previously documented conditional alpha-squaring, sampler wrap, player-order, or Windows-descriptor findings in `CURRENT_RENDERER_DIRTY_VISUAL_AUDIT_2026-10-23.md` and `RAFT_MIP_RENDERER_EXPERIMENT_2026-10-23.md`. Read-only source/diff analysis only: no build, GPU/Xorg, tracked-file edit, worker-report edit, or capture. Renderer lane released.
