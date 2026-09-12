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

void TickBattleCrossingReservationAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 static TWeakObjectPtr<UWorld> DoneWorld;if(DoneWorld==PC->GetWorld()||PC->GetWorld()->GetTimeSeconds()<5)return;DoneWorld=PC->GetWorld();
 bool Pass=true;auto Check=[&](bool Value,const TCHAR* Label){if(!Value){Pass=false;UE_LOG(LogTemp,Error,TEXT("CrossingReservationFailure: %s"),Label);}};
 auto* Gate=PC->GetWorld()->SpawnActor<ABattleRoadCrossing>();Gate->SetActorLocation(FVector(-18000,13048,420));Gate->bVehicleGreen=true;
 auto SpawnBody=[&](FVector Position){auto* A=PC->GetWorld()->SpawnActor<AActor>();auto* B=NewObject<UBoxComponent>(A);A->SetRootComponent(B);B->SetBoxExtent(FVector(236,114,55));B->SetCollisionProfileName(TEXT("BlockAllDynamic"));B->RegisterComponent();A->SetActorLocation(Position);return A;};
 auto* First=SpawnBody(FVector(-19000,13048,420));auto* Second=SpawnBody(FVector(-17000,13048,420));
 Check(Gate->TryReserve(First),TEXT("First reservation"));Check(Gate->TryReserve(First),TEXT("Idempotent owner"));Check(!Gate->TryReserve(Second),TEXT("Competing car rejected"));
 Gate->ReleaseVehicle(Second);Check(!Gate->CanEnter(Second),TEXT("Wrong owner cannot release"));
 Gate->bVehicleGreen=false;Check(Gate->TryReserve(First),TEXT("Committed car retains access after red"));
 First->SetActorLocation(Gate->GetActorLocation());Gate->Tick(.05f);Check(!Gate->CanEnter(Second),TEXT("Occupied reservation retained"));
 First->SetActorLocation(FVector(-17000,14000,420));Gate->Tick(.05f);Check(!Gate->CanEnter(Second),TEXT("Release does not override red"));
 Gate->bVehicleGreen=true;Check(Gate->TryReserve(Second),TEXT("Reservation released after tail clears"));
 Second->Destroy();Gate->Tick(.05f);Check(Gate->TryReserve(First),TEXT("Destroyed owner releases crossing"));
 Gate->ReleaseVehicle(First);First->Destroy();Gate->Destroy();
 UE_LOG(LogTemp,Display,TEXT("CrossingReservationAudit: {\"passed\":%s,\"scope\":\"ownership, contention, red commitment, tail clearance and destruction\"}"),Pass?TEXT("true"):TEXT("false"));PC->ConsoleCommand(TEXT("quit"));
#endif
}
