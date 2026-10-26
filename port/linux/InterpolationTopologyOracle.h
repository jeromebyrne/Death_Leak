#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <vector>

namespace deathleak::linux_port {
// A CPU-only admission contract. Painter keys identify objects, not command
// positions; particles need a generation when a slot is reused.
struct TopologyKey {
    std::uint64_t object = 0;
    std::uint32_t generation = 0;
    bool operator<(const TopologyKey& rhs) const {
        return object == rhs.object ? generation < rhs.generation : object < rhs.object;
    }
    bool operator==(const TopologyKey& rhs) const {
        return object == rhs.object && generation == rhs.generation;
    }
};
struct TopologyItem {
    TopologyKey key;
    int painterOrdinal = 0;
    bool visible = false;
    float worldX = 0, worldY = 0;
    float cameraFactorX = 1, cameraFactorY = 1;
};
struct TopologySnapshot {
    std::uint64_t sceneEpoch = 0;
    float cameraX = 0, cameraY = 0;
    bool discontinuity = false;
    std::vector<TopologyItem> items; // actual painter order, invisible identities retained
};
enum class TopologyDecision { Interpolate, HoldCurrent, Invalid };
struct TopologyResult {
    TopologyDecision decision = TopologyDecision::Invalid;
    std::vector<TopologyItem> sample;
};
inline bool ValidTopology(const TopologySnapshot& s) {
    if (!std::isfinite(s.cameraX) || !std::isfinite(s.cameraY)) return false;
    std::set<TopologyKey> keys;
    int last = 0;
    bool first = true;
    for (const auto& item : s.items) {
        if (!item.key.object || !keys.insert(item.key).second ||
            (!first && item.painterOrdinal < last) ||
            !std::isfinite(item.worldX) || !std::isfinite(item.worldY) ||
            !std::isfinite(item.cameraFactorX) || !std::isfinite(item.cameraFactorY)) return false;
        first = false;
        last = item.painterOrdinal;
    }
    return true;
}
inline TopologyResult SampleTopology(const TopologySnapshot& previous,
                                     const TopologySnapshot& current, float alpha) {
    if (!ValidTopology(previous) || !ValidTopology(current) ||
        !std::isfinite(alpha) || alpha < 0 || alpha > 1) return {};
    TopologyResult result;
    result.sample = current.items;
    result.decision = TopologyDecision::HoldCurrent;
    for (auto& item : result.sample) {
        item.worldX -= current.cameraX * item.cameraFactorX;
        item.worldY -= current.cameraY * item.cameraFactorY;
    }
    // Epoch/camera cuts never smear a previous scene across a new scene.
    if (previous.sceneEpoch != current.sceneEpoch || current.discontinuity ||
        previous.discontinuity) return result;
    std::set<TopologyKey> priorKeys;
    for (const auto& item : previous.items) priorKeys.insert(item.key);
    // A birth, death, visibility or painter-order change holds the whole family
    // on the current snapshot. Partial lerp would detach attached effects.
    if (priorKeys.size() != current.items.size()) return result;
    for (std::size_t i = 0; i < current.items.size(); ++i) {
        const auto& a = previous.items[i];
        const auto& b = current.items[i];
        // Unmarked teleport or camera reset is not a continuous sample.
        if (std::fabs(b.worldX - a.worldX) > 250.f ||
            std::fabs(b.worldY - a.worldY) > 250.f ||
            std::fabs(current.cameraX - previous.cameraX) > 250.f ||
            std::fabs(current.cameraY - previous.cameraY) > 250.f) return result;
        if (!(a.key == b.key) || a.painterOrdinal != b.painterOrdinal ||
            a.visible != b.visible || a.cameraFactorX != b.cameraFactorX ||
            a.cameraFactorY != b.cameraFactorY) return result;
    }
    result.decision = TopologyDecision::Interpolate;
    const float cameraX = previous.cameraX + (current.cameraX - previous.cameraX) * alpha;
    const float cameraY = previous.cameraY + (current.cameraY - previous.cameraY) * alpha;
    for (std::size_t i = 0; i < result.sample.size(); ++i) {
        auto& out = result.sample[i];
        const auto& a = previous.items[i];
        const auto& b = current.items[i];
        // World coordinates are kept here; projection must consume the same
        // interpolated camera for ordinary and display-only presents.
        out.worldX = a.worldX + (b.worldX - a.worldX) * alpha;
        out.worldY = a.worldY + (b.worldY - a.worldY) * alpha;
        out.worldX -= cameraX * out.cameraFactorX;
        out.worldY -= cameraY * out.cameraFactorY;
    }
    return result;
}
}
