#pragma once
// Compiled into the test binary only. Delegates all renderer work and observes
// the production scene's actual DrawSprite texture command before presentation.
#include "../render/Renderer.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <unordered_map>

class BodySlowFrameObserver final : public deathleak::render::Renderer {
    std::unique_ptr<deathleak::render::Renderer> inner;
    std::unordered_map<deathleak::render::TextureId, std::string> paths;
    int ordinal = -1, bodyCount = 0, updateCount = 0, groundedCount = 0;
    float raw = 0, modified = 0, prior = 0;
    std::string bodyPath;
public:
    explicit BodySlowFrameObserver(std::unique_ptr<deathleak::render::Renderer> renderer)
        : inner(std::move(renderer)) {}
    void Start(int frame, float requested, float current, float preceding) {
        ordinal = frame; raw = requested; modified = current; prior = preceding;
        bodyCount = 0; updateCount = 0; groundedCount = 0; bodyPath.clear();
    }
    void BodyUpdate(bool grounded) { ++updateCount; if (grounded) ++groundedCount; }
    bool Initialize(const char* title, int w, int h, std::string& error) override {
        return inner->Initialize(title, w, h, error);
    }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string& error) override {
        const auto id = inner->LoadTexture(path, error);
        if (id) paths[id] = path;
        return id;
    }
    void BeginFrame(deathleak::render::Color color) override { inner->BeginFrame(color); }
    void SetWorldZoom(float zoom) override { inner->SetWorldZoom(zoom); }
    void DrawSprite(const deathleak::render::Sprite& sprite) override {
        auto it = paths.find(sprite.texture);
        if (it != paths.end() && it->second.find("Game/Media/characters/player/idle_default/idle_default_") == 0) {
            ++bodyCount; bodyPath = it->second;
        }
        inner->DrawSprite(sprite);
    }
    void FillRect(deathleak::render::Rect rect, deathleak::render::Color color) override { inner->FillRect(rect, color); }
    void ApplyScenePostprocess() override { inner->ApplyScenePostprocess(); }
    void EndFrame() override {
        inner->EndFrame();
        std::cout << std::setprecision(9) << "BODY_COMMAND ordinal=" << ordinal
                  << " raw=" << raw << " modified=" << modified << " prior=" << prior
                  << " eligible=" << updateCount << " grounded=" << groundedCount << " count=" << bodyCount
                  << " path=" << (bodyPath.empty() ? "NONE" : bodyPath) << '\n';
    }
    bool PollQuit() override { return inner->PollQuit(); }
    bool KeyDown(int key) const override { return inner->KeyDown(key); }
    deathleak::render::ControllerSticks ControllerAxes() const override { return inner->ControllerAxes(); }
    deathleak::render::MouseState Mouse() const override { return inner->Mouse(); }
    bool SaveFrame(const std::string& path, std::string& error) override { return inner->SaveFrame(path, error); }
};
