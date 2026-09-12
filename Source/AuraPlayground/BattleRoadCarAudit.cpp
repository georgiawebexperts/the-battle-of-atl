#include "Misc/CommandLine.h"
#include "BattleRoadCar.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
void TickBattleRoadCarAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadCar> Car;TWeakObjectPtr<AActor> Block;float Clock=0,StoppedAt=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("RoadCarAudit: {\"passed\":%s,\"reason\":\"%s\",\"distance\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Car.IsValid()?S.Car->DistanceTravelled:0);PC->ConsoleCommand(TEXT("quit"));};
#define CHECK_CAR(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 CHECK_CAR(S.Clock<35,"Timed out");
 if(S.Phase==0){
  auto* Car=PC->GetWorld()->SpawnActor<ABattleRoadCar>();S.Car=Car;Car->Route={FVector(-19800,13075,350),FVector(-15800,13015,300)};
  CHECK_CAR(Car->StartRoute(),"Route start failed");
  auto* Block=PC->GetWorld()->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Block);Block->SetRootComponent(Box);Box->SetBoxExtent(FVector(40,140,100));Box->SetCollisionProfileName(TEXT("BlockAllDynamic"));Box->RegisterComponent();Block->SetActorLocation(FVector(-18000,13048,430));S.Block=Block;S.Phase=1;S.Clock=0;return;
 }
 auto* Car=S.Car.Get();CHECK_CAR(Car,"Car missing");CHECK_CAR(Car->bGrounded,"Lost road support");
 if(S.Phase==1&&S.Clock>8){
  CHECK_CAR(Car->DistanceTravelled>800,"Car did not advance");
  CHECK_CAR(Car->Speed<1&&Car->bObstacleAhead,"Car did not stop for obstruction");
  CHECK_CAR(Car->GetActorLocation().X<-18270,"Car penetrated obstruction");
  S.StoppedAt=Car->DistanceTravelled;S.Clock=0;S.Phase=2;return;
 }
 if(S.Phase==2&&S.Clock>2){CHECK_CAR(FMath::Abs(Car->DistanceTravelled-S.StoppedAt)<1,"Stopped car crept forward");S.Block->Destroy();S.Phase=3;S.Clock=0;return;}
 if(S.Phase==3&&Car->bRouteFinished){CHECK_CAR(Car->DistanceTravelled>3990,"Route ended prematurely");CHECK_CAR(Car->Speed==0,"Route end did not stop");Finish(true,TEXT("Grounded travel, braking, stationary hold, resume and route end passed"));}
#undef CHECK_CAR
#endif
}

#include "EngineUtils.h"
void TickBattleRoadLaneAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TArray<TWeakObjectPtr<ABattleRoadCar>> Cars;float Clock=0,ProgressClock=0;bool Started=false,Done=false,SawSignalWait=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;float Travel=0;for(auto C:S.Cars)if(C.IsValid()){Travel+=C->DistanceTravelled;UE_LOG(LogTemp,Display,TEXT("RoadLaneCar: name=%s distance=%.3f position=%s speed=%.3f obstacle=%s finished=%d"),*C->GetName(),C->DistanceTravelled,*C->GetActorLocation().ToString(),C->Speed,*C->LastObstacle,C->bRouteFinished);}UE_LOG(LogTemp,Display,TEXT("RoadLaneAudit: {\"passed\":%s,\"reason\":\"%s\",\"cars\":%d,\"total_distance_cm\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Cars.Num(),Travel);PC->ConsoleCommand(TEXT("quit"));};
 if(S.Clock>110){Finish(false,TEXT("Full lane traversal timed out"));return;}
 if(!S.Started){
  for(TActorIterator<ABattleRoadCar> It(PC->GetWorld());It;++It)if(It->ActorHasTag(TEXT("TenthCarLaneReview")))S.Cars.Add(*It);
  if(S.Cars.Num()!=2){Finish(false,TEXT("Expected opposing lane candidates"));return;}
  for(auto C:S.Cars)if(!C->StartRoute()){Finish(false,TEXT("Lane start rejected"));return;}
  S.Started=true;return;
 }
 S.ProgressClock+=Dt;if(S.ProgressClock>10){S.ProgressClock=0;for(auto C:S.Cars)if(C.IsValid())UE_LOG(LogTemp,Display,TEXT("RoadLaneProgress: %s d=%.2f p=%s speed=%.1f obstacle=%s"),*C->GetName(),C->DistanceTravelled,*C->GetActorLocation().ToString(),C->Speed,*C->LastObstacle);}
 bool Finished=true;
 for(auto C:S.Cars){
  if(!C.IsValid()||!C->bGrounded){Finish(false,TEXT("Lost car or wheel support"));return;}
  S.SawSignalWait|=C->bWaitingForCrossing&&C->Speed<1.f;
  if(!C->bRouteFinished)Finished=false;
  else if(C->DistanceTravelled<39000||C->Speed!=0){Finish(false,TEXT("Incomplete traversal or failed endpoint stop"));return;}
 }
 if(Finished){if(FParse::Param(FCommandLine::Get(),TEXT("BattleRequireSignalWait"))&&!S.SawSignalWait)Finish(false,TEXT("No stopped signal queue observed"));else Finish(true,S.SawSignalWait?TEXT("Both full lanes completed with stopped signal queue, support and endpoint stops"):TEXT("Both opposing 10th Street lanes traversed with wheel support and endpoint stops"));}
#endif
}
