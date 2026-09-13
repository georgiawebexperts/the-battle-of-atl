#include "BattleBike.h"
#include "BattlePickup.h"
#include "BattleRider.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
void TickBattleSpeedAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;int Stage=0;float Clock=0,Peak=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("SpeedPickupAudit: {\"passed\":%s,\"reason\":\"%s\",\"peak_speed_cm_s\":%.2f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Peak);PC->ConsoleCommand(TEXT("quit"));};
 auto* B=Cast<ABattleBike>(PC->GetPawn());if(!B){Finish(false,TEXT("Missing bike"));return;}
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(PC));if(!Mode)return;
 auto Spawn=[&](){const FTransform T(B->GetActorLocation()+FVector(60,0,-20));auto* P=PC->GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),T,PC,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);P->bSpeedBonus=true;P->FinishSpawning(T);P->SetActorTickEnabled(false);return P;};
 auto Shot=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleSpeedReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/Name,false,false);};
 if(S.Stage==0){
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)It->Destroy();
  Mode->StartCountdown=0;Mode->bRunEnded=false;B->RiderHealth=100;B->DamageGrace=100;
  B->SetActorLocationAndRotation(FVector(-10000,4000,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);B->Ride->StopMovementImmediately();B->Ride->SetMovementMode(MOVE_Walking);B->Ride->Speed=0;B->Ride->Gear=1;B->Ride->bRealHandling=false;B->Ride->bForceNextFloorCheck=true;
  auto* P=Spawn();B->RiderHealth=0;const bool DeadBlocked=!P->TryCollect(B);B->RiderHealth=100;
  if(!DeadBlocked||!P->TryCollect(B)||P->TryCollect(B)||B->Ride->BoostRemaining!=5.f){Finish(false,TEXT("Collection, dead guard or duplicate guard failed"));return;}
  S.Stage=1;S.Clock=0;return;
 }
 S.Clock+=Dt;S.Peak=FMath::Max(S.Peak,B->Ride->Speed);
 if(B->bCrashActive){Finish(false,TEXT("Unexpected knockdown in boost lane"));return;}
 if(S.Stage==1&&S.Clock>1.8f){if(S.Peak<2000||B->Ride->BoostRemaining<=0){Finish(false,TEXT("Boost did not reach temporary speed"));return;}Shot(TEXT("boost-active.png"));S.Stage=2;}
 if(S.Stage==2&&S.Clock>8.8f){
  if(B->Ride->BoostRemaining>0||B->Ride->Speed>700){Finish(false,TEXT("Boost did not expire and ease back to gear speed"));return;}
  Shot(TEXT("boost-expired.png"));S.Stage=3;
 }
 if(S.Stage==3&&S.Clock>9.2f){B->Ride->Speed=0;B->Ride->StopMovementImmediately();auto* P=Spawn();
  if(!B->Dismount()){Finish(false,TEXT("Dismount failed"));return;}
  auto* Person=Cast<ABattleRider>(PC->GetPawn());if(!Person||P->TryCollect(Person)||P->bConsumed){Finish(false,TEXT("Foot rider consumed mounted-only boost"));return;}
  Finish(true,TEXT("Mounted pickup, duplicate/death/foot guards, acceleration and timed expiry pass"));
 }
#endif
}
