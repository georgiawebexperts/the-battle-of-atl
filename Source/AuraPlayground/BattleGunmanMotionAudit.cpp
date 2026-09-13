#include "BattleGunman.h"
#include "BattleZombie.h"
#include "BattleBike.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
void TickGunmanMotionReview(ABattleEnemyDirector* D,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=D->GetWorld();if(W->GetTimeSeconds()<5)return;
 static ABattleGunman* Gun=nullptr;static float Clock=0;static int Stage=0,NextCapture=0;
 FString Dir;FParse::Value(FCommandLine::Get(),TEXT("GunmanReviewDir="),Dir);
 auto Finish=[&](bool Passed,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("GunmanAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Passed?TEXT("true"):TEXT("false"),Reason);FPlatformMisc::RequestExit(false);Stage=9;};
 auto* Bike=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));if(!Bike||!Mode)return;
 if(Stage==0){
  for(TActorIterator<APiedmontPedestrian> It(W);It;++It)It->Destroy();
  Bike->Ride->StopMovementImmediately();Mode->bTutorialActive=true;
  const FVector Spot=Bike->GetActorLocation()+FVector(500,0,0);Gun=W->SpawnActor<ABattleGunman>(Spot,FRotator(0,180,0));
  if(!Gun){Finish(false,TEXT("No gunman"));return;}
  const FVector Eye=Spot+FVector(-180,170,110);auto* Camera=W->SpawnActor<ACameraActor>(Eye,(Spot+FVector(0,0,20)-Eye).Rotation());auto* PC=UGameplayStatics::GetPlayerController(W,0);PC->SetViewTarget(Camera);if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;
  Clock=0;Stage=1;return;
 }
 Clock+=Dt;
 if(Stage==1&&Clock>.7f){
  Mode->bTutorialActive=false;Mode->StartCountdown=0;Bike->DamageGrace=0;Gun->Cooldown=0;
  if(!Gun->TryAim(Bike)){Finish(false,TEXT("Unable to start warning"));return;}
  // Leave the locked aim and attack radius so one complete raise/fire/lower cycle can finish.
  Bike->SetActorLocation(Bike->GetActorLocation()+FVector(0,4000,0),false,nullptr,ETeleportType::TeleportPhysics);
  Clock=0;Stage=2;
 }
 if(Stage==2){
  const float Times[]={.05f,.15f,.35f,.7f,1.5f,1.9f,2.8f,3.f,3.2f,3.5f,4.f};
  if(NextCapture<11&&Clock>=Times[NextCapture]){
   if(!Dir.IsEmpty())FScreenshotRequest::RequestScreenshot(Dir/FString::Printf(TEXT("motion-%02d.png"),NextCapture),false,false);
   UE_LOG(LogTemp,Display,TEXT("GunmanMotion: sample=%d time=%.3f warning=%d drawn=%d shots=%d"),NextCapture,Clock,Gun->bWarning,Gun->bWeaponDrawn,Gun->ShotsFired);NextCapture++;
  }
  if(Clock>4.4f){Finish(Gun->ShotsFired==1&&!Gun->bWarning&&!Gun->bWeaponDrawn&&Bike->RiderHealth==100,TEXT("Single locked-aim shot completed; player dodged and shooter returned to idle"));}
 }
#endif
}
