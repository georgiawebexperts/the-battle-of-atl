#include "BattleRoadCar.h"
#include "BattleRoadCrossing.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
void TickBattleKrogRiderAudit(APlayerController*,float);
void TickBattleMonroeOccupancyAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleKrogRiderAudit"))){TickBattleKrogRiderAudit(PC,Dt);return;}
 struct FState{TWeakObjectPtr<UWorld> World;TArray<TWeakObjectPtr<ABattleRoadCar>> Cars;TWeakObjectPtr<ABattleRoadCrossing> Gate;TWeakObjectPtr<ABattleRider> Occupant;FVector ClearSpot=FVector::ZeroVector;float Clock=0,Hold=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("MonroeOccupancy: {\"passed\":%s,\"reason\":\"%s\",\"both_stopped_seconds\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Hold);PC->ConsoleCommand(TEXT("quit"));};
 if(S.Clock>90){Finish(false,TEXT("Crossing occupancy test timed out"));return;}
 if(S.Phase==0){
  for(TActorIterator<ABattleRoadCrossing> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("MonroeCrossingReview")))S.Gate=*I;
  for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("MonroeCarLaneReview")))S.Cars.Add(*I);
  if(!S.Gate.IsValid()||S.Cars.Num()!=2){Finish(false,TEXT("Review actors missing"));return;}
  S.Gate->bAutoCycle=false;S.Gate->bVehicleGreen=true;S.Gate->bVehicleAmber=false;
  // Occupy the crossing with the player, on foot.
  //
  // CanEnter() now holds for another ABattleRoadCar and for a player-controlled
  // pawn, and for nothing else. A bare character capsule stood here until
  // 406c376 (2026-09-18, "Junction traffic flows: only vehicles block the
  // crossing") retired the pedestrian rule - walkers cross the junction
  // constantly, so every car read the box as blocked and the intersection
  // silted up with stationary traffic. This stage kept spawning the capsule and
  // read the result as "Car crossed occupied stop line", which is a contract
  // two builds old rather than a defect. The promise a player can see is that
  // the traffic waits for them standing in the road, so that is what occupies
  // the crossing now.
  if(auto* Bike=Cast<ABattleBike>(PC->GetPawn()))Bike->Dismount();
  auto* Rider=Cast<ABattleRider>(PC->GetPawn());
  if(!Rider){Finish(false,TEXT("On-foot rider missing"));return;}
  // The dismount plays a 0.45 s step-off that lerps the rider back to the bike
  // on every frame, so a placement issued underneath it does not survive it -
  // the same trap that made BattleEntranceWalkAudit look like a stalled walk.
  Rider->StepOffRemaining=0;Rider->SetActorRotation(FRotator(0,Rider->GetActorRotation().Yaw,0));
  Rider->GetCharacterMovement()->StopMovementImmediately();
  S.ClearSpot=Rider->GetActorLocation();
  Rider->SetActorLocation(S.Gate->GetActorLocation(),false,nullptr,ETeleportType::TeleportPhysics);
  S.Occupant=Rider;
  for(auto C:S.Cars)if(C->Crossings.Num()!=1||C->Crossings[0].Crossing!=S.Gate.Get()||!C->StartRoute()){Finish(false,TEXT("Car crossing binding failed"));return;}
  S.Phase=1;return;
}
 bool BothStopped=true,AllFinished=true;
 for(auto C:S.Cars){
  if(!C.IsValid()||!C->bGrounded){Finish(false,TEXT("Car lost support"));return;}
  if(S.Phase==1){
   if(C->DistanceTravelled>C->Crossings[0].StopDistance+.5f){Finish(false,TEXT("Car crossed occupied stop line"));return;}
   BothStopped&=C->bWaitingForCrossing&&C->Speed<1&&C->Crossings[0].StopDistance-C->DistanceTravelled<10;
  }
  AllFinished&=C->bRouteFinished&&C->Speed==0;
 }
 if(S.Phase==1){if(BothStopped)S.Hold+=Dt;else S.Hold=0;if(S.Hold>=2){
  // Step back out of the carriageway rather than vanishing: the crossing has to
  // clear the way a player clears it, and the cars then have to resume.
  if(S.Occupant.IsValid()){S.Occupant->GetCharacterMovement()->StopMovementImmediately();S.Occupant->SetActorLocation(S.ClearSpot,false,nullptr,ETeleportType::TeleportPhysics);}
  S.Phase=2;}}
 else if(AllFinished)Finish(true,TEXT("Both cars held on green for the player standing in the crossing, then completed after they stepped clear"));
#endif
}
