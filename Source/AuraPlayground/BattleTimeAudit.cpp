#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
void ABattleMacController::TickTimeAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());if(!Mode||!Bike)return;
 TimeAuditClock+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleTimeAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"time_pickups\":%d}"),Pass?TEXT("true"):TEXT("false"),TimeAuditStage,Reason,Mode->Pickups?Mode->Pickups->TimePickups:0);ConsoleCommand(TEXT("quit"));TimeAuditStage=99;};
#define CHECK_TIME(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){TimeAuditStage++;TimeAuditClock=0;};
 auto Target=[&](bool Zombie){
  const FVector Spot=Person->GetActorLocation()+FVector(300,0,0);
  APiedmontExplorer* Actor=Zombie?static_cast<APiedmontExplorer*>(GetWorld()->SpawnActor<ABattleZombie>(Spot,FRotator::ZeroRotator)):GetWorld()->SpawnActor<APiedmontPedestrian>(Spot,FRotator::ZeroRotator);
  if(Actor){Actor->SetActorTickEnabled(false);Actor->GetCharacterMovement()->DisableMovement();}TimeAuditTarget=Actor;return Actor;
 };
 auto Aim=[&](){FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);if(TimeAuditTarget.IsValid())SetControlRotation((TimeAuditTarget->GetActorLocation()-Eye).Rotation());};
 if(TimeAuditStage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
  CHECK_TIME(Mode->Pickups&&Mode->Pickups->TimePickups==5,"Incomplete bonus-time layout");
  Mode->TimeRemaining=300;Mode->StartCountdown=1;CHECK_TIME(!Mode->AdjustRunTime(30,TEXT("invalid"))&&Mode->TimeRemaining==300,"Countdown guard failed");Mode->StartCountdown=0;
  SetPause(true);CHECK_TIME(!Mode->AdjustRunTime(30,TEXT("invalid")),"Pause guard failed");SetPause(false);
  Mode->Tick(2);CHECK_TIME(FMath::IsNearlyEqual(Mode->TimeRemaining,298.f,.001f),"Bike clock rate wrong");
  auto* Pickup=GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),FTransform(Bike->GetActorLocation()+FVector(70,0,-20)),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  CHECK_TIME(Pickup,"Pickup fixture missing");Pickup->bTimeBonus=true;Pickup->FinishSpawning(FTransform(Bike->GetActorLocation()+FVector(70,0,-20)));Pickup->SetActorTickEnabled(false);
  CHECK_TIME(Pickup->TryCollect(Bike)&&Mode->TimeRemaining==328&&!Pickup->TryCollect(Bike)&&Mode->TimeRemaining==328,"Bonus collection/repeat guard failed");
  CHECK_TIME(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());Mode->Tick(2);CHECK_TIME(FMath::IsNearlyEqual(Mode->TimeRemaining,325.5f,.001f),"Foot clock rate wrong");
  // Dismounting deliberately holsters the pistol, so the on-foot shots below
  // have to draw first. This audit predates the draw step and was firing a
  // holstered gun, which is why it reported "Pistol failed".
  CHECK_TIME(Person&&Person->ToggleDrawWeapon(),"Could not draw the pistol after dismount");
  CHECK_TIME(Target(true),"Zombie fixture missing");Next();
 }else if(TimeAuditStage==1){Aim();if(TimeAuditClock>.4f){const float Before=Mode->TimeRemaining;CHECK_TIME(Person->Fire(),"Pistol failed");CHECK_TIME(FMath::IsNearlyEqual(Mode->TimeRemaining-Before,10.f,.001f),"Zombie hit did not add ten seconds");TimeAuditTarget->Destroy();CHECK_TIME(Target(false),"Pedestrian fixture missing");Next();}}
 else if(TimeAuditStage==2){Aim();if(TimeAuditClock>.4f){const float Before=Mode->TimeRemaining;CHECK_TIME(Person->Fire(),"Second pistol shot failed");CHECK_TIME(FMath::IsNearlyEqual(Mode->TimeRemaining-Before,-10.f,.001f),"Pedestrian hit did not subtract ten seconds");TimeAuditTarget->Destroy();Bike->GiveWeapon(1,18);CHECK_TIME(Person->SelectWeapon(1)&&Target(true),"Shotgun fixture failed");Next();}}
 else if(TimeAuditStage==3){Aim();if(TimeAuditClock>.5f){const float Before=Mode->TimeRemaining;CHECK_TIME(Person->Fire(),"Shotgun failed");CHECK_TIME(FMath::IsNearlyEqual(Mode->TimeRemaining-Before,10.f,.001f),"Shotgun counted multiple pellets");Next();}}
 else if(TimeAuditStage==4){SetControlRotation(FRotator(80,0,0));if(TimeAuditClock>1){const float Before=Mode->TimeRemaining;CHECK_TIME(Person->Fire()&&Mode->TimeRemaining==Before,"Miss changed time");Mode->TimeRemaining=5;CHECK_TIME(Mode->AdjustRunTime(-10,TEXT("test penalty"))&&Mode->TimeRemaining==0&&Mode->bRunEnded,"Penalty did not end run");CHECK_TIME(!Mode->AdjustRunTime(30,TEXT("late bonus"))&&Mode->TimeRemaining==0,"Bonus resurrected ended run");Finish(true,TEXT("Actual bonus layout, single collection, bike/foot clock, live pistol/shotgun rewards and penalties, miss and ended-run guards pass"));}}
 if(TimeAuditClock>10&&TimeAuditStage!=99)Finish(false,TEXT("Time audit timed out"));
#undef CHECK_TIME
#endif
}
