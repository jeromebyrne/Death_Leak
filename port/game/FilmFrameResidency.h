#pragma once

#include "../render/Renderer.h"
#include <array>
#include <string>

namespace deathleak::game {
// The authored film overlay first loads all four frames together and treats
// frame zero as its retry sentinel. Prepare belongs to update/init, never draw.
class FilmFrameResidency {
public:
    void Prepare(render::Renderer& renderer, std::string& error) {
        if (frames_[0]) return;
        constexpr std::array<const char*, 4> paths = {
            "Game/Media/UI/film_frame.png", "Game/Media/UI/film_frame_2.png",
            "Game/Media/UI/film_frame_3.png", "Game/Media/UI/film_frame_4.png"};
        for (std::size_t i = 0; i < paths.size(); ++i)
            frames_[i] = renderer.LoadTexture(paths[i], error);
    }
    render::TextureId Frame(std::size_t index) const { return frames_[index]; }
    std::size_t Size() const { return frames_.size(); }
private:
    std::array<render::TextureId, 4> frames_{};
};
} // namespace deathleak::game
