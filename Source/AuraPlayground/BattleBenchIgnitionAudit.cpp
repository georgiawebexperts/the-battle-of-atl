#include "PiedmontPedestrian.h"
#include "Components/StaticMeshComponent.h"
#include "BattleParkFurniture.h"
#include "BattleBenchFire.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
void TickBattleBenchIgnitionAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleParkFurniture> Furniture;TWeakObjectPtr<APiedmontPedestrian> Visitor;float Clock=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BenchIgnitionAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Reason);PC->ConsoleCommand(TEXT("quit"));};
#define CHECK_IGNITION(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 CHECK_IGNITION(S.Clock<12,"Timed out");
 auto Spawn=[&](){const auto T=S.Furniture->Benches[0];FTransform P(T.TransformVectorNoScale(FVector(0,-1,0)).Rotation(),T.TransformPosition(FVector(0,55,90)));
  auto* V=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),P,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
  V->CityAppearanceVariant=0;UGameplayStatics::FinishSpawningActor(V,P);V->PauseRemaining=30;return V;};
 // Encounter fires only. The Krog wreck and Murder K place permanent decorative
 // fires of this class, so counting every actor made "Unexpected existing fire"
 // fail on a world that is behaving correctly.
 auto FireCount=[&](){int N=0;for(TActorIterator<ABattleBenchFire> It(PC->GetWorld());It;++It)if(!It->IsActorBeingDestroyed()&&It->OwnsBench())++N;return N;};
 if(S.Phase==0){
  for(TActorIterator<ABattleParkFurniture> It(PC->GetWorld());It;++It)if(It->Benches.Num()){S.Furniture=*It;break;}
  CHECK_IGNITION(S.Furniture.IsValid(),"No furniture");CHECK_IGNITION(FireCount()==0,"Unexpected existing fire");
  S.Visitor=Spawn();auto* V=S.Visitor.Get();
  CHECK_IGNITION(!V->BeginBenchIgnition(S.Furniture.Get(),-1),"Invalid bench accepted");
  CHECK_IGNITION(V->BeginBenchIgnition(S.Furniture.Get(),0),"Ignition did not start");
  CHECK_IGNITION(!S.Furniture->IsBenchAvailable(0),"No animation reservation");
  CHECK_IGNITION(V->BenchLighterHandle->IsVisible()&&V->BenchLighterStem->IsVisible(),"Ignition prop hidden during reach");
  CHECK_IGNITION(!ABattleBenchFire::IgniteBench(S.Furniture.Get(),0),"Fire stole occupied bench");
  CHECK_IGNITION(FireCount()==0,"Immediate fire before reach");S.Phase=1;S.Clock=0;return;
 }
 auto* V=S.Visitor.Get();CHECK_IGNITION(V,"Visitor lost");
 if(S.Phase==1&&S.Clock>.6f){
  CHECK_IGNITION(FireCount()==0,"Fire before ignition delay");V->HearHorn(PC->GetPawn());
  CHECK_IGNITION(!V->bBenchReaching&&S.Furniture->IsBenchAvailable(0),"Horn left reservation");
  CHECK_IGNITION(FireCount()==0,"Horn caused fire");
  CHECK_IGNITION(!V->BenchLighterHandle->IsVisible()&&!V->BenchLighterStem->IsVisible(),"Horn left prop visible");V->Destroy();
  S.Visitor=Spawn();CHECK_IGNITION(S.Visitor->BeginBenchIgnition(S.Furniture.Get(),0),"Restart ignition failed");S.Phase=2;S.Clock=0;return;
 }
 if(S.Phase==2&&S.Clock>3.f){
  CHECK_IGNITION(V->BenchesIgnited==1&&FireCount()==1,"Delayed ignition did not produce exactly one fire");
  CHECK_IGNITION(!S.Furniture->IsBenchAvailable(0),"Fire failed to own reservation");V->Destroy();
  CHECK_IGNITION(!S.Furniture->IsBenchAvailable(0),"Visitor destruction released fire reservation");
  for(TActorIterator<ABattleBenchFire> It(PC->GetWorld());It;++It)It->Destroy();
  CHECK_IGNITION(S.Furniture->IsBenchAvailable(0),"Fire cleanup failed");
  S.Visitor=Spawn();CHECK_IGNITION(S.Visitor->BeginBenchIgnition(S.Furniture.Get(),0),"Destruction fixture failed");S.Visitor->Destroy();
  CHECK_IGNITION(S.Furniture->IsBenchAvailable(0)&&FireCount()==0,"Pre-ignition destruction leaked reservation or fire");
  Finish(true,TEXT("delay, occupancy, horn cancellation, single ignition, reservation transfer and destruction passed"));
 }
#undef CHECK_IGNITION
#endif
}
