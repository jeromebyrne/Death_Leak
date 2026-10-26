#pragma once
#include <algorithm>
#include <cmath>

namespace deathleak::linux_port {
// The authored scene and UI stay at 1280x720. Output pixels may be larger
// or letterboxed; mouse coordinates arrive in SDL window coordinates.
struct LogicalViewport {
    int x = 0, y = 0, width = 0, height = 0;
    static constexpr int LogicalWidth = 1280, LogicalHeight = 720;
    static LogicalViewport ForSize(int outputWidth, int outputHeight) {
        if (outputWidth <= 0 || outputHeight <= 0) return {};
        const double scale = std::min(double(outputWidth) / LogicalWidth,
                                      double(outputHeight) / LogicalHeight);
        const int width = std::min(outputWidth, int(std::lround(LogicalWidth * scale)));
        const int height = std::min(outputHeight, int(std::lround(LogicalHeight * scale)));
        return {(outputWidth - width) / 2, (outputHeight - height) / 2, width, height};
    }
    struct Point { float x, y; };
    Point WindowToLogical(float windowX, float windowY, int windowWidth, int windowHeight,
                          int outputWidth, int outputHeight) const {
        if (width <= 0 || height <= 0 || windowWidth <= 0 || windowHeight <= 0 ||
            outputWidth <= 0 || outputHeight <= 0) return {-1.f, -1.f};
        const double pixelX = double(windowX) * outputWidth / windowWidth;
        const double pixelY = double(windowY) * outputHeight / windowHeight;
        return {float((pixelX - x) * LogicalWidth / width),
                float((pixelY - y) * LogicalHeight / height)};
    }
};
}
