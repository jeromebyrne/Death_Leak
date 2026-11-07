# Forest middle-band edge: bounded CPU review — 2026-10-23

## Scope and provenance

This is a **historical post-V-fix capture analysis, not a current-HEAD render or parity claim**. I read only `port/build/middle-band-attribution-2026-10-22/final-baseline-post.bmp`, `final-only-17-post.bmp`, `final-only-19-post.bmp`, and their matching `.log` files for the pixel measurements. The prior attribution note identifies these as final-binary 1280×720 forest-1 `R:180`, 180-frame, fixed-`dt=0.0166667` captures. All three logs confirm strict accelerated X11/OpenGL nouveau NVE7, initialized GPU `Sepia.fx`, 71 loaded sprites and zero missing, and the identical endpoint player `(1368.99,3107.3)` with five orbs. They do **not** independently record the camera transform or effect phase. The captures are matched in commanded route/frame and endpoint, but made in separate processes. No build, gameplay run, display/GPU use, or source edit was performed for this review.

## Method

Read the opaque post-sepia BMP RGB pixels at x=320, 640, 960 for y=380..560 (top-down screen coordinates). Rec.709-style luma is `Y=.2126R+.7152G+.0722B`; each vertical finite difference is `value(y)-value(y-1)`. Positive values brighten downward. In the only-ID images, the uniform pre-strip background is `(9,8,5)`. “First changed” means first exact RGB difference from that background in the stated interval. “First colored/green proxy” means first row with `G>=11` and `R>G>B`, i.e. green-channel rise of at least 3 over background in the sepia image. This is **not raw green dominance**: sepia makes the visible ground brown, and the opaque BMP cannot directly expose texture alpha. Dark/black non-background pixels ahead of the colored onset are therefore reported separately rather than miscalled the alpha edge. The largest absolute luma and G-channel jumps are listed as signed differences at the receiving row. Quantization and other rendered elements can affect single-pixel gradients.

| Capture | x | First changed | First colored/green proxy | Peak `ΔY` in y=380..560 | Peak `ΔG` in y=380..560 |
| --- | ---: | ---: | ---: | ---: | ---: |
| only ID19 | 320 | 441 | 451 | +25.2 @453 | +25 @453 |
| only ID19 | 640 | 461 | 463 | +13.1 @467 | +13 @465 and 467 |
| only ID19 | 960 | 455 | 457 | +21.5 @459 | +21 @459 |
| only ID17 | 320 | 519 | 520 | +49.3 @521 | +49 @521 |
| only ID17 | 640 | 505 | 516 | +57.6 @516 | +57 @516 |
| only ID17 | 960 | 527 | 529 | +64.7 @529 | +64 @529 |
| full baseline | 320 | n/a | n/a | +81.0 @558 | +80 @558 |
| full baseline | 640 | n/a | n/a | −64.8 @409 | −64 @409 |
| full baseline | 960 | n/a | n/a | −93.4 @412 | −92 @412 |

The only-ID19 first exact changed rows at x320/640/960 include faint dark pre-onset pixels; its clear colored rise lies at y451/463/457, with major upward finite-difference peaks at y453/465–467/459. Only-ID17's clear rise is y520/516/529, with sharp peaks y521/516/529. At x640, a dark pixel excursion begins as early as y505 and a black pixel occurs at y515 immediately before the y516 jump. The visible alpha/color edge is inferred from that positive jump, **not** from the first arbitrary non-background pixel. For ID17, the y440..480 window has zero luma/G gradient at all three x locations; ID19 is already at its settled `(74,65,48)` color by y500, so its y500..540 window has zero gradient at all three x locations. The two isolated middle-band onsets are therefore distinct and staggered by about 53 rows at x640 (colored onset 463 versus 516), with modest x-dependent edge texture (ID19 451–463; ID17 516–529).

The full baseline is a layered, animated/effected composition and does not present those isolated peaks as a clean pair. Within the ID19-like y440..480 window, the largest absolute baseline luma/G jumps are x320 `+24.3/+24 @478`, x640 `−21.4/−21 @461`, x960 `+7.1/+7 @459`. Within the ID17-like y500..540 window, they are x320 `+10.0/+10 @519`, x640 `−32.4/−32 @501`, x960 `−35.4/−35 @527` (the latter rebounds `+32.5/+32 @529`). The global baseline peaks in the table instead occur at y558, 409, and 412, demonstrating why a naive “largest edge in the whole column” is not an ID17/19 detector. These coincident or nearby baseline gradients are **not causal attribution**: no post-fix hide/composited subtraction was measured here.

## Finding and limit

**Plausible historical explanation:** the authored post-correction isolated ground strips exhibit two reproducible, horizontally broad colored/brightness onsets in the old corrected frame: ID19 around y451–467 (center strongest near 465–467) and ID17 around y516–529 (center at 516). That spacing and placement make the pair a credible explanation for two straight-looking historical middle-band transitions. The result supports an asset/geometry hypothesis; it does not prove that either strip alone causes a particular full-scene line, nor that any current-HEAD line is a defect. The captures predate current HEAD, no matched original-Windows frame exists here, and the full baseline contains other layers and effects. In particular, do not combine these post-V-fix only-ID images with an older pre-fix hide image to infer a current composited delta. The active ID37 lower-foreground lane is outside this review.
