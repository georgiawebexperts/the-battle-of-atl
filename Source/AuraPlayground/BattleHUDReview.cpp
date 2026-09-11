#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Engine/Engine.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
void ABattleMacController::TickHUDReview(float Dt){
#if !UE_BUILD_SHIPPING
 HUDReviewClock+=Dt;
 if(HUDReviewStage==0&&HUDReviewClock>6){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);
  if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("HUDReview");IFileManager::Get().MakeDirectory(*Folder,true);
  FScreenshotRequest::RequestScreenshot(Folder/TEXT("bike.png"),false,false);HUDReviewStage=1;
 }
 if(HUDReviewStage==1&&HUDReviewClock>7){
  auto* Bike=Cast<ABattleBike>(GetPawn());if(Bike&&!Bike->Dismount()){UE_LOG(LogTemp,Error,TEXT("HUDReview: dismount failed"));ConsoleCommand(TEXT("quit"));return;}HUDReviewStage=2;
 }
 if(HUDReviewStage==2&&HUDReviewClock>9){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("HUDReview");
  FScreenshotRequest::RequestScreenshot(Folder/TEXT("foot.png"),false,false);HUDReviewStage=3;
 }
 if(HUDReviewStage==3&&HUDReviewClock>10){UE_LOG(LogTemp,Display,TEXT("HUDReview: requested bike and foot captures"));ConsoleCommand(TEXT("quit"));}
#endif
}
