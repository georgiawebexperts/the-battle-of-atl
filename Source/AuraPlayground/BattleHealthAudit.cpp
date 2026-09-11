#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ABattleMacController::TickHealthAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Mode||!Mode->Quest)return;
 auto* Quest=Mode->Quest.Get();auto* Bike=Cast<ABattleBike>(GetPawn());
 if(auto* Person=Cast<ABattleRider>(GetPawn()))Bike=Person->ParkedBike;
 if(!Bike)return;
 auto Finish=[&](bool Passed,const TCHAR* Reason){
  UE_LOG(LogTemp,Display,TEXT("BattleHealthAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"deaths\":%d,\"checkpoints\":%d,\"health\":%.2f}"),Passed?TEXT("true"):TEXT("false"),HealthPhase,Reason,Bike->Deaths,Quest->NextCheckpoint,Bike->RiderHealth);
  UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);
 };
#define VERIFY_HEALTH(Condition,Reason) if(!(Condition)){Finish(false,TEXT(Reason));return;}
 auto Place=[&](FVector P){Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->SetActorLocation(P,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;};
 FDamageEvent Damage;HealthAuditClock+=Dt;
 if(HealthPhase==0){
  VERIFY_HEALTH(Quest->bReady&&Quest->CheckpointLocations.Num()==2,"Missing quest checkpoints");
  VERIFY_HEALTH(Quest->SearchDirection(Quest->ArtifactLocation+FVector(0,1000,0))==TEXT("Search NORTH")&&Quest->SearchDirection(Quest->ArtifactLocation-FVector(0,1000,0))==TEXT("Search SOUTH")&&Quest->SearchDirection(Quest->ArtifactLocation-FVector(1000,0,0))==TEXT("Search EAST")&&Quest->SearchDirection(Quest->ArtifactLocation+FVector(1000,0,0))==TEXT("Search WEST"),"Cardinal guidance orientation failed");
  VERIFY_HEALTH(!Quest->ArtifactVisibleOnRadar(Quest->ArtifactLocation)&&ABattleQuest::CoarseDirection(FVector2D(100,20))==ABattleQuest::CoarseDirection(FVector2D(10000,6000)),"Guidance exposes exact bearing/location");
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  Place(Quest->CheckpointLocations[0]+FVector(0,0,98));HealthPhase=1;HealthAuditClock=0;return;
 }
 if(HealthPhase==1&&HealthAuditClock>.3f){
  VERIFY_HEALTH(Quest->NextCheckpoint==0,"Checkpoint unlocked before Artifact");
  VERIFY_HEALTH(Bike->TakeDamage(25,Damage,this,this)==25&&Bike->RiderHealth==75,"Bike damage failed");
  VERIFY_HEALTH(Bike->Dismount(),"Cannot dismount at checkpoint");
  auto* Person=Cast<ABattleRider>(GetPawn());VERIFY_HEALTH(Person&&Person->Health==75,"Health lost on dismount");
  VERIFY_HEALTH(Bike->TakeDamage(500,Damage,this,this)==0,"Empty bike injured rider");
  VERIFY_HEALTH(Person->TakeDamage(25,Damage,this,this)==25&&Person->Health==50&&Bike->RiderHealth==50,"On-foot damage not shared");
  VERIFY_HEALTH(Person->MountBike()&&Bike->RiderHealth==50&&Bike->HurtCooldown==5,"Mount reset health/cooldown");
  VERIFY_HEALTH(Bike->TakeDamage(-10,Damage,this,this)==0&&Bike->RiderHealth==50,"Negative damage healed");
  HealthPhase=2;HealthAuditClock=0;return;
 }
 if(HealthPhase==2&&HealthAuditClock>4.5f){
  VERIFY_HEALTH(Bike->RiderHealth==50,"Health regenerated before delay");HealthPhase=3;return;
 }
 if(HealthPhase==3&&HealthAuditClock>5.6f){
  VERIFY_HEALTH(Bike->RiderHealth>51&&Bike->RiderHealth<54,"Delayed regeneration failed");
  HealthArtifactLocation=Quest->ArtifactLocation;Place(Quest->ArtifactLocation+FVector(0,0,13));HealthPhase=4;HealthAuditClock=0;return;
 }
 if(HealthPhase==4&&HealthAuditClock>.3f){
  VERIFY_HEALTH(Quest->bCollected,"Artifact pickup failed");
  Place(Quest->CheckpointLocations[1]+FVector(0,0,98));HealthPhase=5;HealthAuditClock=0;return;
 }
 if(HealthPhase==5&&HealthAuditClock>.3f){
  VERIFY_HEALTH(Quest->NextCheckpoint==0,"Out-of-order checkpoint accepted");
  Place(Quest->CheckpointLocations[0]+FVector(0,0,98));HealthPhase=6;HealthAuditClock=0;return;
 }
 if(HealthPhase==6&&HealthAuditClock>.3f){
  VERIFY_HEALTH(Quest->NextCheckpoint==1&&Bike->CheckpointName==TEXT("Murder K"),"First checkpoint not recorded");
  VERIFY_HEALTH(Quest->RoutePoints.Num()>1&&Quest->RouteTargetLocation.Equals(Quest->CheckpointLocations[1],1),"Radar did not advance");
  const float Before=Mode->TimeRemaining;Bike->TakeDamage(1000,Damage,this,this);
  VERIFY_HEALTH(Bike->RiderHealth==0&&Bike->RespawnRemaining==2&&FMath::IsNearlyEqual(Before-Mode->TimeRemaining,10.f,.01f),"Lethal hit penalty/recovery failed");
  const float After=Mode->TimeRemaining;VERIFY_HEALTH(Bike->TakeDamage(1000,Damage,this,this)==0&&Mode->TimeRemaining==After,"Repeated hit repeated penalty");
  HealthPhase=7;HealthAuditClock=0;HealthDeathTime=GetWorld()->GetTimeSeconds();return;
 }
 if(HealthPhase==7&&Bike->RespawnRemaining==0){
  VERIFY_HEALTH(GetWorld()->GetTimeSeconds()-HealthDeathTime>=1.98f&&HealthAuditClock<2.3f,"Incorrect respawn duration");
  VERIFY_HEALTH(GetPawn()==Bike&&Bike->RiderHealth==100&&FVector::Dist(Bike->GetActorLocation(),Bike->CheckpointTransform.GetLocation())<150,"Bike checkpoint recovery failed");
  VERIFY_HEALTH(!Quest->bCollected&&!Mode->bItemCollected&&Quest->NextCheckpoint==0&&Quest->bReady&&IsValid(Quest->Artifact)&&Quest->RoutePoints.IsEmpty()&&Quest->SearchResets==1&&Bike->CheckpointTransform.Equals(Quest->InitialStartTransform),"Death did not reset Artifact hunt and park start");
  VERIFY_HEALTH(Bike->TakeDamage(50,Damage,this,this)==0,"Missing respawn protection");
  HealthPhase=8;HealthAuditClock=0;return;
 }
 if(HealthPhase==8&&HealthAuditClock>2.2f){
  VERIFY_HEALTH(Bike->Dismount(),"Cannot dismount after recovery");auto* Person=Cast<ABattleRider>(GetPawn());VERIFY_HEALTH(Person,"Missing FPS pawn");HealthFormerRider=Person;
  const float Before=Mode->TimeRemaining;Person->TakeDamage(1000,Damage,this,this);
  VERIFY_HEALTH(Person->Health==0&&FMath::IsNearlyEqual(Before-Mode->TimeRemaining,10.f,.01f),"On-foot death penalty failed");
  HealthPhase=9;HealthAuditClock=0;return;
 }
 if(HealthPhase==9&&HealthAuditClock>2.2f){
  VERIFY_HEALTH(GetPawn()==Bike&&!HealthFormerRider.IsValid()&&!Bike->bParked&&Bike->RiderHealth==100&&Bike->Deaths==2,"FPS death did not restore bike possession");
  VERIFY_HEALTH(!Quest->bCollected&&Quest->SearchResets==2&&Quest->bReady,"Foot death did not reset hunt");
  Place(Quest->ArtifactLocation+FVector(0,0,13));HealthPhase=91;HealthAuditClock=0;return;
 }
 if(HealthPhase==91&&HealthAuditClock>.3f){
  VERIFY_HEALTH(Quest->bCollected&&Mode->bItemCollected,"Cannot recollect after death");
  Place(Quest->CheckpointLocations[0]+FVector(0,0,98));HealthPhase=92;HealthAuditClock=0;return;
 }
 if(HealthPhase==92&&HealthAuditClock>.3f){
  VERIFY_HEALTH(Quest->NextCheckpoint==1,"First checkpoint cannot be regained");
  Place(Quest->CheckpointLocations[1]+FVector(0,0,98));HealthPhase=10;HealthAuditClock=0;return;
 }
 if(HealthPhase==10&&HealthAuditClock>.3f){
  VERIFY_HEALTH(Quest->NextCheckpoint==2&&Bike->CheckpointName==TEXT("Krog Street Market")&&Quest->bCollected,"Second checkpoint failed");
  Finish(true,TEXT("Cardinal hunt, both death resets, recollection, health and ordered checkpoints pass"));return;
 }
 if(HealthAuditClock>15)Finish(false,TEXT("Phase timed out"));
#undef VERIFY_HEALTH
#endif
}
