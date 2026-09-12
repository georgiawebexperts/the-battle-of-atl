#include "PiedmontPedestrian.h"
#include "PiedmontBike.h"
#include "BattleParkFurniture.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
void TickBattleBenchReachAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> Visitor;float Clock=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BenchReachAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Reason);PC->ConsoleCommand(TEXT("quit"));};
#define REQUIRE_REACH(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 REQUIRE_REACH(S.Clock<15,"Timed out");
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(PC));REQUIRE_REACH(Mode,"No mode");
 if(S.Phase==0){
  for(TActorIterator<ABattleParkFurniture> It(PC->GetWorld());It;++It)if(It->Benches.Num()){
   const auto T=It->Benches[0];FTransform Spawn(T.TransformVectorNoScale(FVector(0,-1,0)).Rotation(),T.TransformPosition(FVector(0,80,90)));
   auto* V=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),Spawn,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
   V->CityAppearanceVariant=0;UGameplayStatics::FinishSpawningActor(V,Spawn);V->PauseRemaining=30;S.Visitor=V;break;
  }
  auto* V=S.Visitor.Get();REQUIRE_REACH(V,"No visitor");
  REQUIRE_REACH(V->BeginBenchReach(),"Initial reach failed");
  REQUIRE_REACH(!V->BeginBenchReach(),"Duplicate reach accepted");
  REQUIRE_REACH(!V->BeginSleeping(),"Sleep overwrote active reach");
  V->HearHorn(PC->GetPawn());REQUIRE_REACH(!V->bBenchReaching,"Horn did not cancel reach");
  REQUIRE_REACH(V->BeginBenchReach(),"Could not restart after horn");
  V->BikeImpact(220,FVector::ForwardVector);REQUIRE_REACH(!V->bBenchReaching&&V->StumbleRemaining>0,"Bike impact failed to cancel");
  REQUIRE_REACH(!V->BeginBenchReach(),"Reach accepted during stumble");S.Phase=1;S.Clock=0;return;
 }
 auto* V=S.Visitor.Get();REQUIRE_REACH(V,"Visitor lost");
 if(S.Phase==1&&V->StumbleRemaining<=0){
  REQUIRE_REACH(V->BeginBenchReach(),"Reach failed after stumble");// Exercise the visitor update before the controller opens its pausing loss menu.
  Mode->bRunEnded=true;V->Tick(Dt);S.Phase=2;
 }
 if(S.Phase==2&&!V->bBenchReaching){
  REQUIRE_REACH(!V->BeginBenchReach(),"Reach accepted after run ended");Mode->bRunEnded=false;
  REQUIRE_REACH(V->BeginBenchReach(),"Reach failed after mode restored");
  V->TakeDamage(1,FDamageEvent(),PC,PC->GetPawn());
  REQUIRE_REACH(V->bDead&&!V->bBenchReaching,"Damage failed to cancel reach");
  REQUIRE_REACH(!V->BeginBenchReach(),"Dead visitor accepted reach");
  Finish(true,TEXT("duplicate, sleep conflict, horn, impact, stumble, run end, restart and damage guards passed"));
 }
#undef REQUIRE_REACH
#endif
}
