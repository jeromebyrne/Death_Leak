# Active focus-bar vertical flip — read-only CPU discriminator (2026-10-23)

## Result

**Current tree already contains the exact one-field correction.** `Game/XmlFiles/UI/game_hud.xml:88-98` authors `91_xp_meter` with `vflip="true"` and `xp_meter_bar_middle.png`. `UISprite::XmlRead` retains `m_verticalFlip`; `UISprite::Reset` reverses Y texture coordinates when it is set; `UIMeter::Draw` changes horizontal flip for pieces but does not clear vertical flip. The current Linux `port/game/Hud.cpp` focus-fill command passes `1.f, 0, false, true` after tint, meaning opacity 1, rotation 0, `flipX=false`, **`flipY=true`** per `port/render/Renderer.h::Sprite`. Its texture, destination `{43,66,127.f * displayed,9}`, and tint `{255,255,255,220}` are unchanged from the pre-fix diff. No new source patch is indicated.

## Bounded discriminator

A read-only Python source/XML assertion was executed and **PASS**: it parsed the XML meter and its authored texture/vertical flag, identified the actual `focusBar_` DrawSprite aggregate in current `Hud::Draw`, checked the positive-fill destination expression and exact trailing flip fields, and inspected the existing texture-ID-aware renderer spy. The existing `port/tests/focus_bar_vertical_uv_private_cpu_spy_20261023.cpp` drives `Hud::Draw` with nonzero displayed focus, including a snap decrease to 0.5 at process time `1 + 1/60`, yielding an expected **63.5 px** fill width. It identifies the focus command by loaded texture ID (not ordinal alone), asserts `flipY=true`, destination, tint, other sprite fields, nine sprite commands and no non-focus vertical flips. The earlier implementation handoff `port/PRIVATE_FOCUS_BAR_VERTICAL_UV_CPU_IMPLEMENTATION_20261023.md` reports this executable spy and existing HUD transition/health-length spies PASS; I did not compile or rerun them under this read-only assignment. A zero-width/zero-focus control would not demonstrate visible UV parity.

The prior one-field fix was adding `flipY=true` only to the focus fill aggregate in `port/game/Hud.cpp`. The working-tree diff confirms this exact change is currently present. A live renderer/GPU or Windows pixel match is **not** established by this CPU/source discriminator.

## Verification affected and release

Maintain the focused vertical-UV spy plus existing `hud_transition_cpu_spy_test.cpp` and `health_meter_length_cpu_spy_test.cpp` when integrating the HUD edit; later, if desired, compare a same-state positive-fill GPU ROI against a Windows reference. This private read-only QA lane is **released**. The separate HUD-film owner retains the build/Xorg/GPU/capture lane. No shared source, test, status, Git index, build, Docker, GPU, or capture was changed here; this report is the only write. No commit.
