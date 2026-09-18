#include "BattleMacController.h"
#include "BattlePickup.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleCheckpoints.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ABattleMacController::TickPickupAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode||!Mode->Pickups)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());auto* Person=Cast<ABattleRider>(GetPawn());if(Person)Bike=Person->ParkedBike;if(!Bike)return;
 auto* Cola=Cast<ABattleColaPickup>(PickupAuditTarget.Get());FDamageEvent Damage;PickupAuditClock+=Dt;
 auto Finish=[&](bool Passed,const TCHAR* Reason){
  UE_LOG(LogTemp,Display,TEXT("BattlePickupAudit: {\"passed\":%s,\"difficulty\":\"%s\",\"phase\":%d,\"reason\":\"%s\",\"spawned\":%d,\"park\":%d,\"trail\":%d,\"collected\":%d}"),Passed?TEXT("true"):TEXT("false"),*Mode->DifficultyName.ToString(),PickupPhase,Reason,Mode->Pickups->Spawned,Mode->Pickups->ParkPickups,Mode->Pickups->TrailPickups,Bike->HealthPickups);
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
#define VERIFY_PICKUP(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Spawn=[&](FVector P){auto* Item=GetWorld()->SpawnActor<ABattleColaPickup>(P,FRotator::ZeroRotator);if(Item)Item->HealAmount=Mode->Difficulty.ColaHealAmount;PickupAuditTarget=Item;return Item;};
 if(PickupPhase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  int32 Count=0;bool Near[2]={false,false};
  // Time and speed crates share the cola class, so count only health pickups.
  for(TActorIterator<ABattleColaPickup> It(GetWorld());It;++It){if(It->bTimeBonus||It->bSpeedBonus)continue;Count++;if(!Cola)Cola=*It;for(int32 I=0;I<2;I++){const auto& A=BattleCheckpoints::Anchors[I];if(FVector::Dist2D(It->GetActorLocation(),FVector(A.X,A.Y,A.Z))<1500)Near[I]=true;}}
  VERIFY_PICKUP(Count==Mode->Difficulty.HealthPickups&&Mode->Pickups->Spawned==Count&&Mode->Pickups->ParkPickups>0&&Mode->Pickups->TrailPickups>0,"Incomplete pickup layout");
  // Supplied as "a pickup at each landmark", not a 1.5 m tolerance.
  VERIFY_PICKUP(Near[0]&&Near[1],"Checkpoint missing a pickup");
  VERIFY_PICKUP(Cola,"No pickup to test");PickupAuditTarget=Cola;
  Bike->SetActorLocation(Cola->GetActorLocation()+FVector(0,0,33),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->bForceNextFloorCheck=true;
  PickupPhase=1;PickupAuditClock=0;return;
 }
 if(PickupPhase==1&&PickupAuditClock>.2f){
  VERIFY_PICKUP(Cola&&!Cola->bConsumed&&Bike->RiderHealth==100&&!Cola->TryCollect(Bike),"Full health wasted a pickup");
  Bike->TakeDamage(25,Damage,this,this);VERIFY_PICKUP(Cola->TryCollect(Bike)&&Bike->RiderHealth==100&&Bike->LastHealAmount==25&&Bike->HealthPickups==1,"Capped bike healing failed");
  VERIFY_PICKUP(!Cola->TryCollect(Bike)&&Bike->HealthPickups==1&&Bike->HurtCooldown==5,"Duplicate collection or cooldown reset");
  Bike->TakeDamage(60,Damage,this,this);
  auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>(Bike->GetActorLocation()+Bike->GetActorForwardVector()*55,Bike->GetActorRotation());VERIFY_PICKUP(Wall,"Wall fixture failed");
  auto* Mesh=Wall->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Wall->SetActorScale3D(FVector(.12,1,2));Mesh->SetCollisionProfileName(TEXT("BlockAll"));PickupAuditWall=Wall;
  VERIFY_PICKUP(Spawn(Bike->GetActorLocation()+Bike->GetActorForwardVector()*110-FVector(0,0,33)),"Pickup fixture failed");PickupPhase=2;PickupAuditClock=0;return;
 }
 if(PickupPhase==2&&PickupAuditClock>.3f){
  VERIFY_PICKUP(Cola&&!Cola->bConsumed&&Bike->RiderHealth==40,"Pickup collected through wall");PickupAuditWall->Destroy();PickupPhase=3;PickupAuditClock=0;return;
 }
 if(PickupPhase==3&&PickupAuditClock>.2f){
  VERIFY_PICKUP(!PickupAuditTarget.IsValid()&&Bike->RiderHealth==75&&Bike->HealthPickups==2,"Unblocked automatic pickup failed");
  VERIFY_PICKUP(Bike->Dismount(),"Dismount fixture failed");Person=Cast<ABattleRider>(GetPawn());VERIFY_PICKUP(Person,"No FPS rider");
  PickupPhase=8;PickupAuditClock=0;return;
 }
 if(PickupPhase==8){
  // The dismount plays a short step-off. Place the on-foot fixture from the
  // settled stance, not from mid-air over the saddle, or the pickup and the
  // later walk-away jump land somewhere else and can pick up a course reward.
  VERIFY_PICKUP(Person&&PickupAuditClock<4,"Dismount never settled");
  if(Person->StepOffRemaining>0)return;
  VERIFY_PICKUP(Spawn(Person->GetActorLocation()+Person->GetActorForwardVector()*60-FVector(0,0,25)),"FPS pickup fixture failed");PickupPhase=4;PickupAuditClock=0;return;
 }
 if(PickupPhase==4&&PickupAuditClock>.2f){
  VERIFY_PICKUP(Person&&Person->Health==100&&Bike->RiderHealth==100&&Bike->HealthPickups==3,"On-foot healing was not shared");
  Person->TakeDamage(40,Damage,this,this);Person->SetActorLocation(Person->GetActorLocation()+Person->GetActorForwardVector()*1000,false,nullptr,ETeleportType::TeleportPhysics);
  Cola=Spawn(Bike->GetActorLocation()+Bike->GetActorRightVector()*110-FVector(0,0,33));VERIFY_PICKUP(Cola&&!Cola->TryCollect(Bike),"Unoccupied bike collected health");PickupPhase=5;PickupAuditClock=0;return;
 }
 if(PickupPhase==5&&PickupAuditClock>.2f){
  VERIFY_PICKUP(Cola&&Person&&Bike->RiderHealth==60&&Person->Health==60,"Distant rider collected health");
  Person->SetActorLocation(Cola->GetActorLocation()+Bike->GetActorForwardVector()*70+FVector(0,0,25),false,nullptr,ETeleportType::TeleportPhysics);PickupPhase=6;PickupAuditClock=0;return;
 }
 if(PickupPhase==6&&PickupAuditClock>.2f){
  VERIFY_PICKUP(Person&&Bike->RiderHealth==95&&Person->Health==95&&Bike->HealthPickups==4,"Returning rider could not collect");
  Person->TakeDamage(1000,Damage,this,this);Cola=Spawn(Person->GetActorLocation()+FVector(0,0,-25));VERIFY_PICKUP(Cola&&!Cola->TryCollect(Person)&&Bike->RiderHealth==0,"Pickup bypassed death recovery");
  VERIFY_PICKUP(Bike->RestoreRiderHealth(-1)==0,"Negative healing accepted");PickupPhase=7;PickupAuditClock=0;return;
 }
 if(PickupPhase==7&&PickupAuditClock>.3f){VERIFY_PICKUP(Cola&&!Cola->bConsumed&&Bike->RiderHealth==0,"Dead rider auto-collected pickup");Finish(true,TEXT("Layout, checkpoints, bike/FPS healing, cap, walls, duplicate and death guards pass"));return;}
 if(PickupAuditClock>15)Finish(false,TEXT("Pickup phase timed out"));
#undef VERIFY_PICKUP
#endif
}
