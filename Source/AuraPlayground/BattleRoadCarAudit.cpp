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
