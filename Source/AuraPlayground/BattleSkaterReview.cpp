#include "BattleMacController.h"
#include "BattleSkater.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
void ABattleMacController::TickSkaterReview(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 FlushPressedKeys();if(!IsMoveInputIgnored())SetIgnoreMoveInput(true);if(!IsLookInputIgnored())SetIgnoreLookInput(true);
 if(HUDReviewStage==0){
  if(TActorIterator<ABattleSkater> It(GetWorld());It){SkateReviewActor=*It;It->SetActorLocationAndRotation(FVector(38300,75050,738),FRotator(0,180,0),false,nullptr,ETeleportType::TeleportPhysics);It->RouteIndex=5;It->SkateClock=It->PhaseOffset=0;}
  auto* Cam=GetWorld()->SpawnActor<ACameraActor>();SkateReviewCamera=Cam;if(Cam)SetViewTarget(Cam);HUDReviewStage=1;HUDReviewClock=0;SkateReviewFrames=0;
 }
 auto* S=Cast<ABattleSkater>(SkateReviewActor.Get());auto* Cam=SkateReviewCamera.Get();if(!S||!Cam){ConsoleCommand(TEXT("quit"));return;}
 const FVector Eye=S->GetActorLocation()+S->GetActorRightVector()*390+S->GetActorForwardVector()*120+FVector(0,0,50),Target=S->GetActorLocation()-FVector(0,0,10);
 Cam->SetActorLocationAndRotation(Eye,(Target-Eye).Rotation());HUDReviewClock+=Dt;
 if(SkateReviewFrames<24&&HUDReviewClock>SkateReviewFrames*.1f+.1f){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("SkaterReview");IFileManager::Get().MakeDirectory(*Folder,true);
  FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("skater-%03d.png"),SkateReviewFrames),false,false);UE_LOG(LogTemp,Display,TEXT("SkaterPose: frame=%d clock=%.3f push=%.3f"),SkateReviewFrames,S->SkateClock,S->PushAmount);SkateReviewFrames++;
 }
 if(SkateReviewFrames==24&&HUDReviewClock>2.8f){UE_LOG(LogTemp,Display,TEXT("SkaterReview: frames=24 stance_error=%.3f"),S->StanceError);ConsoleCommand(TEXT("quit"));}
#endif
}
