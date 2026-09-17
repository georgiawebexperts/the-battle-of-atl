#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleHomeData.h"
#include "BattleHome.h"
#include "BattleQuest.h"
#include "BattleCheckpoints.h"
#include "BattleBoathouse.h"
#include "BattleDuck.h"
#include "BattleSkyline.h"
#include "BattleTutorialData.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattlePolice.h"
#include "BattleDrone.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "BattleParkFurniture.h"
#include "Camera/CameraActor.h"
#include "Engine/PostProcessVolume.h"
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
 if(HUDReviewStage==0&&HUDReviewClock<.1f){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleCrowdVarietyReview"))){
   for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
   for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It){It->DesiredPopulation=0;It->SetActorTickEnabled(false);}
   APawn* Viewer=GetPawn();for(int32 I=0;I<6;++I){
    FVector Spot=Viewer->GetActorLocation()+Viewer->GetActorForwardVector()*550+Viewer->GetActorRightVector()*((I-2.5f)*105);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(Viewer);
    if(GetWorld()->LineTraceSingleByChannel(Hit,Spot+FVector(0,0,500),Spot-FVector(0,0,1000),ECC_Visibility,Q))Spot.Z=Hit.ImpactPoint.Z+98;
    const FTransform T((Viewer->GetActorLocation()-Spot).Rotation(),Spot);auto* Visitor=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T);Visitor->CityAppearanceVariant=I%2;Visitor->CityOutfitVariant=I;Visitor->FinishSpawning(T);Visitor->PauseRemaining=100;Visitor->GetCharacterMovement()->DisableMovement();
   }
  }

  if(FParse::Param(FCommandLine::Get(),TEXT("BattleExposureCandidate"))){auto* Volume=GetWorld()->SpawnActor<APostProcessVolume>();Volume->bUnbound=true;Volume->Priority=100;Volume->Settings.bOverride_AutoExposureBias=true;Volume->Settings.AutoExposureBias-=.35f;}
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleTunnelExposureReview")))if(auto* Bike=Cast<ABattleBike>(GetPawn())){Bike->SetActorLocationAndRotation(FVector(30609,117632,1058),FRotator(0,68,0),false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(FRotator(0,68,0));Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->bForceNextFloorCheck=true;}
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleObjectiveReview")))if(auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(M->Quest){M->Quest->bCollected=true;M->bItemCollected=true;M->Trouble=4;M->PeopleHit=2;M->Quest->NextCheckpoint=HUDReviewStage<1?0:1;}
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
 // Park beside the lost phone so the watch panel can be judged at full signal.
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleWatchReview"))){
  if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(Mode->Quest&&Mode->Quest->bReady)if(auto* Bike=Cast<ABattleBike>(GetPawn())){
   const FVector Target=Mode->Quest->ArtifactLocation+FVector(0,900,0);FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
   const FVector Spot=GetWorld()->LineTraceSingleByChannel(Ground,Target+FVector(0,0,1200),Target-FVector(0,0,1500),ECC_Visibility,Q)?Ground.ImpactPoint+FVector(0,0,98):Target+FVector(0,0,98);
   const FRotator Facing(0,(Mode->Quest->ArtifactLocation-Spot).Rotation().Yaw,0);
   Bike->SetActorLocationAndRotation(Spot,Facing,false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(Facing);
   Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->bForceNextFloorCheck=true;
   UE_LOG(LogTemp,Display,TEXT("WatchReview: spot=%s phone=%s distance_cm=%.0f"),*Spot.ToString(),*Mode->Quest->ArtifactLocation.ToString(),FVector::Dist2D(Spot,Mode->Quest->ArtifactLocation));
  }
 }
 // Roll up to Murder K so the loitering rent-a-cops heckle on camera.
 // Stand beside the new lakeside boathouse and look back at it.
 // Stand on the lake shore and bring the ducks in close so they can be judged.
 // Look south-west from the gate at the downtown skyline. The index picks the
 // vantage: 0 rider at the gate, 1 aerial over the gate, 2 lake-east shore,
 // 3 halfway to the cluster, 4 gate just above the canopy.
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleSkylineReview"))){
  ABattleSkyline* Line=nullptr;
  for(TActorIterator<ABattleSkyline> It(GetWorld());It;++It)if(!Line)Line=*It;
  int32 Index=0;FParse::Value(FCommandLine::Get(),TEXT("BattleSkylineReviewIndex="),Index);
  if(Line)if(auto* Bike=Cast<ABattleBike>(GetPawn())){
   FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
   const FVector Try=BattleTutorialData::Gate+FVector(400,1400,0);
   const FVector Spot=GetWorld()->LineTraceSingleByChannel(Ground,Try+FVector(0,0,900),Try-FVector(0,0,1600),ECC_Visibility,Q)?Ground.ImpactPoint+FVector(0,0,98):Try+FVector(0,0,98);
   const FRotator Facing(0,(Line->Centre-Spot).Rotation().Yaw,0);
   Bike->SetActorLocationAndRotation(Spot,Facing,false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(Facing);
   Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->bForceNextFloorCheck=true;
   FHitResult Cluster;FCollisionQueryParams CQ;
   const bool bClusterGround=GetWorld()->LineTraceSingleByChannel(Cluster,Line->Centre+FVector(0,0,20000),Line->Centre-FVector(0,0,20000),ECC_Visibility,CQ);
   UE_LOG(LogTemp,Display,TEXT("SkylineReview: index=%d spot=%s centre=%s towers=%d distance_m=%.0f cluster_ground=%s base_offset_cm=%.0f"),
    Index,*Spot.ToString(),*Line->Centre.ToString(),Line->Towers,FVector::Dist2D(Spot,Line->Centre)/100.f,
    bClusterGround?TEXT("hit"):TEXT("none"),bClusterGround?Line->Centre.Z-Cluster.ImpactPoint.Z:0.f);
   if(Index>0){
    const float Lift=Index==1?9000.f:Index==3?6000.f:165.f;
    const FVector Where=Index==2?FVector(-19000.f,-2000.f,0):Index==3?FVector(-24000.f,20000.f,0):
     Index==5?FVector(BattleCheckpoints::Anchors[0].X,BattleCheckpoints::Anchors[0].Y,0):
     Index==6?FVector(-14000.f,3000.f,0):FVector(Try.X,Try.Y,0);
    FHitResult Camera;const bool bCamera=GetWorld()->LineTraceSingleByChannel(Camera,Where+FVector(0,0,12000),Where-FVector(0,0,20000),ECC_Visibility,Q);
    const FVector Eye=(bCamera?Camera.ImpactPoint:Where)+FVector(0,0,Lift);
    if(auto* Cam=GetWorld()->SpawnActor<ACameraActor>(Eye,(Line->Centre+FVector(0,0,Index==3?2600.f:1400.f)-Eye).Rotation()))SetViewTarget(Cam);
   }
  }
 }
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleDuckReview"))){
  ABattleDuckFlock* Flock=nullptr;
  for(TActorIterator<ABattleDuckFlock> It(GetWorld());It;++It)if(!Flock)Flock=*It;
  if(Flock&&Flock->Ducks.Num()>0){
   FVector Centre=FVector::ZeroVector;int32 Live=0;
   for(const auto& Duck:Flock->Ducks)if(IsValid(Duck)){Centre+=Duck->GetActorLocation();++Live;}
   if(Live>0)Centre/=Live;
   // Cluster the raft and look at it from just above the water, because the
   // ducks are far too small to judge from the shore.
   for(int32 I=0;I<Flock->Ducks.Num();++I)if(IsValid(Flock->Ducks[I])){
    const float Angle=I*2*PI/FMath::Max(1,Flock->Ducks.Num());
    Flock->Ducks[I]->FlyTo(FVector(Centre.X+FMath::Cos(Angle)*260.f,Centre.Y+FMath::Sin(Angle)*260.f,0),float(I)*.4f);
   }
   const float Surface=Flock->Ducks.Num()&&IsValid(Flock->Ducks[0])?Flock->Ducks[0]->WaterZ:Centre.Z;
   const FVector LookAt(Centre.X,Centre.Y,Surface+26);
   const FVector Eye=LookAt+FVector(0,-700,170);
   if(auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(LookAt-Eye).Rotation()))SetViewTarget(Camera);
   UE_LOG(LogTemp,Display,TEXT("DuckReview: centre=%s ducks=%d eye=%s"),*Centre.ToString(),Flock->Ducks.Num(),*Eye.ToString());
  }
 }
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleBoathouseReview"))){
  for(TActorIterator<ABattleBoathouse> It(GetWorld());It;++It)if(It->bPlaced)if(auto* Bike=Cast<ABattleBike>(GetPawn())){
   const FTransform Xf=It->GetActorTransform();
   const FVector Hull=Xf.TransformPosition(FVector(-700,0,0));
   FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
   // Stand on the shore, not in the lake: accept the first offset that traces
   // dry ground above the water line.
   FVector Spot=Hull+FVector(0,0,98);
   for(const FVector Local:{FVector(-2500,1300,0),FVector(-2500,-1300,0),FVector(-1900,1600,0),FVector(300,1500,0),FVector(300,-1500,0),FVector(1500,900,0)}){
    const FVector Try=Xf.TransformPosition(Local);
    if(GetWorld()->LineTraceSingleByChannel(Ground,Try+FVector(0,0,700),Try-FVector(0,0,1500),ECC_Visibility,Q)&&Ground.ImpactPoint.Z>It->WaterZ+40){
     Spot=Ground.ImpactPoint+FVector(0,0,98);break;
    }
   }
   const FRotator Facing(0,(Hull-Spot).Rotation().Yaw,0);
   Bike->SetActorLocationAndRotation(Spot,Facing,false,nullptr,ETeleportType::TeleportPhysics);SetControlRotation(Facing);
   Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->bForceNextFloorCheck=true;
   UE_LOG(LogTemp,Display,TEXT("BoathouseReview: spot=%s hull=%s dock_over_water=%s hull_on_land=%s"),*Spot.ToString(),*Hull.ToString(),
    It->bDockOverWater?TEXT("true"):TEXT("false"),It->bHullOnLand?TEXT("true"):TEXT("false"));
   break;
  }
 }
 if(HUDReviewStage==0&&HUDReviewClock<.1f&&FParse::Param(FCommandLine::Get(),TEXT("BattleRentACopReview"))){
  // Murder K only activates once the phone is collected (or trouble is high),
  // exactly as it does on a real run home.
  if(auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this)))if(Mode->Quest&&Mode->Enemies)if(auto* Bike=Cast<ABattleBike>(GetPawn())){
   Mode->Quest->bCollected=true;Mode->bTutorialActive=false;Mode->StartCountdown=0;
   Mode->Enemies->bFreezeSpawns=false;Bike->DamageGrace=0;
    const auto& A=BattleCheckpoints::Anchors[0];const FVector Target(A.X,A.Y,A.Z+98);
   FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
   const FVector Spot=GetWorld()->LineTraceSingleByChannel(Ground,Target+FVector(0,0,500),Target-FVector(0,0,900),ECC_Visibility,Q)?Ground.ImpactPoint+FVector(0,0,98):Target;
   Bike->SetActorLocationAndRotation(Spot,FRotator(0,A.Yaw+180,0),false,nullptr,ETeleportType::TeleportPhysics);
   SetControlRotation(FRotator(0,A.Yaw+180,0));Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->Ride->bForceNextFloorCheck=true;
   // Now that the rider is standing in the plaza, the encounter's own approach
   // gate is satisfied; spawn it deterministically rather than waiting.
   Mode->Enemies->TickMurderK(0);
   int32 RentACops=0,Ticking=0;
   for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)if(It->bAmbientMurderK){++RentACops;if(It->IsActorTickEnabled())++Ticking;}
   UE_LOG(LogTemp,Display,TEXT("RentACopReview: spawned_rentacops=%d ticking=%d"),RentACops,Ticking);
   UE_LOG(LogTemp,Display,TEXT("RentACopReview: spot=%s anchor=%s"),*Spot.ToString(),*Target.ToString());
  }
 }
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
   if(auto* Drone=GetWorld()->SpawnActor<ABattleDrone>(Spot,FRotator(-15,Viewer->GetActorRotation().Yaw+180,0))){Drone->SetActorTickEnabled(false);if(FParse::Param(FCommandLine::Get(),TEXT("BattleDroneAimReview"))){const FVector Eye=Spot+FVector(-500,0,80);auto* Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Spot-Eye).Rotation());SetViewTarget(Camera);}}
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
