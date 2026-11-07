#pragma once
// Test-only no-pixel renderer. Inject with -include when compiling main.cpp;
// intentionally omit SDLRenderer.cpp so this factory is the sole definition.
#include "port/render/Renderer.h"
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace deathleak::render {
class FastFadeCpuRenderer final : public Renderer {
    std::unordered_map<std::string, TextureId> ids_;
public:
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    TextureId LoadTexture(const std::string& path, std::string& error) override {
        if (!std::filesystem::exists(path)) { error = "Missing test asset: " + path; return InvalidTexture; }
        auto [it, inserted] = ids_.try_emplace(path, static_cast<TextureId>(ids_.size() + 1));
        return it->second;
    }
    void BeginFrame(Color) override {}
    void DrawSprite(const Sprite&) override {}
    void FillRect(Rect, Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string& error) override {
        error = "No-pixel CPU renderer cannot save a frame"; return false;
    }
};
inline std::unique_ptr<Renderer> CreateRenderer(bool) {
    return std::make_unique<FastFadeCpuRenderer>();
}
}
