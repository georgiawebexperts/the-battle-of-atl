#include "BattleGunman.h"
#include "GameFramework/HUD.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "BattleZombie.h"
#include "BattleBike.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformMisc.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "GameFramework/PlayerController.h"
void TickGunmanMotionReview(ABattleEnemyDirector* D,float Dt);
void TickBattleGunmanAudit(ABattleEnemyDirector* D,float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("GunmanMotionReview"))){TickGunmanMotionReview(D,Dt);return;}
 auto* W=D->GetWorld();if(W->GetTimeSeconds()<5)return;
 static int Phase=0;static float Clock=0;static ABattleGunman* Gun=nullptr;static AStaticMeshActor* Cover=nullptr;static FVector Origin;
 auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));if(!B||!M)return;
 auto End=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("GunmanAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Phase,Why);FPlatformMisc::RequestExit(false);};
#define GUN_CHECK(C,R) if(!(C)){End(false,TEXT(R));return;}
 FString ReviewDir;FParse::Value(FCommandLine::Get(),TEXT("GunmanReviewDir="),ReviewDir);
 static ACameraActor* RearCamera=nullptr;
 static bool CapturedWarning=false,CapturedShot=false,CapturedBehind=false,CapturedShooter=false;
 if(!ReviewDir.IsEmpty()&&Phase==1){
  if(Clock>1&&!CapturedWarning){FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("warning.png"),false,false);CapturedWarning=true;}
  if(Clock>1.3f&&Clock<2.7f&&!RearCamera){RearCamera=W->SpawnActor<ACameraActor>(Origin+FVector(0,0,180),FRotator(0,180,0));UGameplayStatics::GetPlayerController(W,0)->SetViewTarget(RearCamera);}
  if(Clock>2.5f&&!CapturedBehind){FVector Eye;FRotator View;UGameplayStatics::GetPlayerController(W,0)->GetPlayerViewPoint(Eye,View);GUN_CHECK(FMath::Abs(FMath::FindDeltaAngleDegrees(View.Yaw,(Gun->GetActorLocation()-Eye).Rotation().Yaw))>150,"Rear review camera did not face away from gunman");FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("behind-warning.png"),false,false);CapturedBehind=true;}
  if(Clock>=2.7f)UGameplayStatics::GetPlayerController(W,0)->SetViewTarget(B);
  if(Gun&&Gun->ShotAlertRemaining>0&&!CapturedShot){FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("covered-shot.png"),false,false);CapturedShot=true;}
 }
 if(!ReviewDir.IsEmpty()&&Phase==2&&Clock>.6f&&!CapturedShooter){FScreenshotRequest::RequestScreenshot(ReviewDir/TEXT("visible-shooter.png"),false,false);CapturedShooter=true;}
 Clock+=Dt;
 if(Phase==0){
  for(TActorIterator<APiedmontPedestrian> It(W);It;++It)It->Destroy(); // Isolate this cover/aim fixture from random crowd occlusion.
  B->DamageGrace=0;B->Ride->StopMovementImmediately();Origin=B->GetActorLocation();
  Gun=W->SpawnActor<ABattleGunman>(Origin+FVector(500,0,0),FRotator(0,180,0));GUN_CHECK(Gun,"Gunman spawn failed");Gun->Cooldown=0;UGameplayStatics::GetPlayerController(W,0)->SetControlRotation(FRotator(0,0,0));
  M->bTutorialActive=true;GUN_CHECK(!Gun->TryAim(B),"Gunman attacked during practice");M->bTutorialActive=false;M->StartCountdown=0;
  GUN_CHECK(Gun->TryAim(B)&&Gun->WindupRemaining>=1.8f,"No warning before shot");
  if(!ReviewDir.IsEmpty())Gun->WindupRemaining=3.8f; // Allow camera settling for front/behind review only.
  Cover=W->SpawnActor<AStaticMeshActor>((Gun->GetActorLocation()+Origin)*.5f,FRotator::ZeroRotator);GUN_CHECK(Cover,"Cover spawn failed");
  Cover->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Cover->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Cover->SetActorScale3D(FVector(.5,4,4));Cover->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
  Phase=1;Clock=0;
 }else if(Phase==1&&Clock>(ReviewDir.IsEmpty()?2.1f:4.1f)){
  GUN_CHECK(Gun->ShotsFired==1&&B->RiderHealth==100,"Cover did not stop warned shot");Cover->Destroy();Gun->Cooldown=0;
  GUN_CHECK(Gun->TryAim(B),"Could not aim after cover removed");B->SetActorLocation(Origin+FVector(0,400,0),false,nullptr,ETeleportType::TeleportPhysics);if(!ReviewDir.IsEmpty()){const bool Grip=FParse::Param(FCommandLine::Get(),TEXT("GunmanGripReview"));if(Grip)if(auto* HUD=UGameplayStatics::GetPlayerController(W,0)->GetHUD())HUD->bShowHUD=false;const FVector Focus=Gun->GetActorLocation()+FVector(0,0,Grip?20:25);const FVector Eye=Gun->GetActorLocation()+(Grip?FVector(-130,FParse::Param(FCommandLine::Get(),TEXT("GunmanGripOpposite"))?100:-100,80):FVector(-450,-280,130));auto* Cam=W->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation());UGameplayStatics::GetPlayerController(W,0)->SetViewTarget(Cam);}Phase=2;Clock=0;
 }else if(Phase==2&&Clock>2.1f){
  GUN_CHECK(Gun->ShotsFired==2&&B->RiderHealth==100,"Shot tracked player after aim lock");
  B->SetActorLocation(Origin,false,nullptr,ETeleportType::TeleportPhysics);Gun->Cooldown=0;GUN_CHECK(Gun->TryAim(B),"Could not aim at stationary target");Phase=3;Clock=0;
 }else if(Phase==3&&Clock>1.9f){
  GUN_CHECK(Gun->ShotsFired==3&&B->RiderHealth==0&&B->Deaths==1,"Uncovered shot was not fatal");
  End(true,TEXT("Practice protected, warning precedes shot, cover blocks, movement dodges locked aim, stationary hit kills"));Phase=4;
 }
#undef GUN_CHECK
#endif
}
