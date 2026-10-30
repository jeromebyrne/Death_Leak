#pragma once
#include <array>
#include <string_view>
namespace deathleak::game {
struct Technique { const char* id; const char* title; int cost; bool newGamePlus=false; };
// Release prices from FeatureUnlockManager::GetFeatureCost, not debug prices.
inline constexpr std::array<Technique,8> Techniques{{
 {"kDownwardDash","DOWNWARD SMASH",450}, {"kRoll","ROLL",25},
 {"kSlowMotion","FOCUS",600}, {"kDeflection","DEFLECTION",375},
 {"kSpeedIncrease","EXTRA SPEED",200},
 {"kProjectileDamageIncrease","EXTRA DAMAGE",225},
 {"kProjectileSpeedIncrease","EXTRA THROWING SPEED",200},
 {"kBombUnlock","BOMBS",1000,true}
}};
inline const Technique* FindTechnique(std::string_view id) {
 for(const auto& technique:Techniques)if(id==technique.id)return &technique;
 return nullptr;
}
}
