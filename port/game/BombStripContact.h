#pragma once
#include "BombProjectileMotion.h"
#include <algorithm>
#include <cmath>

namespace deathleak::game {
// Source: Projectile.cpp:783-798, SolidLineStrip.cpp:459-516.
// World Y-up, 30x30 zero-offset bomb collider. Pure query: no snap, bounce,
// gravity/fuse mutation, renderer dependency, or launch TTL.
struct BombStripContact {
    using Point = BombProjectileMotion::Point;
    struct Segment { float x1, y1, x2, y2; };
    enum class Ray { None, Forward, Previous, Collider };
    struct Hit {
        Ray ray = Ray::None;
        Point point{0, 0};
        explicit operator bool() const { return ray != Ray::None; }
    };

    static Hit Query(const BombProjectileMotion& bomb, Point previous,
                     Segment line, const BombProjectileMotion::Environment& water) {
        if (water.inWater || water.wasInWaterLastFrame) return {};
        // Match per-segment BoxHitCheck/IsSolidSpriteInRectangle strict AABB.
        // Deliberately NOT an unbounded sweep: source also requires the CURRENT
        // collider to overlap this candidate segment's bounding rectangle.
        const float hw = BombProjectileMotion::ColliderWidth * .5f;
        const float hh = BombProjectileMotion::ColliderHeight * .5f;
        if (!(bomb.x + hw > std::min(line.x1, line.x2) &&
              bomb.x - hw < std::max(line.x1, line.x2) &&
              bomb.y + hh > std::min(line.y1, line.y2) &&
              bomb.y - hh < std::max(line.y1, line.y2))) return {};
        // BombProjectile::Update normalizes its post-gravity/drag velocity
        // before movement. End extends HALF COLLIDER WIDTH on both axes.
        const float speed = std::hypot(bomb.vx, bomb.vy);
        const Point centre{bomb.x, bomb.y};
        const Point end{bomb.x + (speed > 0 ? bomb.vx / speed * hw : 0),
                        bomb.y + (speed > 0 ? bomb.vy / speed * hw : 0)};
        Point intersection{};
        if (Intersect(line, centre, end, intersection)) return {Ray::Forward, intersection};
        // Previous CENTRE to CURRENT forward endpoint (not previous endpoint).
        if (Intersect(line, previous, end, intersection)) return {Ray::Previous, intersection};
        // Original uses X(), not a swept bottom or a floor-height sample.
        if (Intersect(line, {bomb.x, bomb.y + hh}, {bomb.x, bomb.y - hh}, intersection))
            return {Ray::Collider, intersection};
        return {};
    }
private:
    static bool Intersect(Segment line, Point start, Point end, Point& point) {
        const Point a{line.x1, line.y1};
        const Point d{line.x2 - line.x1, line.y2 - line.y1};
        const Point r{end.x - start.x, end.y - start.y};
        const float determinant = d.x * r.y - d.y * r.x;
        // Source rejects parallel/collinear (and zero-length) rays/segments.
        if (std::abs(determinant) <= 1.e-8f) return false;
        const Point delta{start.x - a.x, start.y - a.y};
        const float t = (delta.x * r.y - delta.y * r.x) / determinant;
        const float u = (delta.x * d.y - delta.y * d.x) / determinant;
        if (t < 0 || t > 1 || u < 0 || u > 1) return false;
        point = {a.x + t * d.x, a.y + t * d.y};
        return true;
    }
};
}
