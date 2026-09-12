#include "BattleRoadCar.h"
#include "BattleRoadCrossing.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
void TickBattleRoadAmberAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadCar> Car;TWeakObjectPtr<ABattleRoadCrossing> Gate;float Clock=0,Hold=0,MinimumSpeed=10000;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("RoadAmberAudit: {\"passed\":%s,\"reason\":\"%s\",\"minimum_committed_speed\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Why,S.MinimumSpeed);PC->ConsoleCommand(TEXT("quit"));};
#define CHECK_AMBER(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 CHECK_AMBER(S.Clock<25,"Amber sequence timed out");
 auto SpawnCar=[&](){auto* C=PC->GetWorld()->SpawnActor<ABattleRoadCar>();C->Route={FVector(-19800,13075,350),FVector(-15800,13015,300)};FBattleCarCrossing B;B.Crossing=S.Gate.Get();B.StopDistance=1300;C->Crossings.Add(B);S.Car=C;return C->StartRoute();};
 if(S.Phase==0){
  auto* Cycle=PC->GetWorld()->SpawnActor<ABattleRoadCrossing>();Cycle->GreenSeconds=2;Cycle->AmberSeconds=1;Cycle->RedSeconds=2;Cycle->bAutoCycle=true;
  Cycle->Tick(.1f);CHECK_AMBER(Cycle->bVehicleGreen&&!Cycle->bVehicleAmber,"Cycle green");Cycle->Tick(1.91f);CHECK_AMBER(!Cycle->bVehicleGreen&&Cycle->bVehicleAmber,"Cycle amber");Cycle->Tick(1.f);CHECK_AMBER(!Cycle->bVehicleGreen&&!Cycle->bVehicleAmber,"Cycle red");Cycle->Tick(2.f);CHECK_AMBER(Cycle->bVehicleGreen&&!Cycle->bVehicleAmber,"Cycle repeats");Cycle->Destroy();
  auto* Gate=PC->GetWorld()->SpawnActor<ABattleRoadCrossing>();Gate->SetActorLocation(FVector(-18000,13048,420));Gate->bVehicleGreen=true;S.Gate=Gate;CHECK_AMBER(SpawnCar(),"Initial spawn");S.Phase=1;S.Clock=0;return;
 }
 auto* Car=S.Car.Get();CHECK_AMBER(Car&&Car->bGrounded,"Car lost support");
 if(S.Phase==1&&Car->DistanceTravelled>1120){CHECK_AMBER(Car->Speed>600,"Did not reach approach speed");S.Gate->bVehicleGreen=false;S.Gate->bVehicleAmber=true;S.Phase=2;S.Clock=0;}
 if(S.Phase==2){S.MinimumSpeed=FMath::Min(S.MinimumSpeed,Car->Speed);if(Car->DistanceTravelled>2300){CHECK_AMBER(S.MinimumSpeed>500,"Near-line amber caused abrupt braking");Car->Destroy();CHECK_AMBER(SpawnCar(),"Second spawn");S.Phase=3;S.Clock=0;return;}}
 if(S.Phase==3&&Car->DistanceTravelled>1200&&Car->Speed<1){CHECK_AMBER(Car->DistanceTravelled<=1300&&Car->bWaitingForCrossing,"Distant amber did not stop");S.Hold=Car->DistanceTravelled;S.Phase=4;S.Clock=0;return;}
 if(S.Phase==4&&S.Clock>2){CHECK_AMBER(FMath::Abs(Car->DistanceTravelled-S.Hold)<1,"Amber queue crept forward");S.Gate->bVehicleAmber=false;S.Gate->bVehicleGreen=true;S.Phase=5;S.Clock=0;}
 if(S.Phase==5&&Car->bRouteFinished){CHECK_AMBER(Car->DistanceTravelled>3990,"Failed to resume");Finish(true,TEXT("Cycle timing, near-line amber clearance, distant amber stop, hold and green resume passed"));}
#undef CHECK_AMBER
#endif
}
