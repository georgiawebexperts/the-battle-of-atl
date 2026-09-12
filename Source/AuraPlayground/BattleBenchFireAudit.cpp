#include "BattleBenchFire.h"
#include "BattleParkFurniture.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
void TickBattleBenchFireAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleParkFurniture> Furniture;TWeakObjectPtr<ABattleBenchFire> Second,Third;float Clock=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BenchFireAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Reason);PC->ConsoleCommand(TEXT("quit"));};
#define REQUIRE_FIRE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 REQUIRE_FIRE(S.Clock<15,"Timed out");
 if(S.Phase==0){
  for(TActorIterator<ABattleParkFurniture> It(PC->GetWorld());It;++It)if(It->Benches.Num()>=3){S.Furniture=*It;break;}
  auto* F=S.Furniture.Get();REQUIRE_FIRE(F,"Insufficient park benches");
  REQUIRE_FIRE(!ABattleBenchFire::IgniteBench(F,-1),"Invalid index accepted");
  auto* First=ABattleBenchFire::IgniteBench(F,0);REQUIRE_FIRE(First&&!F->IsBenchAvailable(0),"First fire failed to reserve bench");
  REQUIRE_FIRE(F->ReserveBench(0,First),"Same-owner reservation not idempotent");
  REQUIRE_FIRE(!ABattleBenchFire::IgniteBench(F,0),"Duplicate fire accepted");
  S.Second=ABattleBenchFire::IgniteBench(F,1);REQUIRE_FIRE(S.Second.IsValid(),"Second fire rejected");
  REQUIRE_FIRE(!ABattleBenchFire::IgniteBench(F,2),"Two-fire cap exceeded");
  F->ReleaseBench(0,S.Second.Get());REQUIRE_FIRE(!F->IsBenchAvailable(0),"Wrong owner released reservation");
  First->Destroy();REQUIRE_FIRE(F->IsBenchAvailable(0),"Destruction left bench reserved");
  S.Third=ABattleBenchFire::IgniteBench(F,2);REQUIRE_FIRE(S.Third.IsValid(),"Destroyed fire still counted against cap");
  S.Second->SetLifeSpan(.25f);S.Phase=1;S.Clock=0;return;
 }
 if(S.Phase==1&&S.Clock>.8f){
  REQUIRE_FIRE(!S.Second.IsValid()&&S.Furniture->IsBenchAvailable(1),"Expiry failed to release bench");
  S.Third->Destroy();REQUIRE_FIRE(S.Furniture->IsBenchAvailable(2),"Last bench remained reserved");
  Finish(true,TEXT("reservation ownership, duplicate rejection, two-fire cap, destruction and expiry passed"));
 }
#undef REQUIRE_FIRE
#endif
}
