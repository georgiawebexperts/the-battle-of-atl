#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "BattleZombie.h"
#include "BattleZombie.h"
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
void TickBattleZombieDeathAudit(ABattleEnemyDirector* D,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=D->GetWorld();if(W->GetTimeSeconds()<5)return;
 static int Phase=0;static float Clock=0,HeadZ=0;static ABattleZombie* Gun=nullptr;static bool Falling=false;
 auto End=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("ZombieDeathAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Why);FPlatformMisc::RequestExit(false);};
#define CHECK_DEATH(C,R) if(!(C)){End(false,TEXT(R));Phase=9;return;}
 FString Dir;FParse::Value(FCommandLine::Get(),TEXT("GunmanReviewDir="),Dir);Clock+=Dt;
 if(Phase==0){
  for(TActorIterator<APiedmontPedestrian> It(W);It;++It)It->Destroy();
  auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));CHECK_DEATH(B,"No bike");B->Ride->StopMovementImmediately();
  auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));M->bTutorialActive=true;
  const FVector Spot=B->GetActorLocation()+FVector(500,0,0);const FTransform Spawn(FRotator(0,180,0),Spot);Gun=W->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),Spawn);if(Gun){Gun->VisualStyle=FParse::Param(FCommandLine::Get(),TEXT("BattlePunkReview"))?1:0;Gun->FinishSpawning(Spawn);}CHECK_DEATH(Gun,"No gunman");
  const FVector Eye=Spot+FVector(-380,-420,220);auto* Cam=W->SpawnActor<ACameraActor>(Eye,(Spot-Eye).Rotation());UGameplayStatics::GetPlayerController(W,0)->SetViewTarget(Cam);Phase=1;Clock=0;
 }else if(Phase==1&&Clock>1.5f){
  HeadZ=Gun->Body->GetSocketLocation(TEXT("Head")).Z;UGameplayStatics::ApplyDamage(Gun,100,nullptr,UGameplayStatics::GetPlayerPawn(W,0),UDamageType::StaticClass());
  CHECK_DEATH(Gun->bDead&&Gun->DeathPhysics&&Gun->DeathPhysics->IsSimulatingPhysics(TEXT("Chest")),"Death did not enter physics");Phase=2;Clock=0;
 }else if(Phase==2){
  if(Clock>.3f&&!Falling&&!Dir.IsEmpty()){FScreenshotRequest::RequestScreenshot(Dir/TEXT("falling.png"),false,false);Falling=true;}
  if(Clock>2.5f){
   auto* Hip=Gun->DeathPhysics->GetBodyInstance(TEXT("Chest"));CHECK_DEATH(Hip,"No physical chest");
   CHECK_DEATH(FVector::Dist(Gun->Body->GetSocketLocation(TEXT("Chest")),Hip->GetUnrealWorldTransform().GetLocation())<10,"Rendered pose does not follow physics");
   CHECK_DEATH(Gun->Body->GetSocketLocation(TEXT("Head")).Z<HeadZ-60,"Attacker remained upright");
   CHECK_DEATH(!Gun->bTelegraphing,"Dead zombie kept attack warning");
   if(!Dir.IsEmpty())FScreenshotRequest::RequestScreenshot(Dir/TEXT("settled.png"),false,false);Phase=3;Clock=0;
  }
 }else if(Phase==3&&Clock>.3f){End(true,TEXT("Fatal damage starts physics, visible chest follows body, head falls and dead zombie attack warning clears"));Phase=9;}
#undef CHECK_DEATH
#endif
}
