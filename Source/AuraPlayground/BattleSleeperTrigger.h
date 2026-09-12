#pragma once
#include <algorithm>

// One chance per approach, with a wider exit distance to prevent boundary jitter.
struct FBattleSleeperTrigger {
 float CooldownSeconds=0;
 bool bArmed=true;
 bool Observe(float Distance,float Dt,bool Eligible){
  CooldownSeconds=std::max(0.f,CooldownSeconds-std::max(0.f,Dt));
  if(Distance>750.f)bArmed=true;
  if(Distance>450.f||!bArmed)return false;
  bArmed=false;
  return Eligible&&CooldownSeconds<=0;
 }
 void Attempted(bool Woke){CooldownSeconds=Woke?90.f:30.f;}
};
