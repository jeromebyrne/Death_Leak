#include "../game/AuthoredPlatformMotion.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
using Motion = deathleak::game::AuthoredPlatformMotion;
using Point = Motion::Point;
static constexpr float tick = 1.f / 60.f;
static void Near(float a, float b) { assert(std::abs(a - b) < .002f); }
static float Attr(const TiXmlElement* e, const char* name) {
    const char* value = e->Attribute(name); assert(value); return std::strtof(value, nullptr);
}
static Motion::Config Read(const TiXmlElement* e) {
    Motion::Config c;
    c.kind = std::string(e->Value()) == "PathingPlatform" ? Motion::Kind::Pathing : Motion::Kind::Falling;
    const auto* p = e->FirstChildElement("position"); c.origin = {Attr(p,"x"), Attr(p,"y")};
    p = e->FirstChildElement("maxvelocity"); c.maxVelocity = {Attr(p,"x"), Attr(p,"y")};
    p = e->FirstChildElement("resistance"); c.resistanceX = Attr(p,"x"); c.resistanceY = Attr(p,"y");
    c.alpha = Attr(e->FirstChildElement("alpha"), "value");
    p = e->FirstChildElement("sine_wave_props"); c.sineActive = std::string(p->Attribute("active")) == "true";
    c.sineOffsetX = Attr(p,"x_offset"); c.sineOffsetY = Attr(p,"y_offset"); c.sineSpeed = Attr(p,"amplitude");
    if (c.kind == Motion::Kind::Falling) {
        c.maxTriggerTime = Attr(e->FirstChildElement("maxtriggertime"),"value");
        c.timeUntilReturn = Attr(e->FirstChildElement("timeuntilreturn"),"value");
    } else {
        p = e->FirstChildElement("platformspeed"); c.speed = {Attr(p,"x"), Attr(p,"y")};
        c.pathWhenTriggered = std::string(e->FirstChildElement("platformpathtype")->Attribute("value")) == "pathwhentriggered";
        c.pathForward = std::string(e->FirstChildElement("platformpathstate")->Attribute("value")) != "pathbackward";
        for (p = e->FirstChildElement("pathpoints")->FirstChildElement(); p; p = p->NextSiblingElement())
            c.points.push_back({c.origin.x + Attr(p,"x"), c.origin.y + Attr(p,"y")});
    }
    return c;
}
static const TiXmlElement* Find(TiXmlDocument& d, const char* name) {
    // Level files are a flat <GameObjects> root; avoid assuming its spelling.
    for (auto* e = d.RootElement()->FirstChildElement(); e; e = e->NextSiblingElement())
        if (std::string(e->Value()) == name) return e;
    assert(false); return nullptr;
}
static void PathTests() {
    Motion::Config c; c.kind = Motion::Kind::Pathing; c.maxVelocity = {100,100};
    c.speed = {12,6}; c.points = {{0,0},{100,100},{200,0}};
    Motion m(c); m.Update(tick,0); assert(m.TargetIndex() == 1); Near(m.Position().x,0);
    m.Update(tick,tick); Near(m.Position().x,0); // base runs BEFORE steering
    Near(m.Velocity().x,12 / std::sqrt(2.f)); Near(m.Velocity().y,6 / std::sqrt(2.f));
    m.Update(tick,2*tick); assert(m.Position().x > 8); assert(m.Position().y > 4);
    c.pathForward = false; m.Reset(c); m.Update(tick,0); assert(m.TargetIndex() == 2);
    c.pathForward = true; c.points = {{10,0},{100,0}}; m.Reset(c);
    m.Update(tick,0); assert(m.TargetIndex() == 0); Near(m.Velocity().x,12); // strict radius
    m.Update(tick,tick); assert(m.TargetIndex() == 1); Near(m.Position().x,12); // no snap
    c.points = {{100,0}}; c.speed = {150,150}; c.maxVelocity = {150,150}; m.Reset(c);
    m.Update(tick,0); m.Update(tick,tick); Near(m.Position().x,150);
    m.Update(tick,2*tick); Near(m.Position().x,0); // overshoot reverses then loops
    assert(m.TargetIndex() == 0); // single-point loops are bounded
    c.points = {{100,0},{200,0},{300,0}}; c.pathForward = false;
    c.pathWhenTriggered = true; m.Reset(c); m.OnPlayerCollision(true,0);
    m.Update(tick,0,false); assert(m.PathPhase() == Motion::PathState::Returning);
    assert(m.TargetIndex() == 1); // backward journey returns by advancing indices
    m.OnPlayerCollision(true,tick); assert(m.TargetIndex() == 0);
    assert(m.PathPhase() == Motion::PathState::Backward);
    c.pathForward = true; c.pathWhenTriggered = false;
    c.points.clear(); m.Reset(c); m.Update(tick,0); Near(m.Position().x,0);
    c.points = {{0,0},{100,0}}; c.pathWhenTriggered = true; c.speed = {7,7};
    m.Reset(c); m.Update(tick,0); assert(m.PathPhase() == Motion::PathState::NotPathing);
    m.OnPlayerCollision(true,0); m.Update(tick,0,true); assert(m.TargetIndex() == 1);
    m.Update(tick,tick,true); m.Update(tick,2*tick,true); Near(m.Position().x,7);
    m.Update(tick,3*tick,false); assert(m.PathPhase() == Motion::PathState::Returning); assert(m.TargetIndex() == 0);
    m.OnPlayerCollision(true,4*tick); assert(m.PathPhase() == Motion::PathState::Forward); assert(m.TargetIndex() == 1);
    // Return-to-start keeps velocity for a transition tick, then NotPathing zeros it.
    m.Reset(c); m.OnPlayerCollision(true,0); m.Update(tick,0,true);
    m.Update(tick,tick,false); m.Update(tick,2*tick,false);
    assert(m.PathPhase() == Motion::PathState::NotPathing);
}
static void FallingTests() {
    assert(Motion::PlayerCanTrigger(0,1,0,-1,1,0));
    assert(!Motion::PlayerCanTrigger(-1,1,0,-1,1,0));
    assert(!Motion::PlayerCanTrigger(0,0,0,-1,1,0));
    assert(!Motion::PlayerCanTrigger(0,1,.01f,-1,1,0));
    Motion::Config c; c.kind = Motion::Kind::Falling; c.origin = {10,200};
    c.maxVelocity = {10,-1}; c.resistanceY = 2; c.maxTriggerTime = 1; c.timeUntilReturn = 1;
    c.sineActive = true; c.sineSpeed = 10; c.sineOffsetX = .65f; c.sineOffsetY = -.5f;
    Motion m(c); m.Update(tick,0); Near(m.Position().y,200 - .5f * std::sin(10*tick));
    m.OnPlayerCollision(true,0); assert(!m.SineActive());
    m.Update(tick,.25); Near(m.Alpha(),.5f); m.Update(tick,.5); Near(m.Alpha(),1);
    m.Update(tick,.75); Near(m.Alpha(),.5f); m.Update(tick,1); Near(m.Alpha(),1);
    assert(m.FallPhase() == Motion::FallState::Triggered); // strict deadline
    m.OnPlayerCollision(true,1); // no timer restart
    const float before = m.Position().y;
    m.Update(tick,1.01); assert(m.FallPhase() == Motion::FallState::Falling); Near(m.Position().y,before);
    m.Update(tick,1.02); Near(m.Position().y,before-1.5f); // delayed gravity, resistanceY=2
    m.Update(tick,2.01); assert(m.FallPhase() == Motion::FallState::Falling);
    m.Update(tick,2.02); assert(m.FallPhase() == Motion::FallState::Returning);
    const auto start = m.Position(); Near(m.Velocity().y,0);
    m.Update(tick,3.02); Near(m.Position().y,(start.y+200)/2);
    m.Update(tick,4.011); assert(m.FallPhase() == Motion::FallState::Static);
    Near(m.Position().y,200); assert(!m.SineActive()); // no re-enable after return
    m.Reset(c); assert(m.SineActive()); Near(m.Alpha(),1); Near(m.Position().y,200);
    m.OnExplosion(0); assert(m.SineActive()); // deliberately asymmetric source behavior
    const auto p = m.Position(); m.Update(0,9); Near(m.Position().y,p.y);
    // Gravity's threshold is strict: at exactly -15 the NEXT acceleration
    // remains the 3x fall-start value; below -15 it becomes ordinary gravity.
    c.sineActive = false; c.resistanceY = 1; c.maxTriggerTime = 0; c.timeUntilReturn = 100;
    m.Reset(c); m.OnPlayerCollision(true,0); m.Update(tick,.01);
    for (int i = 0; i < 6; ++i) m.Update(tick,.02 + i*tick);
    Near(m.Velocity().y,-18);
    m.Update(tick,.2); Near(m.Velocity().y,-19);
    c.maxTriggerTime = 0; c.sineActive = false; m.Reset(c); m.OnPlayerCollision(true,0);
    m.Update(tick,0); assert(std::isfinite(m.Alpha())); m.Update(tick,.01);
    assert(m.FallPhase() == Motion::FallState::Falling);
}
static void AuthoredTests() {
    TiXmlDocument sea("Game/XmlFiles/levels/sea_2.xml"); assert(sea.LoadFile());
    auto c = Read(Find(sea,"PathingPlatform"));
    assert(c.pathWhenTriggered && c.points.size() == 1); Near(c.maxVelocity.y,.8f); // source atof prefix
    Near(c.points[0].x - c.origin.x,-999999); Motion boat(c);
    boat.Update(tick,0); Near(boat.Position().x,c.origin.x);
    boat.OnPlayerCollision(true,0); boat.Update(tick,tick,true); Near(boat.Velocity().x,-1);
    boat.Update(tick,2*tick,true); Near(boat.Delta().x,-1); // maxvelocity=1, not speed=2
    TiXmlDocument cave("Game/XmlFiles/levels/cave_platforming_1.xml"); assert(cave.LoadFile());
    int count = 0;
    for (auto* e = cave.RootElement()->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::string(e->Value()) != "FallingPlatform") continue;
        ++count; c = Read(e); assert(c.sineActive); Near(c.timeUntilReturn,9999);
        Motion stone(c); stone.OnPlayerCollision(true,0); stone.Update(tick,c.maxTriggerTime+0.01);
        assert(stone.FallPhase() == Motion::FallState::Falling);
        stone.Update(tick,10); assert(stone.FallPhase() == Motion::FallState::Falling);
    }
    assert(count == 8);
    TiXmlDocument test("Game/XmlFiles/levels/test.xml"); assert(test.LoadFile());
    c = Read(Find(test,"PathingPlatform")); assert(c.points.size() == 4);
    Near(c.points[2].x-c.origin.x,750); Near(c.points[2].y-c.origin.y,1000);
    c = Read(Find(test,"FallingPlatform")); Near(c.alpha,.8f); Near(c.timeUntilReturn,5);
}
int main() { PathTests(); FallingTests(); AuthoredTests(); std::cout << "authored platform motion: PASS (source fixtures + boundary cases)\n"; }
