#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleZombie.h"
#include "BattleRunRecords.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/DamageType.h"

// ATL Streak proof, in game terms: three punks down inside the 5 s window must
// chain to STREAK x3, heal the rider on the third kill, reset after the window
// lapses, and persist only the best streak per difficulty.
void ABattleMacController::TickStreakAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Bike=Cast<ABattleBike>(GetPawn());
 if(auto* Person=Cast<ABattleRider>(GetPawn()))Bike=Person->ParkedBike;
 StreakAuditClock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleStreakAudit: {\"passed\":%s,\"streak\":%d,\"best\":%d,\"kills\":%d,\"health\":%.0f,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Bike?Bike->Streak:-1,Bike?Bike->BestStreak:-1,Bike?Bike->EnemyKills:-1,Bike?Bike->RiderHealth:-1,Reason);UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);};
#define SCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 SCHECK(Mode&&Bike&&Mode->Enemies,"Missing bike or director");
 Mode->Enemies->bFreezeSpawns=true;
 auto SpawnPunk=[&](){auto* Z=GetWorld()->SpawnActor<ABattleZombie>(Bike->GetActorLocation()+FVector(140,0,0),FRotator::ZeroRotator);if(Z){Z->SetActorTickEnabled(false);Z->GetCharacterMovement()->DisableMovement();Z->Emergence=0;Z->Tags.Add(TEXT("BattleStreakFixture"));}return Z;};
 auto KillPunk=[&](ABattleZombie* Z){if(!Z)return;UGameplayStatics::ApplyDamage(Z,400,Bike->GetController(),Bike,UDamageType::StaticClass());if(Z->bDead)Bike->AwardEnemyKill();};
 if(StreakPhase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  Bike->Ride->Speed=0;Bike->Ride->StopMovementImmediately();Mode->StartCountdown=0;Mode->bRunEnded=false;Bike->Streak=0;Bike->BestStreak=0;Bike->RiderHealth=100;
  auto* Z=SpawnPunk();SCHECK(Z,"Fixture punk did not spawn");StreakTarget=Z;StreakPhase=1;StreakAuditClock=0;
 }
 else if(StreakPhase==1&&StreakAuditClock>.2f){
  KillPunk(Cast<ABattleZombie>(StreakTarget.Get()));
  SCHECK(Bike->Streak==1&&Bike->BestStreak==1&&Bike->EnemyKills==1&&Bike->RiderHealth==100,"First kill did not open a streak at x1 without healing");
  StreakPhase=2;StreakAuditClock=0;
 }
 else if(StreakPhase==2&&StreakAuditClock>.2f){
  auto* Z=SpawnPunk();SCHECK(Z,"Second fixture punk did not spawn");StreakTarget=Z;KillPunk(Z);
  SCHECK(Bike->Streak==2&&Bike->BestStreak==2&&Bike->RiderHealth==100&&!Bike->ShotNotice.Contains(TEXT("+15")),"Second kill chained but healed early or broke the streak");
  StreakPhase=3;StreakAuditClock=0;
 }
 else if(StreakPhase==3&&StreakAuditClock>.2f){
  Bike->RiderHealth=50;
  auto* Z=SpawnPunk();SCHECK(Z,"Third fixture punk did not spawn");StreakTarget=Z;KillPunk(Z);
  SCHECK(Bike->Streak==3&&Bike->BestStreak==3&&Bike->RiderHealth==65&&Bike->ShotNotice.Contains(TEXT("+15")),"Third chained kill did not heal 15 or reach x3");
  StreakPhase=4;StreakAuditClock=0;
 }
 else if(StreakPhase==4&&StreakAuditClock>5.5f){
  auto* Z=SpawnPunk();SCHECK(Z,"Fourth fixture punk did not spawn");StreakTarget=Z;KillPunk(Z);
  SCHECK(Bike->Streak==1&&Bike->BestStreak==3&&Bike->RiderHealth==65,"Late kill did not reset the chain while keeping the best streak");
  StreakPhase=5;StreakAuditClock=0;
 }
 else if(StreakPhase==5){
  UGameplayStatics::DeleteGameInSlot(BattleRecords::Slot(),0);
  SCHECK(BattleRecords::RecordStreak(TEXT("Easy"),Bike->BestStreak),"Streak did not persist");
  SCHECK(BattleRecords::BestStreak(TEXT("Easy"))==3,"Persisted streak is not the best one");
  SCHECK(BattleRecords::RecordStreak(TEXT("Easy"),2)&&BattleRecords::BestStreak(TEXT("Easy"))==3,"A worse streak overwrote the best one");
  UGameplayStatics::DeleteGameInSlot(BattleRecords::Slot(),0);
  Finish(true,TEXT("5 s chain, x3 heal, window reset and best-streak persistence pass"));
 }
 if(StreakAuditClock>15)Finish(false,TEXT("Phase timeout"));
#undef SCHECK
#endif
}