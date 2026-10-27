# Resumed keyboard/mouse verification — 2026-10-26

The remote fetch configuration tracks only `master`; an explicit port-ref fetch (not the initial generic fetch) recovered `4b29327`, a docs-only Sea-1 blocker commit after the controls base. That upstream record is preserved during integration.

User explicitly authorized resumption of the stopped controls checkpoint. Owner: announcing hidden child `331a5bf4-61f5-4ba3-8d7a-b77a3b72558a`; completion is delivered through its parent. No scheduler or unrelated stopped worker was restarted.

## Software scope

The existing controls branch adds attached-gamepad priority, dedicated menu navigation/accept and modal back/restart, and logical-coordinate gameplay cursor aim/fire with keyboard fallback. The deterministic mouse script now reaches the same gameplay consumer used for live pointer coordinates. It does not inject a projectile directly.

Resumption finished the previously interrupted Docker build. The first complete-suite attempt exposed inherited CPU spies using the pre-fullscreen `Renderer::Initialize` signature. The strafe fixture also still expected an omitted held aim line despite production emitting that line. Repair keeps explicit body/aim-line count, texture and alpha assertions, not just a relaxed count. These changes are made in the clean controls checkout; the unrelated dirty source checkout is preserved.

Further suite triage exposed legacy printed endpoints predating the source-backed `22950f8` 60-Hz delta clamp: swim Y differs by 0.01, focus recovery by 0.0001, and descending contact Y by 0.001. Tests preserve the gameplay assertions while correcting bounded numeric expectations; no simulation constants or route inputs are changed.

The mouse consumer regression checks exact held-fire count (three shots in 30 fixed steps), left/right projectile velocity sign, release after one step (one shot), all four out-of-viewport edges (no shots), and ordinary scripted-input ownership (no mouse override). This is deterministic functionality evidence, not physical input delivery.

The SDL backend fixture (`make -C port input-backend-test`) uses SDL virtual game controllers to verify initial attachment, all mapped buttons, analog movement/menu/aim axes, detachment to disconnected fallback, and hot-plug reconnection. It invokes the actual renderer backend and explicitly allows a dummy software renderer only for this input fixture. It is not physical controller or display acceptance.

## Physical GPU/display observations

A 180-frame active Forest normal-control rightward smoke on physical Xorg `:0` selected strict SDL OpenGL, `accelerated=1 software=0`, direct `nouveau / NVE7`; software rendering was not enabled. Actual output was 1280×720. It ended alive with five earned orbs and zero missing original assets. After 120 warmup frames, 58 frame-wall samples measured p50 16.9911 ms, p95 17.3452 ms, p99/max 18.403 ms; no >25/33 ms samples. Present-interval p50/p95/p99 were 17.4416/18.3388/18.9008 ms. This short, windowed, scripted check is not a sustained GPU benchmark, GPU timer measurement, human-visible parity review, or a 16.67-ms acceptance pass.

Fresh read-only XRandR reports the physical LVDS current mode at 1440×900 ~59.90 Hz, with no advertised physical mode above 60 Hz; external DP/VGA outputs are disconnected. Apparent double-scan mode clocks do not establish >60-Hz physical scanout.

The X root tree currently contains `slick-greeter`, not an unlocked gameplay desktop. No login/unlock or input injection into that session was attempted. `/proc/bus/input/devices` reports the built-in keyboard/trackpad and `applesmc` accelerometer (`js0`), not a verified SDL gamepad. Physical keyboard/mouse gameplay and gamepad hot-plug remain unaccepted. Recovery requires a user-unlocked/attended desktop, a real controller, and a connected >60-Hz display for that separate gate.

## Broad-suite blocker and bounded recovery

The full integration suite does **not** pass. Its historical `R:710,E:40,B:4500` village route falls from Waterfall-top to bottom at frame 788 and ends there with 12 orbs, rather than reaching village. An exact archive/build of remote `4b29327` produces the identical transition, player/combat endpoint and earned count as the controls binary (wall elapsed normalized). This is verified inherited route/test drift, not a controls regression. No gameplay constants, damage, currency or authored geometry were altered to make it pass. The longer diagnostic suffix was explicitly non-fail-fast and is **not** acceptance evidence; it also shows why blindly refreshing endpoint numbers would be invalid.

Implemented the next bounded recovery gate, `sh port/tests/waterfall_village_recovery.sh`: ordinary controls `R:711,E:40,B:4515` now reach village central through five authored transitions at 398/1074/1730/2730/4053, ending at `(7287.18,-1525.13)`, alive at health 4, with 101 distinct earned saved IDs and balance 101. Synthetic 60/144 fixed gameplay/combat endpoints are identical. The timing sweep finds the crossing window at jump starts 711–734 (710 fails); the one-frame-delayed jump is used. This is an independently asserted bounded route, not repair of all old earned/purchase/ending itineraries and not high-refresh hardware evidence.

Until the old progression fixtures and their earned-purchase/ending suffixes are reconciled on the current fixed clock and the full integration command passes, controls must not be merged/published as an integration-complete port. Existing controls work remains on its isolated branch; unrelated primary source changes and upstream Sea-1 blocker documentation are preserved.

## Remaining acceptance

Complete suite result and final integration SHA are recorded in `CHECKPOINT_STATUS.md`. Overall port remains partial: real-input/hot-plug playtest, matched original visuals, sustained physical-GPU/thermal frame-budget performance, high-refresh physical display and broader gameplay/audio parity are open. Offscreen/synthetic tests do not close them.
