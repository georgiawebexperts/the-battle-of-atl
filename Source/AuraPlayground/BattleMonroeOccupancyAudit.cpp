#include "BattleRoadCar.h"
#include "BattleRoadCrossing.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
void TickBattleMonroeOccupancyAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TArray<TWeakObjectPtr<ABattleRoadCar>> Cars;TWeakObjectPtr<ABattleRoadCrossing> Gate;TWeakObjectPtr<ACharacter> Occupant;float Clock=0,Hold=0;int Phase=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;if(S.Occupant.IsValid())S.Occupant->Destroy();UE_LOG(LogTemp,Display,TEXT("MonroeOccupancy: {\"passed\":%s,\"reason\":\"%s\",\"both_stopped_seconds\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Hold);PC->ConsoleCommand(TEXT("quit"));};
 if(S.Clock>90){Finish(false,TEXT("Crossing occupancy test timed out"));return;}
 if(S.Phase==0){
  for(TActorIterator<ABattleRoadCrossing> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("MonroeCrossingReview")))S.Gate=*I;
  for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("MonroeCarLaneReview")))S.Cars.Add(*I);
  if(!S.Gate.IsValid()||S.Cars.Num()!=2){Finish(false,TEXT("Review actors missing"));return;}
  S.Gate->bAutoCycle=false;S.Gate->bVehicleGreen=true;S.Gate->bVehicleAmber=false;
  FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  S.Occupant=PC->GetWorld()->SpawnActor<ACharacter>(S.Gate->GetActorLocation(),FRotator::ZeroRotator,Params);
  if(!S.Occupant.IsValid()){Finish(false,TEXT("Character fixture missing"));return;}
  S.Occupant->GetCapsuleComponent()->InitCapsuleSize(32,90);S.Occupant->GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));S.Occupant->GetCharacterMovement()->DisableMovement();
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
 if(S.Phase==1){if(BothStopped)S.Hold+=Dt;else S.Hold=0;if(S.Hold>=2){S.Occupant->Destroy();S.Phase=2;}}
 else if(AllFinished)Finish(true,TEXT("Both cars held on green for character capsule, then completed after clearance"));
#endif
}
