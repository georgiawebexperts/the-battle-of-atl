#include "BattleMacController.h"
#include "BattleMemorial.h"
#include "BattleSpiritData.h"
#include "BattleBike.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/OverlapResult.h"
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
  TArray<UStaticMeshComponent*> Parts;Memorial->GetComponents(Parts);if(Parts.Num()!=7){End(false,TEXT("Missing flower parts or marker"));return;}
  if(!Memorial->Words||Memorial->Words->Text.IsEmpty()){End(false,TEXT("Memorial has no inscription"));return;}
  for(auto* P:Parts)if(!P->GetStaticMesh()||!P->GetMaterial(0)){End(false,TEXT("Missing cooked geometry or material"));return;}
  FCollisionQueryParams Q(SCENE_QUERY_STAT(MemorialClearance),false,GetPawn());FHitResult Hit;
  for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("PiedmontTraffic"))||It->ActorHasTag(TEXT("BattleZombie")))Q.AddIgnoredActor(*It);
  int32 Blocked=0;
  for(int32 I=0;I<UE_ARRAY_COUNT(BattleSpiritData::Ride);++I){
   // The authored route was captured against an older ground surface, so its Z
   // is stale; test clearance above the ground that is there now, otherwise
   // every point reads as buried in the trail apron.
   FVector P=BattleSpiritData::Ride[I];
   FHitResult Floor;
   if(GetWorld()->LineTraceSingleByChannel(Floor,P+FVector(0,0,1500),P-FVector(0,0,1500),ECC_Visibility,Q))P.Z=Floor.ImpactPoint.Z;
   TArray<FOverlapResult> Hits;
   if(!GetWorld()->OverlapMultiByChannel(Hits,P+FVector(0,0,98),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q))continue;
   for(const FOverlapResult& O:Hits){
    UE_LOG(LogTemp,Display,TEXT("MemorialObstruction: index=%d at=%s actor=%s component=%s class=%s"),I,*P.ToString(),*GetNameSafe(O.GetActor()),*GetNameSafe(O.GetComponent()),O.GetActor()?*O.GetActor()->GetClass()->GetName():TEXT("?"));
   }
   ++Blocked;if(Blocked>=6)break;
  }
  if(Blocked){End(false,TEXT("Obstructed route"));return;}
  const FVector Center=Memorial->GetActorLocation();
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Center+FVector(0,0,80),Center-FVector(0,0,50),ECC_Visibility,Q)||Hit.GetActor()!=Memorial){End(false,TEXT("Memorial boundary absent"));return;}
  Q.AddIgnoredActor(Memorial);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,Center+FVector(0,0,60),Center-FVector(0,0,60),ECC_Visibility,Q)||Center.Z-Hit.ImpactPoint.Z>1||Center.Z-Hit.ImpactPoint.Z< -5){End(false,TEXT("Stone not seated in ground"));return;}
  if(auto* H=GetHUD())H->bShowHUD=false;SetIgnoreMoveInput(true);SetIgnoreLookInput(true);
  MemorialReviewCamera=GetWorld()->SpawnActor<ACameraActor>();MemorialReviewCamera->GetCameraComponent()->FieldOfView=70;
  // Look at it from the ride line, at a rider's eye height: this is the shot
  // that answers "would Elliott actually see this memorial go past?".
  FVector From=Center;float BestD=BIG_NUMBER;
  for(const FVector& P:BattleSpiritData::Ride){const float D=FVector::Dist2D(P,Center);if(D<BestD){BestD=D;From=P;}}
  const FVector Eye=From+FVector(0,0,155);
  MemorialReviewCamera->SetActorLocationAndRotation(Eye,(Center+FVector(0,0,95)-Eye).Rotation());SetViewTarget(MemorialReviewCamera);MemorialReviewStage=1;MemorialReviewClock=0;return;
 }
 MemorialReviewClock+=Dt;
 auto Capture=[&](const TCHAR* Name){FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir);FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);};
 if(MemorialReviewStage==1&&MemorialReviewClock>1){Capture(TEXT("wide"));MemorialReviewStage=2;}
 else if(MemorialReviewStage==2&&MemorialReviewClock>1.5f){const FVector Center=BattleSpiritData::Memorial;const FVector Eye=Center+FRotator(0,BattleSpiritData::MemorialYaw,0).RotateVector(FVector(-80,-105,125));MemorialReviewCamera->SetActorLocationAndRotation(Eye,(Center+FVector(0,0,6)-Eye).Rotation());MemorialReviewStage=3;}
 else if(MemorialReviewStage==3&&MemorialReviewClock>2.7f){Capture(TEXT("close"));MemorialReviewStage=4;}
 else if(MemorialReviewStage==4&&MemorialReviewClock>3.5f)End(true,TEXT("Six cooked parts, protective boundary and 63 clear route capsules verified; images require visual review"));
#endif
}
