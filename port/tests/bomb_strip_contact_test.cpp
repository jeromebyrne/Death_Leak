#include "../game/BombStripContact.h"
#include <cstdlib>
#include <iostream>

using Motion = deathleak::game::BombProjectileMotion;
using Contact = deathleak::game::BombStripContact;
using Ray = Contact::Ray;
static int checks = 0;
static void check(bool ok, const char* message) {
    ++checks;
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
static Motion at(float x, float y, float vx, float vy) {
    Motion b; b.x=x; b.y=y; b.vx=vx; b.vy=vy; return b;
}
int main() {
    Motion::Environment dry;
    auto b = at(0,0,15,0);
    auto hit = Contact::Query(b, {-1,0}, {10,-50,10,50}, dry);
    check(hit.ray==Ray::Forward && hit.point.x==10 && hit.point.y==0,
          "horizontal bomb hits vertical strip with forward ray");
    b = at(0,0,-15,0);
    check(Contact::Query(b, {1,0}, {-10,50,-10,-50}, dry).ray==Ray::Forward,
          "leftward forward ray handles reversed segment");
    b = at(10,0,15,0);
    check(Contact::Query(b, {-10,0}, {0,-50,0,50}, dry).ray==Ray::Previous,
          "previous centre crosses wall behind current centre");
    b = at(0,10,15,0);
    check(Contact::Query(b, {-10,-10}, {6,4,9,4}, dry).ray==Ray::Previous,
          "previous ray ends at current forward tip, not current centre");
    b = at(0,5,15,0);
    check(Contact::Query(b, {-1,5}, {-20,0,20,0}, dry).ray==Ray::Collider,
          "top-bottom collider ray detects floor without floor crossing");
    b = at(0,10,0,-15);
    check(Contact::Query(b, {0,11}, {-50,-50,50,50}, dry).ray==Ray::Forward,
          "downward approach hits sloped strip");
    b = at(0,-10,0,15);
    check(Contact::Query(b, {0,-11}, {50,50,-50,-50}, dry).ray==Ray::Forward,
          "upward approach from below hits reversed sloped strip");
    b = at(0,5,0,0);
    check(Contact::Query(b, {0,5}, {-20,0,20,0}, dry).ray==Ray::Collider,
          "stationary query has safe zero direction and retains collider ray");
    b = at(0,0,15,0);
    check(!Contact::Query(b, {-1,0}, {15,-50,15,50}, dry),
          "strict source candidate AABB excludes exact tangency");
    b = at(30,0,15,0);
    check(!Contact::Query(b, {-30,0}, {0,-50,0,50}, dry),
          "previous sweep does not bypass source current-collider candidate gate");
    b = at(0,0,0,15);
    check(!Contact::Query(b, {0,-1}, {0,-50,0,50}, dry),
          "parallel collinear rays do not invent vertical strip contact");
    check(!Contact::Query(b, {0,-1}, {0,0,0,0}, dry),
          "degenerate segment does not collide");
    b = at(0,0,15,0);
    Motion::Environment wet; wet.inWater=true;
    check(!Contact::Query(b, {-1,0}, {10,-50,10,50}, wet), "current water excludes geometry");
    wet.inWater=false; wet.wasInWaterLastFrame=true;
    check(!Contact::Query(b, {-1,0}, {10,-50,10,50}, wet), "previous water excludes geometry");
    check(!b.OnContact(Motion::Contact::SolidLine, 1, wet) && b.active,
          "water also excludes contact fuse mutation");

    // Follow actual integration order: update, query, then contact. Wall is not
    // a box/floor surrogate. This is the T05 regression with real tick velocity.
    Motion flying(0,0,1,0);
    flying.Update(0.f, 1000, dry);
    check(flying.active && !flying.HasExploded(), "no launch TTL even after long active flight");
    Motion::Point previous{flying.x,flying.y};
    flying.Update(1.f/60.f, 1001, dry);
    check(bool(Contact::Query(flying, previous, {20,-50,20,50}, dry)),
          "post-gravity direction still hits authored vertical wall");
    const float stopX=flying.x, stopY=flying.y;
    check(flying.OnContact(Motion::Contact::SolidLine, 1001, dry), "verified hit deactivates");
    check(!flying.active && flying.vx==0 && flying.vy==0 && flying.InactiveSince()==1001,
          "first contact records fuse and zeros velocity without bounce");
    check(!flying.OnContact(Motion::Contact::SolidLine, 1003, dry) && flying.InactiveSince()==1001,
          "repeat contact does not restart fuse");
    flying.Update(1.f/60.f, 1006, dry);
    check(!flying.HasExploded() && flying.x==stopX && flying.y==stopY,
          "strict fuse boundary and stopped position/gravity preserved");
    flying.Update(1.f/60.f, 1006.0001, dry);
    check(flying.HasExploded() && flying.TakeExplosion() && !flying.TakeExplosion(),
          "strictly later than contact plus five emits exactly one explosion");
    std::cout << checks << " bomb strip/fuse checks passed\n";
}
