#include "BattleMacController.h"
#include "BattleMemorial.h"
#include "BattleSpiritData.h"
#include "BattleBike.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
void ABattleMacController::TickMemorialReview(float Dt){
#if !UE_BUILD_SHIPPING
 if(MemorialReviewStage<0||GetWorld()->GetTimeSeconds()<5)return;
 auto End=[&](bool Pass,const TCHAR* Why){MemorialReviewStage=-1;UE_LOG(LogTemp,Display,TEXT("BattleMemorialReview: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Why);ConsoleCommand(TEXT("quit"));};
 if(MemorialReviewStage==0){
  ABattleMemorial* Memorial=nullptr;for(TActorIterator<ABattleMemorial> It(GetWorld());It;++It)Memorial=*It;
  if(!Memorial){End(false,TEXT("Missing memorial"));return;}
  TArray<UStaticMeshComponent*> Parts;Memorial->GetComponents(Parts);if(Parts.Num()!=6){End(false,TEXT("Missing flower parts"));return;}
  for(auto* P:Parts)if(!P->GetStaticMesh()||!P->GetMaterial(0)){End(false,TEXT("Missing cooked geometry or material"));return;}
  FCollisionQueryParams Q(SCENE_QUERY_STAT(MemorialClearance),false,GetPawn());FHitResult Hit;
  for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("PiedmontTraffic"))||It->ActorHasTag(TEXT("BattleZombie")))Q.AddIgnoredActor(*It);
  for(const FVector& P:BattleSpiritData::Ride){
   if(GetWorld()->OverlapBlockingTestByChannel(P+FVector(0,0,98),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q)){End(false,TEXT("Obstructed route"));return;}
  }
  const FVector Center=Memorial->GetActorLocation();
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Center+FVector(0,0,80),Center-FVector(0,0,50),ECC_Visibility,Q)||Hit.GetActor()!=Memorial){End(false,TEXT("Memorial boundary absent"));return;}
  Q.AddIgnoredActor(Memorial);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Center+FVector(0,0,60),Center-FVector(0,0,60),ECC_Visibility,Q)||Center.Z-Hit.ImpactPoint.Z>1||Center.Z-Hit.ImpactPoint.Z< -5){End(false,TEXT("Stone not seated in ground"));return;}
  if(auto* H=GetHUD())H->bShowHUD=false;SetIgnoreMoveInput(true);SetIgnoreLookInput(true);
  MemorialReviewCamera=GetWorld()->SpawnActor<ACameraActor>();MemorialReviewCamera->GetCameraComponent()->FieldOfView=55;
  const FVector Eye=Center+FRotator(0,BattleSpiritData::MemorialYaw,0).RotateVector(FVector(-340,-400,220));MemorialReviewCamera->SetActorLocationAndRotation(Eye,(Center+FVector(0,0,12)-Eye).Rotation());SetViewTarget(MemorialReviewCamera);MemorialReviewStage=1;MemorialReviewClock=0;return;
 }
 MemorialReviewClock+=Dt;
 auto Capture=[&](const TCHAR* Name){FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir);FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);};
 if(MemorialReviewStage==1&&MemorialReviewClock>1){Capture(TEXT("wide"));MemorialReviewStage=2;}
 else if(MemorialReviewStage==2&&MemorialReviewClock>1.5f){const FVector Center=BattleSpiritData::Memorial;const FVector Eye=Center+FRotator(0,BattleSpiritData::MemorialYaw,0).RotateVector(FVector(-80,-105,125));MemorialReviewCamera->SetActorLocationAndRotation(Eye,(Center+FVector(0,0,6)-Eye).Rotation());MemorialReviewStage=3;}
 else if(MemorialReviewStage==3&&MemorialReviewClock>2.7f){Capture(TEXT("close"));MemorialReviewStage=4;}
 else if(MemorialReviewStage==4&&MemorialReviewClock>3.5f)End(true,TEXT("Six cooked parts, protective boundary and 63 clear route capsules verified; images require visual review"));
#endif
}
