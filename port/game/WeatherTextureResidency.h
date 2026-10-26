#pragma once

#include "WeatherRain.h"
#include "WeatherLightning.h"
#include <iostream>

namespace deathleak::game {
// Prepare weather art on the fixed-update side of the presentation boundary.
// Only visible families become resident; a failed load retains the old retry
// behavior on a later update. Presentation reads IDs but never performs I/O.
class WeatherTextureResidency {
public:
    void Prepare(render::Renderer& renderer, WeatherRain& rain,
                 bool rainVisible, bool lightningVisible) {
        if (lightningVisible && !lightning_) {
            std::string error;
            lightning_ = renderer.LoadTexture("Game/Media/lightning_layer.png", error);
            if (!lightning_) std::cerr << "Lightning texture load failed: " << error << '\n';
        }
        if (rainVisible && !rain_) {
            std::string error;
            rain_ = renderer.LoadTexture("Game/Media/rain_drop.png", error);
            if (!rain_) std::cerr << "Rain texture load failed: " << error << '\n';
        }
        if (rainVisible) rain.SetTexture(rain_);
    }
    render::TextureId Lightning() const { return lightning_; }
    render::TextureId Rain() const { return rain_; }
private:
    render::TextureId lightning_ = render::InvalidTexture;
    render::TextureId rain_ = render::InvalidTexture;
};
} // namespace deathleak::game
