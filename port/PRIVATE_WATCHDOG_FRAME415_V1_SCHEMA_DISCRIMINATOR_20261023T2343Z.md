# Frame-415 v1 packet schema discriminator — 2026-10-23 23:43Z

## Verdict and evidence

**Strict v1 shape regression, isolated to two `transition` members.** I parsed the saved original `port/build/private-watchdog-linux-frame415-atomic-20261023t2322z/frame415.json` and fresh `port/build/private-watchdog-postqa-frame-packet-gpu-20261023t2340z/frame415.json` recursively. Both declare `schema: deathleak-linux-frame415-v1`; all top-level and other nested key sets, array lengths, and JSON value types match. The fresh packet alone adds `transition.firstPostfadeOrdinal: -1` and `transition.loadingSplashSubmitted: true`. Image/state values are run-dependent and were not compared for equality. The post-QA gate reports separately acquired GPU runs, not same-run frame 415/424 evidence; this schema finding needs no GPU inference.

At `port/linux/main.cpp:1744`, the tag already branches on `postfadePacket`; at lines 1764–1767 the two new members are emitted unconditionally before the common transition members. `packetFirstPostfadeOrdinal` is postfade-specific state and `-1` has no useful first-reveal meaning. `loadingSplashSubmitted` is true on this v1 draw and is derivable from its phase/tint fields, but is still an undocumented shape change. `preHudImage`, `dynamicDraw.sceneDrawIdentifierScope`, and ordered scene draws are already conditional on `postfadePacket` (lines 1762–1763 and 1799–1819); the postfade packet has its own `deathleak-linux-postfade-waterfall-v1` schema tag. The first-reveal and postfade CLI flags are mutually exclusive (lines 245–256).

## Precise compatible source-owner fix

Keep `deathleak-linux-frame415-v1` **exactly** at its saved original key/type shape. In the shared serializer, emit `transition.activeLevel` first, then emit `firstPostfadeOrdinal` and `loadingSplashSubmitted` **only when `postfadePacket` is true**, then emit the existing common `loadingPhaseOnEntry` onward. Preserve the current postfade v1 tag and those two fields there. A minimal, valid split of the existing stream at that point is:

```cpp
packet << "  \"transition\": {\"activeLevel\": " << JsonString(currentLevel);
if (postfadePacket)
    packet << ", \"firstPostfadeOrdinal\": " << packetFirstPostfadeOrdinal
           << ", \"loadingSplashSubmitted\": "
           << (loadingPhase != LoadingPhase::None);
packet << ", \"loadingPhaseOnEntry\": "
       << static_cast<int>(packetLoadingPhaseOnEntry);
```

Continue the existing common stream from `loadingPhaseAtDraw` onward; retain the existing `std::boolalpha` setup. Do **not** change the existing frame-415 tag or turn those keys into v1 `null` sentinels; either would leave a strict original-v1 consumer with a changed shape. If those fields genuinely must be present on first-reveal packets, create a new `deathleak-linux-frame415-v2` schema and keep a selectable/emitted v1 path; that is unnecessary for current functionality.

## Focused static / CPU regression gate for the source owner

1. Parse the two saved fixture JSONs above with Python. Assert original and fresh packet tags are the same; compute recursive object-key and JSON-type fingerprints. Before the fix, assert the *only* differences are the two added transition keys, to guard against accidental fixture confusion.
2. After editing, exercise the serializer in a CPU-only fixture/unit harness (or a non-GPU capture route if available) for both packet modes. Parse emitted JSON, compare frame-415 recursive key/type fingerprint against the **saved original** fixture, and assert the two keys are absent there. Assert postfade tag remains `deathleak-linux-postfade-waterfall-v1`, both keys are present with integer/bool types, and `preHudImage`/`sceneDrawIdentifierScope` remain postfade-only. Check that frame-415 `orderedSceneDraws` remains the unavailable object, postfade is an array. No image hash or camera numeric equality across runs is expected.
3. Add a source-level assertion at the serializer seam that these two names occur only inside the postfade branch; this catches a future unconditional reintroduction even when no CPU capture backend can run. A JSON parse/fixture assertion, rather than a brittle full JSON byte comparison, checks the actual compatibility contract.
4. Keep the existing atomic JSON-last behavior and BMP/hash validation in any runtime smoke test. This schema repair alone does not remove the separate CLI same-run pairing blocker and does not establish original-D3D10 parity.

## Consumer check / handoff

`git grep` over tracked non-Markdown code, excluding `main.cpp` and build output, found no parser or caller of `deathleak-linux-frame415-v1`, the two new keys, or `--atomic-frame415-packet`. A second scan of port source/scripts outside `port/build` likewise found only `main.cpp`. Saved JSONs and Markdown reports are evidence/documentation, not executable consumers. Thus no in-repository executable consumer requires these extra first-reveal fields; external strict consumers remain unknown, so preserving the published v1 shape is the conservative fix.

Read-only investigation; only this unique report was written. No source, test, build, Docker, GPU/Xorg, shared checkpoint, index, or owner file changed. Handoff to current `main.cpp` camera/source owner for the conditional serializer edit and CPU regression gate.
