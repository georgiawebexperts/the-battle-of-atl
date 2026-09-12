#include "BattleRoadCar.h"
#include "BattleRoadCrossing.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
void TickBattleRoadCrossingAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadCar> Car;TWeakObjectPtr<ABattleRoadCrossing> Gate;TWeakObjectPtr<AActor> Occupant;float Clock=0,StoppedAt=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("RoadCrossingAudit: {\"passed\":%s,\"reason\":\"%s\",\"distance_cm\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Car.IsValid()?S.Car->DistanceTravelled:0);PC->ConsoleCommand(TEXT("quit"));};
#define CHECK_CROSSING(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 CHECK_CROSSING(S.Clock<25,"Crossing sequence timed out");
 if(S.Phase==0){
  auto* Gate=PC->GetWorld()->SpawnActor<ABattleRoadCrossing>();Gate->SetActorLocation(FVector(-18000,13048,420));S.Gate=Gate;
  auto* Car=PC->GetWorld()->SpawnActor<ABattleRoadCar>();S.Car=Car;Car->Route={FVector(-19800,13075,350),FVector(-15800,13015,300)};
  FBattleCarCrossing Binding;Binding.Crossing=Gate;Binding.StopDistance=1300;Car->Crossings.Add(Binding);
  CHECK_CROSSING(Car->StartRoute(),"Start rejected");S.Phase=1;S.Clock=0;return;
 }
 auto* Car=S.Car.Get();CHECK_CROSSING(Car&&Car->bGrounded,"Car or ground missing");
 if(S.Phase==1&&S.Clock>7){
  CHECK_CROSSING(Car->DistanceTravelled>1200&&Car->DistanceTravelled<=1300&&Car->Speed<1&&Car->bWaitingForCrossing,"Did not stop at red line");S.StoppedAt=Car->DistanceTravelled;
  auto* Occupant=PC->GetWorld()->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Occupant);Occupant->SetRootComponent(Box);Box->SetBoxExtent(FVector(30,30,90));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->SetCollisionObjectType(ECC_Pawn);Box->RegisterComponent();Occupant->SetActorLocation(FVector(-18000,13498,420));S.Occupant=Occupant;S.Gate->bVehicleGreen=true;
  CHECK_CROSSING(!S.Gate->CanEnter(Car),"Green ignored crossing occupant");S.Phase=2;S.Clock=0;return;
 }
 if(S.Phase==2&&S.Clock>2){CHECK_CROSSING(FMath::Abs(Car->DistanceTravelled-S.StoppedAt)<1&&Car->Speed<1,"Entered occupied crossing");S.Occupant->Destroy();S.Phase=3;S.Clock=0;return;}
 if(S.Phase==3&&Car->DistanceTravelled>2300){S.Gate->bVehicleGreen=false;S.Phase=4;S.Clock=0;return;}
 if(S.Phase==4&&Car->bRouteFinished){CHECK_CROSSING(Car->DistanceTravelled>3990,"Failed to clear crossing on red transition");Finish(true,TEXT("Red stop, occupied-green hold, clear release and committed crossing exit passed"));}
#undef CHECK_CROSSING
#endif
}
