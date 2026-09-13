#include "BattleZombie.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/HUD.h"
#include "BattleGunman.h"
#include "BattleBike.h"
#include "Camera/CameraActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformMisc.h"
void TickBattleGunmanDeathAudit(ABattleEnemyDirector* D,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=D->GetWorld();if(W->GetTimeSeconds()<5)return;
 static int Phase=0;static float Clock=0,HeadZ=0;static ABattleGunman* Gun=nullptr;static bool Falling=false;static FName HipName=TEXT("Hips");
 auto End=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("GunmanDeathAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Why);FPlatformMisc::RequestExit(false);};
#define CHECK_DEATH(C,R) if(!(C)){End(false,TEXT(R));Phase=9;return;}
 FString Dir;FParse::Value(FCommandLine::Get(),TEXT("GunmanReviewDir="),Dir);Clock+=Dt;
 if(Phase==0){
  auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));CHECK_DEATH(B,"No bike");B->Ride->StopMovementImmediately();
  auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));M->bTutorialActive=true;
  const FVector Spot=B->GetActorLocation()+FVector(500,0,0);Gun=W->SpawnActor<ABattleGunman>(Spot,FRotator(0,180,0));CHECK_DEATH(Gun,"No gunman");
  const FVector Eye=Spot+FVector(-260,-280,180);auto* Cam=W->SpawnActor<ACameraActor>(Eye,(Spot-Eye).Rotation());UGameplayStatics::GetPlayerController(W,0)->SetViewTarget(Cam);if(!Dir.IsEmpty())if(auto* HUD=UGameplayStatics::GetPlayerController(W,0)->GetHUD())HUD->bShowHUD=false;Phase=1;Clock=0;
 }else if(Phase==1&&Clock>.6f){
  auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));CHECK_DEATH(B,"No bike at aim start");B->DamageGrace=0;
  auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));M->bTutorialActive=false;M->StartCountdown=0;Gun->Cooldown=0;
  CHECK_DEATH(Gun->TryAim(B)&&Gun->bWarning,"Could not begin live warning before fatal hit");Phase=4;Clock=0;
 }else if(Phase==4&&Clock>.4f){
  CHECK_DEATH(Gun->bWarning&&Gun->ShotsFired==0,"Warning ended before fatal hit");
  if(Gun->DetailedWeapon)CHECK_DEATH(Gun->DetailedWeapon->IsVisible(),"Detailed weapon missing during warning");
  HipName=Gun->Body->GetBoneIndex(TEXT("pelvis"))>=0?FName(TEXT("pelvis")):FName(TEXT("Hips"));HeadZ=Gun->Body->GetSocketLocation(TEXT("Head")).Z;UGameplayStatics::ApplyDamage(Gun,100,nullptr,UGameplayStatics::GetPlayerPawn(W,0),UDamageType::StaticClass());
  CHECK_DEATH(!Gun->bWarning&&!Gun->Weapon->IsVisible()&&(!Gun->DetailedWeapon||!Gun->DetailedWeapon->IsVisible()),"Fatal hit left warning or weapon active");
  CHECK_DEATH(Gun->bDead&&Gun->DeathPhysics&&Gun->DeathPhysics->IsSimulatingPhysics(HipName),"Death did not enter physics");Phase=2;Clock=0;
 }else if(Phase==2){
  if(Clock>.3f&&!Falling&&!Dir.IsEmpty()){FScreenshotRequest::RequestScreenshot(Dir/TEXT("falling.png"),false,false);Falling=true;}
  if(Clock>2.5f){
   auto* Hip=Gun->DeathPhysics->GetBodyInstance(HipName);CHECK_DEATH(Hip,"No physical hip");
   CHECK_DEATH(FVector::Dist(Gun->Body->GetSocketLocation(HipName),Hip->GetUnrealWorldTransform().GetLocation())<10,"Rendered pose does not follow physics");
   CHECK_DEATH(Gun->Body->GetSocketLocation(TEXT("Head")).Z<HeadZ-60,"Attacker remained upright");
   CHECK_DEATH(!Gun->ResolveShot()&&Gun->ShotsFired==0&&!Gun->TryAim(UGameplayStatics::GetPlayerPawn(W,0)),"Dead attacker fired or restarted aiming");
   if(!Dir.IsEmpty())FScreenshotRequest::RequestScreenshot(Dir/TEXT("settled.png"),false,false);Phase=3;Clock=0;
  }
 }else if(Phase==3&&Clock>.3f){End(true,TEXT("Fatal hit cancels live warning and weapon, physics collapse follows, dead shooter cannot fire or re-aim"));Phase=9;}
#undef CHECK_DEATH
#endif
}
