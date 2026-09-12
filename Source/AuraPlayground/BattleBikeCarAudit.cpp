#include "BattleBike.h"
#include "BattleRoadCar.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
void TickBattleBikeCarAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadCar> Car;float Clock=0;int32 InitialWipeouts=0;bool Started=false,Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Key=[&](bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::W,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 const bool MovingCar=FParse::Param(FCommandLine::Get(),TEXT("BattleMovingCarImpact"));
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());
 auto Finish=[&](bool Pass,const TCHAR* Why){Key(false);S.Done=true;UE_LOG(LogTemp,Display,TEXT("BikeCarAudit: {\"passed\":%s,\"reason\":\"%s\",\"wipeouts\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,Bike?Bike->Ride->Wipeouts-S.InitialWipeouts:0);PC->ConsoleCommand(TEXT("quit"));};
 if(!Bike){Finish(false,TEXT("Mounted bike missing"));return;}
 if(S.Clock>12){Finish(false,TEXT("Bike impact timed out"));return;}
 if(!S.Started){
  auto* Car=PC->GetWorld()->SpawnActor<ABattleRoadCar>();S.Car=Car;Car->Route={FVector(-18000,13048,350),FVector(-17000,13030,350)};
  if(!Car->StartRoute()||!Car->ActorHasTag(TEXT("RideVehicle"))){Finish(false,TEXT("Car setup/classification failed"));return;}Car->SetActorTickEnabled(false);
  FHitResult Ground;FCollisionQueryParams Q(SCENE_QUERY_STAT(BikeCarStart),true,Bike);if(!PC->GetWorld()->LineTraceSingleByChannel(Ground,FVector(-19800,13075,2000),FVector(-19800,13075,-2000),ECC_Visibility,Q)){Finish(false,TEXT("Start support missing"));return;}
  Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=Bike->Ride->Recovery=0;Bike->Ride->Gear=4;Bike->SetActorLocationAndRotation(Ground.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;S.InitialWipeouts=Bike->Ride->Wipeouts;S.Started=true;S.Clock=0;
  if(MovingCar){
   // Late cut-in inside stopping distance, while the car has road speed.
   const FVector Position=Car->GetActorLocation()+Car->GetActorForwardVector()*275.f+FVector(0,0,25);
   Bike->SetActorLocation(Position,false,nullptr,ETeleportType::TeleportPhysics);Car->Speed=650;Car->SetActorTickEnabled(true);
  }else Key(true);return;
 }
 if(Bike->Ride->Wipeouts>S.InitialWipeouts){Finish(Bike->Ride->Wipeouts==S.InitialWipeouts+1&&Bike->Ride->RecoveryReason==TEXT("Traffic impact"),MovingCar?TEXT("Moving car late cut-in entered traffic recovery"):TEXT("Keyboard-driven direct bike/car impact entered traffic recovery"));}
#endif
}
