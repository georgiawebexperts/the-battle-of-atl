#include "BattleDuck.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontBike.h"
#include "PiedmontExplorer.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

/**
 * Proves the ducks really settle on Lake Clara Meer and that a swimmer who runs
 * into one is dunked and charged time, which is what the design asks for.
 */
void TickBattleDuckAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int Phase=0,OverWater=0;float Clock=0,Waited=0,Lowest=BIG_NUMBER;};
 static FState S;
 if(!PC||!PC->GetWorld()||PC->GetWorld()->GetTimeSeconds()<5)return;
 ABattleDuckFlock* Flock=nullptr;
 for(TActorIterator<ABattleDuckFlock> It(PC->GetWorld());It;++It)if(!Flock)Flock=*It;
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(PC));
 if(!Flock||!Mode){S.Waited+=Dt;if(S.Waited>40.f){UE_LOG(LogTemp,Display,TEXT("BattleDuckAudit: {\"passed\":false,\"reason\":\"no duck flock spawned\"}"));PC->ConsoleCommand(TEXT("quit"));}return;}
 auto Report=[&](bool Pass,const TCHAR* Why,ABattleDuck* Duck){
  UE_LOG(LogTemp,Display,TEXT("BattleDuckAudit: {\"passed\":%s,\"reason\":\"%s\",\"ducks\":%d,\"over_water\":%d,\"bumps\":%d,\"time_delta\":%.1f,\"notice\":\"%s\",\"lowest_offset_cm\":%.1f}"),
   Pass?TEXT("true"):TEXT("false"),Why,Flock->Ducks.Num(),S.OverWater,Duck?Duck->BumpsGiven:0,Mode->LastTimeDelta,*Mode->TimeNotice,
   S.Lowest<BIG_NUMBER?S.Lowest:-1.f);
  PC->ConsoleCommand(TEXT("quit"));
 };
 S.Clock+=Dt;
 if(S.Phase==0){
  if(Flock->Ducks.Num()<4){if(S.Waited+S.Clock>35.f)Report(false,TEXT("fewer than four ducks"),nullptr);return;}
  APiedmontWaterHazard* Lake=nullptr;
  for(TActorIterator<APiedmontWaterHazard> It(PC->GetWorld());It;++It)if(!Lake&&It->Polygon.Num()>=3)Lake=*It;
  if(Lake){
   S.OverWater=0;
   for(const auto& Duck:Flock->Ducks)if(IsValid(Duck)&&Lake->ContainsBike(FVector(Duck->GetActorLocation().X,Duck->GetActorLocation().Y,Duck->WaterZ)))++S.OverWater;
  }
  if(S.OverWater<3){Report(false,TEXT("fewer than three ducks landed on open water"),nullptr);return;}
  if(auto* Bike=Cast<ABattleBike>(PC->GetPawn()))Bike->Dismount();
  S.Clock=0;S.Phase=1;return;
 }
 if(S.Phase==1){
  if(S.Clock<1.5f)return;
  // Pick a settled duck that is provably over open water, not simply the first
  // one that settled. Phase 0 already trusts ContainsBike for its over_water
  // count, but this used to take whatever duck settled first, so when that one
  // was near the shore the rider was dropped outside the hazard and never began
  // swimming. That is the coin flip this audit has been showing: a pass reads
  // lowest_offset_cm -95, a failure -1 with the rider stopped at the water's
  // edge. Measured 2026-09-18 across a sweep and three standalone runs.
  APiedmontWaterHazard* Lake=nullptr;
  for(TActorIterator<APiedmontWaterHazard> It(PC->GetWorld());It;++It)if(!Lake&&It->Polygon.Num()>=3)Lake=*It;
  ABattleDuck* Settled=nullptr;
  for(const auto& Duck:Flock->Ducks)if(IsValid(Duck)&&Duck->State==1&&(!Lake||Lake->ContainsBike(FVector(Duck->GetActorLocation().X,Duck->GetActorLocation().Y,Duck->WaterZ)))){Settled=Duck;break;}
  // Ducks settle one at a time, so wait for one over open water rather than
  // failing the moment none has qualified yet.
  if(!Settled){if(S.Clock>8.f)Report(false,TEXT("no duck settled on open water"),nullptr);return;}
  auto* Person=Cast<APiedmontExplorer>(PC->GetPawn());
  if(!Person){if(S.Clock>10.f)Report(false,TEXT("rider never left the bike"),nullptr);return;}
  // Drop him relative to the WATER, not to the duck. A duck's origin floats about
  // 30 cm above the surface, so "30 cm below the duck" put the swimmer exactly at
  // the waterline: `lowest_offset_cm -1.0`, bSwimming never set, and the audit
  // reported "rider never started swimming in the lake" on roughly every other
  // run. WaterZ is the surface, so this lands him under it every time.
  Person->SetActorLocation(FVector(Settled->GetActorLocation().X,Settled->GetActorLocation().Y,Settled->WaterZ-30.f),false,nullptr,ETeleportType::TeleportPhysics);
  S.Clock=0;S.Phase=2;return;
 }
 if(S.Phase==2){
  auto* Person=Cast<APiedmontExplorer>(PC->GetPawn());
  if(Person&&Person->bSwimming){S.Clock=0;S.Phase=3;return;}
  if(S.Clock>5.f)Report(false,TEXT("rider never started swimming in the lake"),nullptr);
  return;
 }
 if(S.Phase==3){
  auto* Person=Cast<APiedmontExplorer>(PC->GetPawn());
  // The duck that actually bumped him, not whichever duck happens to be first in
  // the flock: the dunk teleport below only fires within 200 cm of this one, and
  // the old first-in-list choice is what left `lowest_offset_cm` at +15 instead of
  // under the surface when the flock had drifted apart.
  ABattleDuck* Target=nullptr;
  float WaterZ=0;
  for(const auto& Duck:Flock->Ducks)if(IsValid(Duck)&&Duck->BumpsGiven>0){Target=Duck;WaterZ=Duck->WaterZ;break;}
  if(!Target)for(const auto& Duck:Flock->Ducks)if(IsValid(Duck)){Target=Duck;WaterZ=Duck->WaterZ;break;}
  if(Person&&Target){
   S.Lowest=FMath::Min(S.Lowest,float(Person->GetActorLocation().Z-WaterZ));
   const FVector Duck=Target->GetActorLocation();
   if(FVector::Dist2D(Person->GetActorLocation(),Duck)<200.f)
    Person->SetActorLocation(FVector(Duck.X,Duck.Y,WaterZ-10),false,nullptr,ETeleportType::TeleportPhysics);
  }
  if(S.Clock>9.f){
   // Dunked means the duck's own record of the drop, not this actor's sampling of
   // the swimmer's height: the teleport happens inside the duck's tick and the
   // swimmer is back at the surface by the time this tick looks.
   const float Dunk=Target?Target->LastDunkDepthCm:0.f;
   const bool Ducked=Dunk>=100.f;
   const bool Charged=Mode->LastTimeDelta==-5.f&&Mode->TimeNotice==TEXT("DUCK!");
   UE_LOG(LogTemp,Display,TEXT("BattleDuckDunk: duck=%s depth_cm=%.1f sampled_lowest_cm=%.1f"),Target?*Target->GetName():TEXT("none"),Dunk,S.Lowest<BIG_NUMBER?S.Lowest:-1.f);
   Report(Ducked&&Charged,Ducked?(Charged?TEXT("swimmer dunked and charged for the duck"):TEXT("time penalty did not fire")):TEXT("swimmer was never dunked"),Target);
  }
  return;
 }
#endif
}
