#pragma once

namespace deathleak::linux_port {
// Snapshot of one fullscreen presentation surface. Focus is intentionally not
// required for a diagnostic capture; dimensions and mode must still match.
struct DisplaySurfaceGeometry {
    int requestedWidth = 0, requestedHeight = 0, requestedRefreshHz = 0;
    int outputWidth = 0, outputHeight = 0;
    int windowWidth = 0, windowHeight = 0;
    int modeWidth = 0, modeHeight = 0, modeRefreshHz = 0;
    bool fullscreen = false, minimized = false;
    bool Matches() const {
        return fullscreen && !minimized && requestedWidth > 0 && requestedHeight > 0 &&
               outputWidth == requestedWidth && outputHeight == requestedHeight &&
               windowWidth == requestedWidth && windowHeight == requestedHeight &&
               modeWidth == requestedWidth && modeHeight == requestedHeight &&
               (!requestedRefreshHz || modeRefreshHz == requestedRefreshHz);
    }
};
}
