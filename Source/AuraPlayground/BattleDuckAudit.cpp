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
  ABattleDuck* Settled=nullptr;
  for(const auto& Duck:Flock->Ducks)if(IsValid(Duck)&&Duck->State==1){Settled=Duck;break;}
  if(!Settled){Report(false,TEXT("no duck settled on the water"),nullptr);return;}
  auto* Person=Cast<APiedmontExplorer>(PC->GetPawn());
  if(!Person){if(S.Clock>10.f)Report(false,TEXT("rider never left the bike"),nullptr);return;}
  Person->SetActorLocation(Settled->GetActorLocation()-FVector(0,0,30),false,nullptr,ETeleportType::TeleportPhysics);
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
  ABattleDuck* Target=nullptr;
  float WaterZ=0;
  for(const auto& Duck:Flock->Ducks)if(IsValid(Duck)){Target=Duck;WaterZ=Duck->WaterZ;break;}
  if(Person&&Target){
   S.Lowest=FMath::Min(S.Lowest,float(Person->GetActorLocation().Z-WaterZ));
   const FVector Duck=Target->GetActorLocation();
   if(FVector::Dist2D(Person->GetActorLocation(),Duck)<200.f)
    Person->SetActorLocation(FVector(Duck.X,Duck.Y,WaterZ-10),false,nullptr,ETeleportType::TeleportPhysics);
  }
  if(S.Clock>9.f){
   const bool Ducked=S.Lowest<10.f;
   const bool Charged=Mode->LastTimeDelta==-5.f&&Mode->TimeNotice==TEXT("DUCK!");
   Report(Ducked&&Charged,Ducked?(Charged?TEXT("swimmer dunked and charged for the duck"):TEXT("time penalty did not fire")):TEXT("swimmer was never dunked"),Target);
  }
  return;
 }
#endif
}
