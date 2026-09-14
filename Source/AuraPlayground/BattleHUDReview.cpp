#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleHomeData.h"
#include "BattleHome.h"
#include "BattleQuest.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattlePolice.h"
#include "BattleDrone.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "BattleParkFurniture.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
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
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleObjectiveReview")))if(auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(M->Quest){M->Quest->bCollected=true;M->bItemCollected=true;M->Quest->NextCheckpoint=HUDReviewStage<1?0:1;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleSkaterReview"))){TickSkaterReview(Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleHomeReview"))||FParse::Param(FCommandLine::Get(),TEXT("BattleHornReview"))||FParse::Param(FCommandLine::Get(),TEXT("BattlePoliceReview"))||FParse::Param(FCommandLine::Get(),TEXT("BattleSkateReview"))){FlushPressedKeys();if(!IsMoveInputIgnored())SetIgnoreMoveInput(true);if(!IsLookInputIgnored())SetIgnoreLookInput(true);if(auto* Bike=Cast<ABattleBike>(GetPawn())){Bike->Ride->Speed=Bike->Ride->Pedal=Bike->Ride->Steer=0;Bike->Ride->StopMovementImmediately();}}
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleBenchReview"))){
  if(auto* Bike=Cast<ABattleBike>(GetPawn()))for(TActorIterator<ABattleParkFurniture> It(GetWorld());It;++It)if(!It->Benches.IsEmpty()){
   const FTransform T=It->Benches[0];const FVector Spot=T.TransformPosition(FVector(0,380,98));const FRotator R(0,(T.GetLocation()-Spot).Rotation().Yaw,0);Bike->SetActorLocationAndRotation(Spot,R,false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(R);Bike->Ride->StopMovementImmediately();break;
  }
 }
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleSkateReview"))){
  if(auto* Bike=Cast<ABattleBike>(GetPawn())){Bike->SetActorLocationAndRotation(FVector(41600,74168,580),FRotator(0,180,0),false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(FRotator(0,180,0));Bike->Ride->StopMovementImmediately();Bike->Ride->bForceNextFloorCheck=true;}
  const FVector Eye(42500,76800,3700),Target(39000,74000,600);if(auto* Cam=GetWorld()->SpawnActor<ACameraActor>(Eye,(Target-Eye).Rotation()))SetViewTarget(Cam);
 }
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleHomeReview"))){
  if(auto* Bike=Cast<ABattleBike>(GetPawn())){Bike->SetActorLocation(BattleHomeData::Gate-BattleHomeData::South*300+FVector(0,0,98),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->Recovery=0;Bike->Ride->bForceNextFloorCheck=true;}
  if(auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(M->Quest){M->Quest->bCollected=true;M->Quest->NextCheckpoint=2;}for(TActorIterator<ABattleHome> It(GetWorld());It;++It){It->bTunnelEntered=false;It->bTunnelExited=false;}
  const FVector Eye=BattleHomeData::Home-BattleHomeData::South*1600+FVector(600,0,800),Target=BattleHomeData::Home+FVector(0,0,170);if(auto* Cam=GetWorld()->SpawnActor<ACameraActor>(Eye,(Target-Eye).Rotation()))SetViewTarget(Cam);
 }
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleMarketReview"))){
  if(auto* Bike=Cast<ABattleBike>(GetPawn())){const FVector XY(-20108,3187,0);FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);if(GetWorld()->LineTraceSingleByChannel(Ground,XY+FVector(0,0,1000),XY-FVector(0,0,1000),ECC_Visibility,Q)){Bike->SetActorLocationAndRotation(Ground.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->StopMovementImmediately();Bike->Ride->bForceNextFloorCheck=true;SetControlRotation(FRotator::ZeroRotator);}}
  const FVector Eye(-20000,2450,650),Target(-18700,3187,20);if(auto* Cam=GetWorld()->SpawnActor<ACameraActor>(Eye,(Target-Eye).Rotation()))SetViewTarget(Cam);
 }
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingHUD")))if(auto* Bike=Cast<ABattleBike>(GetPawn()))Bike->Ride->bRealHandling=true;
 HUDReviewClock+=Dt;
 if(HUDReviewStage==0&&HUDReviewClock>6){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleHornReview"))){
   APawn* Viewer=GetPawn();const FVector Spot=Viewer->GetActorLocation()+Viewer->GetActorForwardVector()*340+FVector(0,0,10);
   if(auto* H=GetWorld()->SpawnActor<ABattleHornPickup>(Spot,FRotator::ZeroRotator)){H->SetActorTickEnabled(false);HornReviewActor=H;const FVector Eye=Spot+FVector(160,190,70);if(auto* Cam=GetWorld()->SpawnActor<ACameraActor>(Eye,(Spot+FVector(0,0,20)-Eye).Rotation()))SetViewTarget(Cam);}
  }
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
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleZombieReview"))){
   for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
   for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
   APawn* Viewer=GetPawn();for(int Style=0;Style<2;Style++){
    const FVector Spot=Viewer->GetActorLocation()+Viewer->GetActorForwardVector()*400+Viewer->GetActorRightVector()*(Style==0?-100:100);
    const FTransform T(FRotator(0,Viewer->GetActorRotation().Yaw+180,0),Spot);
    auto* Z=GetWorld()->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),T,this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);Z->VisualStyle=Style;Z->Emergence=0;Z->FinishSpawning(T);Z->Tick(.1f);Z->SetActorTickEnabled(false);Z->GetCharacterMovement()->DisableMovement();
   }
  }
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleDroneReview"))){
   APawn* Viewer=GetPawn();const FVector Spot=Viewer->GetActorLocation()+Viewer->GetActorForwardVector()*360+Viewer->GetActorRightVector()*80+FVector(0,0,110);
   if(auto* Drone=GetWorld()->SpawnActor<ABattleDrone>(Spot,FRotator(-15,Viewer->GetActorRotation().Yaw+180,0)))Drone->SetActorTickEnabled(false);
  }
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);
  if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("HUDReview");IFileManager::Get().MakeDirectory(*Folder,true);
  FScreenshotRequest::RequestScreenshot(Folder/TEXT("bike.png"),false,false);HUDReviewStage=1;
 }
 if(HUDReviewStage==1&&HUDReviewClock>7){
  auto* Bike=Cast<ABattleBike>(GetPawn());if(Bike&&!Bike->Dismount()){UE_LOG(LogTemp,Error,TEXT("HUDReview: dismount failed"));ConsoleCommand(TEXT("quit"));return;}if(FParse::Param(FCommandLine::Get(),TEXT("BattleRifleReview"))){if(auto* P=Cast<ABattleRider>(GetPawn())){P->ParkedBike->GiveWeapon(4,60);P->SelectWeapon(4);}}if(FParse::Param(FCommandLine::Get(),TEXT("BattleSkateReview"))){SetViewTarget(GetPawn());SetControlRotation(FRotator(0,180,0));}if(FParse::Param(FCommandLine::Get(),TEXT("BattleHomeReview"))){SetViewTarget(GetPawn());SetControlRotation(BattleHomeData::South.Rotation());}if(FParse::Param(FCommandLine::Get(),TEXT("BattleHornReview"))&&HornReviewActor.IsValid()){SetViewTarget(GetPawn());FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((HornReviewActor->GetActorLocation()-Eye).Rotation());}HUDReviewStage=2;
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
 if(HUDReviewStage==5&&HUDReviewClock>12){Aim(false);if(!Rider||!Rider->Fire()){UE_LOG(LogTemp,Error,TEXT("RigReview: fire failed"));}else{Rider->ParkedBike->GiveWeapon(Rider->CurrentWeapon,10);Rider->Reload();}HUDReviewStage=6;}
 if(HUDReviewStage==6&&HUDReviewClock>12.7f){CheckWrists(TEXT("reload"));Capture(TEXT("reload.png"));HUDReviewStage=7;}
 if(HUDReviewStage==7&&HUDReviewClock>15.f){
  CheckWrists(TEXT("recovered"));
  const bool Restored=Rider&&Rider->Ammo==BattleWeapons::Capacity(Rider->CurrentWeapon)&&Rider->ReloadRemaining<=0&&Rider->MountBike();
  UE_LOG(LogTemp,Display,TEXT("RigReview: reload_and_remount=%d"),Restored);
  UE_LOG(LogTemp,Display,TEXT("HUDReview: requested bike and foot captures"));ConsoleCommand(TEXT("quit"));HUDReviewStage=8;
 }
#endif
}
