#include "BattleBike.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "BattleZombie.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

/**
 * Covers the two playtest asks that share one code path: a pedestrian the rider
 * runs into should shout a short line at them, and arcade handling should stop
 * throwing the rider off over a body on foot while realistic still does.
 */
void TickBattleCurseAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;int32 Phase=0;float Clock=0,Total=0;bool Done=false;FString WalkerLine,SleeperLine;int32 WalkerContacts=0;};
 static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<5||!PC->GetPawn())return;
 S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleCurseAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"walker_line\":\"%s\",\"sleeper_line\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Reason,*S.WalkerLine,*S.SleeperLine);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define CHECK_CURSE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
#define ADVANCE(Next,Seconds) do{S.Phase=Next;S.Clock=0;if(Seconds<=0.f)return;}while(0);
 CHECK_CURSE(S.Total<45,"Timed out");
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());CHECK_CURSE(Bike,"Missing bike");
 auto* Move=Bike->Ride.Get();CHECK_CURSE(Move,"Missing movement component");
 // Deferred spawn so the appearance variant is chosen before BeginPlay builds
 // the crowd rig; the sleeper behaviour only exists on the native rig.
 auto Fixture=[&](FVector At){
  const FTransform T(FRotator::ZeroRotator,At);
  auto* P=PC->GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(P){P->CityAppearanceVariant=0;P->PauseRemaining=100;UGameplayStatics::FinishSpawningActor(P,T);}
  return P;
 };
 auto Parked=[&](APiedmontPedestrian* P){if(P){P->SetActorTickEnabled(false);P->GetCharacterMovement()->DisableMovement();}return P;};
 auto FakePersonHit=[&](APiedmontPedestrian* P){
  FHitResult Impact(P,P->GetCapsuleComponent(),P->GetActorLocation(),-Bike->GetActorForwardVector());
  Impact.bBlockingHit=true;Move->Speed=650;Move->Recovery=0;
  const float Latched=Move->ResetContactLatch();
  Move->HandleImpact(Impact,.016f,Bike->GetActorForwardVector());
  return Latched;
 };
 if(S.Phase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(PC->GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<ABattleZombie> It(PC->GetWorld());It;++It)It->Destroy();
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)It->Destroy();
  auto* Walker=Fixture(Bike->GetActorLocation()+FVector(900,0,0));
  CHECK_CURSE(Walker,"Walker fixture failed");
  Walker->GetCharacterMovement()->DisableMovement();
  const int32 Before=Walker->BikeContacts;
  Walker->BikeImpact(220,FVector::ForwardVector);
  CHECK_CURSE(Walker->BikeContacts==Before+1&&Walker->StumbleRemaining>0,"Light bike contact did not stumble the walker");
  CHECK_CURSE(!Walker->CurseLine.IsEmpty()&&Walker->CurseRemaining>0,"Walker did not shout a line");
  S.WalkerLine=Walker->CurseLine;
  // The line has to survive long enough to be read at speed, then go away.
  CHECK_CURSE(Walker->CurseRemaining>=2.f,"Shout is too brief to read");
  ADVANCE(1,0.f);
 }
 else if(S.Phase==1&&S.Clock>1.f){
  APiedmontPedestrian* Walker=nullptr;for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)if(It->CurseLine==S.WalkerLine)Walker=*It;
  CHECK_CURSE(Walker&&Walker->CurseLine==S.WalkerLine&&Walker->CurseRemaining>0,"Shout vanished before it had time to read");
  ADVANCE(2,0.f);
 }
 else if(S.Phase==2&&S.Clock>3.4f){
  bool bStillShouting=false;for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)if(It->CurseRemaining>0)bStillShouting=true;
  CHECK_CURSE(!bStillShouting,"Shout never expired");
  // A sleeper woken by the hit should curse like somebody who was asleep.
  auto* Sleeper=Parked(Fixture(Bike->GetActorLocation()+FVector(1200,0,0)));
  CHECK_CURSE(Sleeper&&Sleeper->BeginSleeping()&&Sleeper->SleepPhase==1,"Sleeper fixture failed");
  Sleeper->SetActorTickEnabled(true);
  Sleeper->BikeImpact(220,FVector::ForwardVector);
  CHECK_CURSE(Sleeper->SleepPhase==0,"Impact did not wake the sleeper");
  CHECK_CURSE(!Sleeper->CurseLine.IsEmpty()&&Sleeper->CurseLine!=S.WalkerLine,"Sleeper shouted the same generic line as a walker");
  S.SleeperLine=Sleeper->CurseLine;
  Move->SetMovementMode(MOVE_Walking);Move->bForceNextFloorCheck=true;
  Bike->RiderHealth=100;Bike->DamageGrace=0;Bike->HurtCooldown=100;
  ADVANCE(3,0.f);
 }
 else if(S.Phase==3&&S.Clock>0.6f){
  // Arcade: a body on foot costs speed and scares the walker, rider stays up.
  Move->bRealHandling=false;Bike->ClearPhysicalCrash();Bike->bParked=false;
  if(Bike->Rider)Bike->Rider->SetVisibility(true,true);
  Bike->RiderHealth=100;
  auto* Victim=Parked(Fixture(Bike->GetActorLocation()+FVector(700,0,0)));
  CHECK_CURSE(Victim,"Arcade impact fixture failed");
  const int32 Contacts=Victim->BikeContacts;
  const float Latched=FakePersonHit(Victim);
  UE_LOG(LogTemp,Display,TEXT("CurseAuditArcade: latched=%.3f speed=%.1f contacts=%d"),Latched,Move->Speed,Victim->BikeContacts);
  CHECK_CURSE(Victim->BikeContacts==Contacts+1&&Victim->StumbleRemaining>0,"Arcade hit did not register on the pedestrian");
  CHECK_CURSE(!Bike->bCrashActive&&Bike->RiderHealth==100,"Arcade handling threw the rider off over a pedestrian");
  ADVANCE(4,0.f);
 }
 else if(S.Phase==4&&S.Clock>0.8f){
  // Realistic: the same hit puts the rider on the ground.
  Move->bRealHandling=true;Bike->RiderHealth=100;Bike->DamageGrace=0;
  auto* Victim=Parked(Fixture(Bike->GetActorLocation()+FVector(700,0,0)));
  CHECK_CURSE(Victim,"Realistic impact fixture failed");
  const float Latched=FakePersonHit(Victim);
  const int32 CrashState=Bike->bCrashActive?1:0,ParkedState=Bike->bParked?1:0,CrashActor=Bike->PlayerCrash?1:0;
  UE_LOG(LogTemp,Display,TEXT("CurseAuditRealistic: latched=%.3f speed=%.1f contacts=%d"),Latched,Move->Speed,Victim->BikeContacts);
  UE_LOG(LogTemp,Display,TEXT("CurseAuditRealistic: speed=%.1f recovery=%.2f real=%d crash=%d health=%.1f wipeouts=%d reason=%s parked=%d player_crash=%d contacts=%d"),
   Move->Speed,Move->Recovery,Move->bRealHandling?1:0,CrashState,Bike->RiderHealth,Move->Wipeouts,*Move->RecoveryReason,ParkedState,CrashActor,Victim->BikeContacts);
  CHECK_CURSE(Bike->bCrashActive&&Bike->RiderHealth<100,"Realistic handling did not throw the rider off");
  Finish(true,TEXT("Shout lines, expiry, sleeper variant and the arcade/realistic pedestrian knock-off split all pass"));
 }
#undef CHECK_CURSE
#undef ADVANCE
#endif
}
