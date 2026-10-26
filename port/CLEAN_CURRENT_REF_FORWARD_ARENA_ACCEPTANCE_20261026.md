# Clean-current-ref forward route and arena-hit CPU gate — 2026-10-26

Base: live `origin/feature/linux-renderer-foundation` `7ac8333dbfadf5accee31f94ff853cc2d5baac3a`, checked in a detached isolated worktree. The primary divergent/dirty checkout was not used for build or source edits. Docker `deathleak-linux-build:bookworm` production `make -C port -j2 all` passed (existing TinyXML warnings). Final `sh port/tests/uninterrupted_forward_combat_routes.sh` passed against that binary.

The uninterrupted ordinary-control, no-fire script `R:1100,Q:40,R:641,J:200,E:2200,L:48,I:10` produced the authored sequence Forest→Waterfall-top frame 398→Waterfall-bottom 762→underwater cave 1378→cave 6 1780→Hot Springs 3224→cave 4 3377→cave 3 3602→cave 4 3637→cave 5 3868→keyless cave-5 door back to Forest-1 frame 4231. Final state: alive, health 25, earned orbs/balance 38, 0 fired. All transitions and final state are now asserted in the scoped test. There was no warp, save load, injected state, XML edit or collision bypass.

The separate uninterrupted ordinary-control script `L:1500,Q:40,L:100,F:300,L:700` entered grass-3 frame 300 and first-ninja-arena frame 1093, fired in that authored arena, then took the authored arena→grass-1 exit frame 2292. Final state: alive, health 22, earned orbs/balance 43, one actual hit, zero kills, 18 fired. This verifies actual damage followed by an exit, not a kill or broad combat parity.

Test-only offscreen OpenGL reported Mesa llvmpipe. Physical `DISPLAY=:0 XAUTHORITY=/home/openclaw/.Xauthority xrandr --current` returned `Invalid MIT-MAGIC-COOKIE-1 key` / `Can't open display :0`. No physical GPU, hardware input, selected >60-Hz, sustained FPS or release acceptance is claimed. Atlas/bind work was not touched.
