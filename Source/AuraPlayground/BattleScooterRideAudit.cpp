#include "BattleScooterScene.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
static TWeakObjectPtr<ABattleScooterScene> RideScene;
static TWeakObjectPtr<ACameraActor> StagingCamera;
static bool Prepared=false;
static float ScooterClosestCm=1.e9f;
bool PrepareScooterRideAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleScooterRideAudit")))return true;
 if(Prepared){if(RideScene.IsValid()&&PC->GetPawn())ScooterClosestCm=FMath::Min(ScooterClosestCm,float(FVector::Dist2D(RideScene->GetActorLocation(),PC->GetPawn()->GetActorLocation())));return true;}
 const FVector Site(30349.800013,114057.877225,1110.508188);
 if(!RideScene.IsValid()){
  for(TActorIterator<ABattleScooterScene> Existing(PC->GetWorld());Existing;++Existing)Existing->Destroy();
  RideScene=PC->GetWorld()->SpawnActor<ABattleScooterScene>(Site,FRotator::ZeroRotator);RideScene->AppearanceChance=1;
  StagingCamera=PC->GetWorld()->SpawnActor<ACameraActor>(Site+FVector(5000,0,200),FRotator::ZeroRotator);
 }
 PC->SetViewTarget(StagingCamera.Get());
 if(RideScene->bSceneReady){Prepared=true;PC->SetViewTarget(PC->GetPawn());StagingCamera->Destroy();return true;}
 return false;
#else
 return true;
#endif
}
bool ReportScooterRideAudit(){
#if !UE_BUILD_SHIPPING
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleScooterRideAudit")))return true;
 const bool Pass=RideScene.IsValid()&&RideScene->bSceneReady&&RideScene->bVisitStarted&&ScooterClosestCm<2500;
 UE_LOG(LogTemp,Display,TEXT("ScooterRideAudit: {\"passed\":%s,\"ready\":%s,\"visit_started\":%s,\"nearest_scene_cm\":%.2f,\"participants\":%d}"),Pass?TEXT("true"):TEXT("false"),RideScene.IsValid()&&RideScene->bSceneReady?TEXT("true"):TEXT("false"),RideScene.IsValid()&&RideScene->bVisitStarted?TEXT("true"):TEXT("false"),ScooterClosestCm,RideScene.IsValid()?RideScene->Participants.Num():-1);return Pass;
#else
 return true;
#endif
}
