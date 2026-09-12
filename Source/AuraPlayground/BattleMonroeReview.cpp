#include "BattleRoadCar.h"
#include "BattleRoadCrossing.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void TickBattleMonroeReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ACameraActor> Camera;TArray<FTransform> Views;float Clock=0;int Phase=0;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Phase==4||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 float CaptureInterval=2.f;FParse::Value(FCommandLine::Get(),TEXT("BattleTrafficReviewInterval="),CaptureInterval);CaptureInterval=FMath::Clamp(CaptureInterval,2.f,20.f);
 if(S.Phase==3){if(S.Clock>CaptureInterval*3+1){S.Phase=4;UE_LOG(LogTemp,Display,TEXT("MonroeReview: complete"));PC->ConsoleCommand(TEXT("quit"));}return;}
 if(!S.Camera.IsValid()){
  // This opt-in scenery review has no rider. Avoid remote combat taking its camera.
  if(APawn* PreviewPawn=PC->GetPawn()){PC->UnPossess();PreviewPawn->Destroy();}
  S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();
  for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("MonroeCarLaneReview"))&&I->Crossings.Num()){
   float D=0;const float Stop=I->Crossings[0].StopDistance;
   for(int J=1;J<I->Route.Num();++J){const FVector A=I->Route[J-1],B=I->Route[J];const float L=FVector::Dist2D(A,B);if(D+L>=Stop){const FVector Direction=(B-A).GetSafeNormal2D();const FVector P=FMath::Lerp(A,B,(Stop-D)/L);S.Views.Add(FTransform(Direction.Rotation(),P-Direction*650+FVector(0,0,140)));break;}D+=L;}
  }
  for(TActorIterator<ABattleRoadCrossing> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("MonroeCrossingReview"))){I->bAutoCycle=false;I->bVehicleGreen=false;I->bVehicleAmber=false;const FVector Offset(3000,1000,1800);S.Views.Add(FTransform((-Offset).Rotation(),I->GetActorLocation()+Offset));}
  if(S.Views.Num()!=3){S.Phase=4;PC->ConsoleCommand(TEXT("quit"));return;}
 }
 S.Camera->SetActorTransform(S.Views[S.Phase]);S.Camera->GetCameraComponent()->SetFieldOfView(90);PC->SetViewTarget(S.Camera.Get());if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;PC->PlayerCameraManager->UpdateCamera(0);
 if(S.Clock>CaptureInterval*(S.Phase+1)){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("monroe-%d.png"),S.Phase+1),false,false);++S.Phase;

 }
#endif
}
