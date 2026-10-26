#pragma once
#include "../render/Renderer.h"
#include "InterpolationTopologyOracle.h"
#include <memory>
#include <utility>
#include <vector>

namespace deathleak::linux_port {
// Captures renderer commands in their original painter order. Resource IDs are
// borrowed from the renderer: a packet is valid only while that renderer keeps
// those textures resident. SaveFrame/EndFrame are deliberately not replayed.
class PresentationCommandPacket {
public:
    enum class Kind { Begin, Zoom, Sprite, SpriteBatch, Fill, FillBatch, Postprocess, Resolve };
    enum class PhasePolicy { Static, CameraParallax, AutoScrollY, ShaderClock };
    struct Command {
        explicit Command(Kind value) : kind(value) {}
        Kind kind;
        render::Color color{};
        render::Rect rect{};
        render::Sprite sprite{};
        std::vector<render::Sprite> sprites;
        std::vector<render::Rect> rects;
        float zoom = 1.f;
        TopologyKey key{}; // Diagnostic semantic identity; zero means unowned.
        float cameraFactorX = 1.f, cameraFactorY = 1.f;
        PhasePolicy phase = PhasePolicy::Static;
        float parallaxX = 0.f, parallaxY = 0.f;
    };
    void Clear() { commands_.clear(); sealed_ = false; }
    void Add(Command command) { commands_.push_back(command); }
    void Seal() { sealed_ = true; }
    bool Sealed() const { return sealed_; }
    std::size_t Size() const { return commands_.size(); }
    const std::vector<Command>& Commands() const { return commands_; }
    bool TagLastSprite(TopologyKey key, float fx, float fy,
                       PhasePolicy phase = PhasePolicy::Static, float px = 0.f, float py = 0.f) {
        if (sealed_ || commands_.empty() || commands_.back().kind != Kind::Sprite || !key.object) return false;
        commands_.back().key = key; commands_.back().cameraFactorX = fx; commands_.back().cameraFactorY = fy;
        commands_.back().phase = phase; commands_.back().parallaxX = px; commands_.back().parallaxY = py;
        return true;
    }
    void Replay(render::Renderer& renderer) const {
        if (!sealed_) return;
        for (const auto& command : commands_) {
            switch (command.kind) {
            case Kind::Begin: renderer.BeginFrame(command.color); break;
            case Kind::Zoom: renderer.SetWorldZoom(command.zoom); break;
            case Kind::Sprite: renderer.DrawSprite(command.sprite); break;
            case Kind::SpriteBatch: renderer.DrawSpriteBatch(command.sprites.data(), command.sprites.size()); break;
            case Kind::Fill: renderer.FillRect(command.rect, command.color); break;
            case Kind::FillBatch: renderer.FillRects(command.rects.data(), command.rects.size(), command.color); break;
            case Kind::Postprocess: renderer.ApplyScenePostprocess(); break;
            case Kind::Resolve: renderer.ResolveUnprocessedFrame(); break;
            }
        }
    }
private:
    std::vector<Command> commands_;
    bool sealed_ = false;
};

// Opt-in recording decorator. Replay targets inner_ directly, so replay cannot
// append to or mutate the sealed packet and cannot sample live game state.
class PresentationRecordingRenderer final : public render::Renderer {
public:
    explicit PresentationRecordingRenderer(std::unique_ptr<render::Renderer> inner) : inner_(std::move(inner)) {}
    void SetRequestedRefreshHz(int hz) override { inner_->SetRequestedRefreshHz(hz); }
    bool Initialize(const char* title, int width, int height, bool fullscreen, std::string& error) override {
        return inner_->Initialize(title, width, height, fullscreen, error);
    }
    render::TextureId LoadTexture(const std::string& path, std::string& error) override {
        return inner_->LoadTexture(path, error);
    }
    void BeginFrame(render::Color color) override {
        if (packet_.Sealed()) { previous_ = packet_; hasPrevious_ = true; }
        packet_.Clear(); PresentationCommandPacket::Command command{PresentationCommandPacket::Kind::Begin}; command.color = color; packet_.Add(command); inner_->BeginFrame(color);
    }
    void SetWorldZoom(float zoom) override {
        PresentationCommandPacket::Command command{PresentationCommandPacket::Kind::Zoom};
        command.zoom = zoom; packet_.Add(command); inner_->SetWorldZoom(zoom);
    }
    void DrawSprite(const render::Sprite& sprite) override {
        PresentationCommandPacket::Command command{PresentationCommandPacket::Kind::Sprite};
        command.sprite = sprite; packet_.Add(command); inner_->DrawSprite(sprite);
    }
    void DrawSpriteBatch(const render::Sprite* sprites, std::size_t count) override {
        PresentationCommandPacket::Command command{PresentationCommandPacket::Kind::SpriteBatch};
        command.sprites.assign(sprites, sprites + count); packet_.Add(std::move(command));
        inner_->DrawSpriteBatch(sprites, count);
    }
    void FillRect(render::Rect rect, render::Color color) override {
        PresentationCommandPacket::Command command{PresentationCommandPacket::Kind::Fill};
        command.rect = rect; command.color = color; packet_.Add(command); inner_->FillRect(rect, color);
    }
    void FillRects(const render::Rect* rects, std::size_t count, render::Color color) override {
        PresentationCommandPacket::Command command{PresentationCommandPacket::Kind::FillBatch};
        command.rects.assign(rects, rects + count); command.color = color; packet_.Add(std::move(command));
        inner_->FillRects(rects, count, color);
    }
    void ApplyScenePostprocess() override {
        packet_.Add(PresentationCommandPacket::Command{PresentationCommandPacket::Kind::Postprocess}); inner_->ApplyScenePostprocess();
    }
    void ResolveUnprocessedFrame() override {
        packet_.Add(PresentationCommandPacket::Command{PresentationCommandPacket::Kind::Resolve}); inner_->ResolveUnprocessedFrame();
    }
    void EndFrame() override { packet_.Seal(); inner_->EndFrame(); }
    bool PollQuit() override { return inner_->PollQuit(); }
    bool PresentationSurfaceStable() const override { return inner_->PresentationSurfaceStable(); }
    bool KeyDown(int key) const override { return inner_->KeyDown(key); }
    render::ControllerSticks ControllerAxes() const override { return inner_->ControllerAxes(); }
    render::MouseState Mouse() const override { return inner_->Mouse(); }
    bool SaveFrame(const std::string& path, std::string& error) override { return inner_->SaveFrame(path, error); }
    const PresentationCommandPacket& Packet() const { return packet_; }
    bool TagLastSprite(TopologyKey key, float fx = 1.f, float fy = 1.f,
                       PresentationCommandPacket::PhasePolicy phase = PresentationCommandPacket::PhasePolicy::Static,
                       float px = 0.f, float py = 0.f) { return packet_.TagLastSprite(key, fx, fy, phase, px, py); }
    bool HasPreviousPacket() const { return hasPrevious_; }
    const PresentationCommandPacket& PreviousPacket() const { return previous_; }
    void Replay() const { packet_.Replay(*inner_); }
    void PresentReplay() { inner_->EndFrame(); }
private:
    std::unique_ptr<render::Renderer> inner_;
    PresentationCommandPacket packet_, previous_;
    bool hasPrevious_ = false;
};
} // namespace deathleak::linux_port
