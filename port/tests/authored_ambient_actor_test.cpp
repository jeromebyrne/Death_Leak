#include "../game/AuthoredAmbientActor.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
using Actor = deathleak::game::AuthoredAmbientActor;
constexpr float tick = 1.f / 60.f;
static void Near(float a, float b, float tolerance = .003f) { assert(std::abs(a-b) < tolerance); }
static float Attr(const TiXmlElement* e, const char* attr) {
    assert(e && e->Attribute(attr)); return std::strtof(e->Attribute(attr), nullptr);
}
static Actor::Config Read(const TiXmlElement* e) {
    Actor::Config c;
    std::string kind = e->Value();
    c.kind = kind == "Rabbit" ? Actor::Kind::Rabbit : kind == "Butterfly" ? Actor::Kind::Butterfly :
             kind == "ActiveBird" ? Actor::Kind::ActiveBird : Actor::Kind::AmbientBird;
    auto* p = e->FirstChildElement("position"); c.origin = {Attr(p,"x"),Attr(p,"y")};
    std::string layer = p->Attribute("depth_layer");
    c.background = layer == "kMoon" || layer == "kFarBackground" || layer == "kMiddleBackground" ||
                   layer == "kNearBackground" || layer == "kGroundBack";
    p = e->FirstChildElement("dimensions"); c.dimensions = {Attr(p,"width"),Attr(p,"height")};
    p = e->FirstChildElement("resistance"); c.resistanceX = Attr(p,"x"); c.resistanceY = Attr(p,"y");
    p = e->FirstChildElement("maxvelocity"); c.maxVelocity = {Attr(p,"x"),Attr(p,"y")};
    c.accelX = Attr(e->FirstChildElement("xaccelrate"),"value");
    c.maxJumpSpeed = Attr(e->FirstChildElement("maxjumpspeed"),"value");
    c.applyGravity = std::string(e->FirstChildElement("applygravity")->Attribute("value")) == "true";
    c.horizontalFlip = std::string(e->FirstChildElement("horizontalflip")->Attribute("value")) == "true";
    p = e->FirstChildElement("collisionboxdimensions"); c.collisionHeight = Attr(p,"y"); c.collisionOffsetY = Attr(p,"offsetY");
    if (c.kind == Actor::Kind::AmbientBird || c.kind == Actor::Kind::ActiveBird)
        c.travelOffset = Attr(e->FirstChildElement("travel_offset"),"value");
    return c;
}
static Actor::Config BirdConfig() {
    Actor::Config c; c.origin = {0,100}; c.maxVelocity = {2.5f,5};
    c.resistanceX = .99f; c.resistanceY = 6; c.accelX = .5f;
    return c;
}
static void BirdPhysicsAndPhases() {
    Actor::Environment env;
    auto c = BirdConfig(); Actor a(c);
    auto p = a.Update(tick,env); Near(p.x,0); Near(p.y,100);
    assert(std::string(p.sequence) == "Running" && p.rate == 9 && a.IsGliding());
    p = a.Update(tick,env); Near(p.x,.5f); Near(a.Velocity().x,.495f);
    assert(std::string(p.sequence) == "Still" && p.rate == 1 && !a.IsGliding());
    p = a.Update(tick,env); Near(p.x,1.495f);
    assert(std::string(p.sequence) == "Running");
    // Independent original MovingSprite recurrence: acceleration is per TICK,
    // clamp BEFORE movement, friction AFTER movement. Never convert speed to /s.
    a.Reset(c); float expectedX = 0, vx = 0;
    for (int i=0;i<600;++i) {
        vx = std::min(2.5f, vx + (i == 0 ? 0.f : .5f)); expectedX += vx; vx *= .99f;
        p = a.Update(tick,env); Near(p.x,expectedX,.015f); Near(a.Velocity().x,vx); Near(p.y,100);
    }
    // Flap/glide durations after the two zero-timer initial transitions.
    for (auto kind : {Actor::Kind::AmbientBird, Actor::Kind::ActiveBird}) {
        c.kind = kind; a.Reset(c); a.Update(tick,env); a.Update(tick,env);
        bool previous = false; int duration = 0, transitions = 0;
        for (int i=0;i<1500;++i) {
            a.Update(tick,env); ++duration;
            if (a.IsGliding() != previous) {
                const bool active = kind == Actor::Kind::ActiveBird;
                const float minimum = previous ? (active ? .35f : .9f) : (active ? 1.f : 2.f);
                const float maximum = previous ? (active ? .7f : 1.9f) : (active ? 2.4f : 3.4f);
                assert(duration*tick >= minimum-.02f && duration*tick <= maximum+.02f);
                previous = a.IsGliding(); duration=0; ++transitions;
            }
        }
        assert(transitions > 8);
    }
    c = BirdConfig(); c.travelOffset = -1000; a.Reset(c);
    a.Update(tick,env); p = a.Update(tick,env); Near(p.x,-.5f); assert(p.facing == -1);
    // dt affects displacement, not source acceleration, even at a partial tick.
    a.Reset(c); a.Update(tick,env); p = a.Update(tick*.5f,env); Near(p.x,-.25f);
}
static void RecycleGates() {
    auto c = BirdConfig(); c.maxVelocity.x = 10; c.resistanceX = 1; c.accelX = 1; c.travelOffset = 1;
    Actor a(c); Actor::Environment e; e.hasPlayer=true; e.player={2000,0};
    e.visibilityKnown=true; e.objectInView=e.startInView=false;
    a.Update(tick,e); auto p=a.Update(tick,e); Near(p.x,1); assert(!p.recycled); // strict travel gate
    p=a.Update(tick,e); Near(p.x,0); assert(p.recycled); Near(a.Velocity().x,2); // velocity preserved
    for (int gate=0;gate<5;++gate) {
        a.Reset(c); auto v=e;
        if (gate==0) v.objectInView=true;
        if (gate==1) v.startInView=true;
        if (gate==2) v.hasPlayer=false;
        if (gate==3) v.visibilityKnown=false;
        if (gate==4) v.player.x=1003; // at tick3 x=3, exact 1000 is NOT enough
        a.Update(tick,v); a.Update(tick,v); p=a.Update(tick,v); Near(p.x,3); assert(!p.recycled);
    }
    c.travelOffset=-1; a.Reset(c); a.Update(tick,e); a.Update(tick,e); p=a.Update(tick,e);
    Near(p.x,0); assert(p.recycled);
    // Callback sees POST-physics position, not a cached previous-frame query.
    c.travelOffset=1; a.Reset(c); e.visibilityKnown=false;
    e.objectInViewAt=[](Actor::Point candidate) { return candidate.x <= 2; };
    e.startInViewAt=[](Actor::Point origin) { Near(origin.x,0); return false; };
    a.Update(tick,e); a.Update(tick,e); p=a.Update(tick,e); assert(p.recycled);
}
static Actor::Config RabbitConfig() {
    Actor::Config c; c.kind=Actor::Kind::Rabbit; c.origin={0,16};
    c.maxVelocity={3,100}; c.accelX=1; c.resistanceX=.9f;
    return c;
}
static void RabbitGates() {
    auto c=RabbitConfig(); Actor a(c); Actor::Environment e;
    e.groundHeight=[](float,float){return 0.f;};
    e.hasPlayer=true; e.player={250,16};
    a.Update(tick,e); assert(a.WanderPhase()==Actor::WanderState::Moving); // strict radial gate
    a.Reset(c); e.player={249.99f,16}; a.Update(tick,e);
    assert(a.WanderPhase()==Actor::WanderState::RunningFromPlayer); Near(a.Velocity().x,0);
    a.Update(tick,e); a.Update(tick,e); a.Update(tick,e);
    assert(a.Position().x < 0 && a.Velocity().x < 0); // run away, not follow
    assert(std::abs(a.Velocity().x) <= 2.01f); // source +/-3 is sign-only, not 3x acceleration
    a.Reset(c); a.SetRabbitMode(Actor::RabbitMode::Follow);
    const float stop=a.StopDistance(); assert(stop >= 120 && stop <=219);
    e.player={stop,16}; a.Update(tick,e); a.Update(tick,e); Near(a.Position().x,0); // stop bound strict
    a.Reset(c); a.SetRabbitMode(Actor::RabbitMode::Follow); e.player={500,16};
    a.Update(tick,e); a.Update(tick,e); Near(a.Position().x,0); // radius bound strict
    a.Reset(c); a.SetRabbitMode(Actor::RabbitMode::Follow); e.player={300,16};
    a.Update(tick,e); auto p=a.Update(tick,e); Near(p.x,1); assert(p.facing==1);
    a.Reset(c); a.SetRabbitMode(Actor::RabbitMode::Repel); e.player={20,16};
    a.Update(tick,e); p=a.Update(tick,e); Near(p.x,-1); assert(p.facing==-1); // no stop bound for repel
    a.Reset(c); a.SetRabbitMode(Actor::RabbitMode::Follow); e.player={300,16}; e.playerOnGround=false;
    a.Update(tick,e); assert(!a.IsGrounded()); p=a.Update(tick,e);
    assert(p.y > 16); assert(std::string(p.sequence)=="JumpingUp");
    // Explicit terrain collision geometry, independent of drawable dimensions.
    c.origin.y=80; c.dimensions={800,800}; c.collisionHeight=20; c.collisionOffsetY=-5;
    a.Reset(c); e.hasPlayer=false;
    for(int i=0;i<100;++i) a.Update(tick,e);
    Near(a.Position().y,15); assert(a.IsGrounded());
    // No supplied floor must not cause silent flat-ground placement.
    e.groundHeight={}; a.Reset(c); for(int i=0;i<15;++i)a.Update(tick,e);
    assert(a.Position().y < 0 && !a.IsGrounded());
    // Moving/resting state scheduler actually stops acceleration periodically.
    c=RabbitConfig(); a.Reset(c); e.groundHeight=[](float,float){return 0.f;};
    bool resting=false, movingAgain=false;
    for(int i=0;i<500;++i) { a.Update(tick,e); resting |= a.WanderPhase()==Actor::WanderState::Resting;
        movingAgain |= resting && a.WanderPhase()==Actor::WanderState::Moving; }
    assert(resting && movingAgain);
}
static void ButterflyMotion() {
    Actor::Config c; c.kind=Actor::Kind::Butterfly; c.origin={0,1000};
    c.resistanceX=.99f; c.resistanceY=6; c.maxVelocity={100,100}; c.accelX=99;
    Actor a(c), b(c); Actor::Environment e; bool burst=false, left=false,right=false,fallingSlowAnimation=false;
    float highest=1000,lowest=1000;
    for(int i=0;i<900;++i) {
        auto p=a.Update(tick,e); auto q=b.Update(tick,e);
        Near(p.x,q.x); Near(p.y,q.y); assert(p.facing==q.facing); // deterministic PRIVATE stream
        assert(std::abs(a.Velocity().x) <= .50001f); assert(std::abs(a.Velocity().y) <= 5.001f);
        assert(std::string(p.sequence)=="Still"); assert(p.rate>0 && p.rate<=45);
        burst |= a.Velocity().y > 1; left |= p.facing==-1; right |= p.facing==1;
        fallingSlowAnimation |= p.rate<45;
        highest=std::max(highest,p.y);lowest=std::min(lowest,p.y);
    }
    assert(burst && left && right && fallingSlowAnimation && highest-lowest>1);
    // Strict radial 130 source gate and background immunity to player impulse.
    a.Reset(c); b.Reset(c); e.hasPlayer=true; e.player={130,1000}; a.Update(tick,e);
    Actor::Environment absent; b.Update(tick,absent); a.Update(tick,absent); b.Update(tick,absent);
    Near(a.Position().y,b.Position().y);
    a.Reset(c); e.player={129,1000}; a.Update(tick,e); a.Update(tick,absent); assert(a.Position().y>1000);
    c.background=true; a.Reset(c); b.Reset(c); a.Update(tick,e); b.Update(tick,absent);
    a.Update(tick,absent); b.Update(tick,absent); Near(a.Position().y,b.Position().y);
}
static void AuthoredXmlCoverage() {
    std::map<std::string,int> counts;
    for(const auto& file: std::filesystem::directory_iterator("Game/XmlFiles/levels")) {
        if(file.path().extension()!=".xml") continue;
        TiXmlDocument doc(file.path().c_str()); assert(doc.LoadFile());
        for(auto* e=doc.RootElement()->FirstChildElement();e;e=e->NextSiblingElement()) {
            std::string name=e->Value();
            if(name!="AmbientBird" && name!="Rabbit" && name!="Butterfly" && name!="ActiveBird")continue;
            ++counts[name]; const auto c=Read(e); Actor a(c); Actor::Environment env;
            float before=c.origin.x;
            for(int i=0;i<120;++i) {const auto p=a.Update(tick,env); assert(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.rate));}
            assert(a.Position().x!=before); // all 46 actors move, not frozen drawable art
            if(name=="AmbientBird"||name=="ActiveBird") Near(a.Position().y,c.origin.y);
        }
    }
    assert(counts["AmbientBird"]==33 && counts["Rabbit"]==5 && counts["Butterfly"]==7 && counts["ActiveBird"]==1);
}
int main() {
    BirdPhysicsAndPhases(); RecycleGates(); RabbitGates(); ButterflyMotion(); AuthoredXmlCoverage();
    std::cout << "authored ambient actors: source physics/state/distance/animation gates and all 46 XML actors PASS\n";
}
