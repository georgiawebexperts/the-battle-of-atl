#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattlePolice.h"
#include "Engine/Engine.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/GameplayStatics.h"
void ABattleMacController::TickHUDReview(float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattlePoliceReview"))){FlushPressedKeys();if(!IsMoveInputIgnored())SetIgnoreMoveInput(true);if(!IsLookInputIgnored())SetIgnoreLookInput(true);if(auto* Bike=Cast<ABattleBike>(GetPawn())){Bike->Ride->Speed=Bike->Ride->Pedal=Bike->Ride->Steer=0;Bike->Ride->StopMovementImmediately();}}
 HUDReviewClock+=Dt;
 if(HUDReviewStage==0&&HUDReviewClock>6){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleTimeReview"))){
   if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))Mode->AdjustRunTime(30,TEXT("TIME BONUS"));
   APawn* ViewerPawn=GetPawn();const FVector Spot=ViewerPawn->GetActorLocation()+ViewerPawn->GetActorForwardVector()*300+ViewerPawn->GetActorRightVector()*100-FVector(0,0,30);
   FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);const FTransform Transform((Eye-Spot).Rotation(),Spot);
   auto* Token=GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),Transform,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
   if(Token){Token->bTimeBonus=true;Token->FinishSpawning(Transform);Token->SetActorTickEnabled(false);}
  }
  if(FParse::Param(FCommandLine::Get(),TEXT("BattlePoliceReview"))){
   APawn* Viewer=GetPawn();const FVector Spot=Viewer->GetActorLocation()+Viewer->GetActorForwardVector()*440-Viewer->GetActorRightVector()*100;
   if(auto* Officer=GetWorld()->SpawnActor<ABattlePolice>(Spot,(Viewer->GetActorLocation()-Spot).Rotation())){Officer->Cooldown=100;Officer->Tick(.1f);Officer->SetActorTickEnabled(false);Officer->GetCharacterMovement()->DisableMovement();Officer->bWarning=true;}
   if(auto* Rules=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this))){Rules->PeopleHit=3;Rules->bPoliceAlert=true;Rules->Trouble=9;}
  }
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
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleRigReview"))){
  if(HUDReviewStage==3&&HUDReviewClock>10){UE_LOG(LogTemp,Display,TEXT("HUDReview: requested bike and foot captures"));ConsoleCommand(TEXT("quit"));}
  return;
 }
 auto* Rider=Cast<ABattleRider>(GetPawn());
 auto CheckWrists=[&](const TCHAR* Phase){
  if(!Rider)return;
  const float RightError=FVector::Distance(Rider->FirstPersonArms->GetBoneLocationByName(TEXT("Hand_R"),EBoneSpaces::ComponentSpace),Rider->RightGrip);
  const float LeftError=FVector::Distance(Rider->FirstPersonArms->GetBoneLocationByName(TEXT("Hand_L"),EBoneSpaces::ComponentSpace),Rider->LeftGrip);
  UE_LOG(LogTemp,Display,TEXT("RigReview: %s wrist_error right=%.3f left=%.3f"),Phase,RightError,LeftError);
  if(RightError>2||LeftError>2)UE_LOG(LogTemp,Error,TEXT("RigReview: grip unreachable"));
 };
 auto Capture=[&](const TCHAR* Name){FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("HUDReview");FScreenshotRequest::RequestScreenshot(Folder/Name,false,false);};
 auto Aim=[&](bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::RightMouseButton,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 if(HUDReviewStage==3&&HUDReviewClock>10){CheckWrists(TEXT("rest"));Aim(true);HUDReviewStage=4;}
 if(HUDReviewStage==4&&HUDReviewClock>11){CheckWrists(TEXT("aim"));Capture(TEXT("aim.png"));HUDReviewStage=5;}
 if(HUDReviewStage==5&&HUDReviewClock>12){Aim(false);if(!Rider||!Rider->Fire()){UE_LOG(LogTemp,Error,TEXT("RigReview: fire failed"));}else{Rider->ParkedBike->GiveWeapon(0,10);Rider->Reload();}HUDReviewStage=6;}
 if(HUDReviewStage==6&&HUDReviewClock>12.7f){CheckWrists(TEXT("reload"));Capture(TEXT("reload.png"));HUDReviewStage=7;}
 if(HUDReviewStage==7&&HUDReviewClock>14.2f){
  CheckWrists(TEXT("recovered"));
  const bool Restored=Rider&&Rider->Ammo==12&&Rider->ReloadRemaining<=0&&Rider->MountBike();
  UE_LOG(LogTemp,Display,TEXT("RigReview: reload_and_remount=%d"),Restored);
  UE_LOG(LogTemp,Display,TEXT("HUDReview: requested bike and foot captures"));ConsoleCommand(TEXT("quit"));HUDReviewStage=8;
 }
#endif
}
