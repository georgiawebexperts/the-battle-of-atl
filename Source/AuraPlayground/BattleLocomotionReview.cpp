#include "BattleMacController.h"
#include "PiedmontExplorer.h"
#include "PiedmontPedestrian.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BattleBike.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void ABattleMacController::TickLocomotionReview(float Dt){
#if !UE_BUILD_SHIPPING
 LocoClock+=Dt;
 if(FParse::Param(FCommandLine::Get(),TEXT("BattlePedalReview"))){
  auto* Bike=Cast<ABattleBike>(GetPawn());if(!Bike||LocoClock<6)return;
  if(!LocoCamera.IsValid()){LocoCamera=GetWorld()->SpawnActor<ACameraActor>();Cast<ACameraActor>(LocoCamera.Get())->GetCameraComponent()->SetFieldOfView(55);SetViewTarget(LocoCamera.Get());if(GetHUD())GetHUD()->bShowHUD=false;}
  const int Phase=FMath::Min(3,int(LocoClock-6));Bike->Ride->Cadence=Phase*PI*.5f;Bike->RefreshRiderPose();
  const FVector Look=Bike->GetActorLocation()+FVector(0,0,5);const FVector Offset=Bike->GetActorRotation().RotateVector(FVector(80,-450,5));LocoCamera->SetActorLocation(Look+Offset);LocoCamera->SetActorRotation((-Offset).Rotation());
  static float MinDot=1,MaxTilt=0;
  for(const TCHAR* Side:{TEXT("L"),TEXT("R")}){const FVector Ankle=Bike->Rider->GetSocketLocation(FName(FString(TEXT("Foot_"))+Side));const FVector Toe=Bike->Rider->GetSocketLocation(FName(FString(TEXT("Foot_"))+Side+TEXT("_end")));const FVector Direction=(Toe-Ankle).GetSafeNormal();MinDot=FMath::Min(MinDot,float(FVector::DotProduct(Direction,Bike->GetActorForwardVector())));MaxTilt=FMath::Max(MaxTilt,float(FMath::Abs(Toe.Z-Ankle.Z)));}
  if(LocoClock>6.4f+LocoStage&&LocoStage<4){FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("pedal-%d.png"),LocoStage),false,false);LocoStage++;}
  if(LocoClock>10.5f){UE_LOG(LogTemp,Display,TEXT("PedalReview: {\"passed\":%s,\"minimum_toe_forward_dot\":%.4f,\"maximum_toe_height_delta_cm\":%.3f}"),MinDot>.95f&&MaxTilt<2?TEXT("true"):TEXT("false"),MinDot,MaxTilt);ConsoleCommand(TEXT("quit"));}
  return;
 }

 if(LocoStage==0&&LocoClock>6){
  APawn* Player=GetPawn();if(!Player)return;
  FHitResult Ground;FCollisionQueryParams Query(SCENE_QUERY_STAT(LocoReview),false,Player);
  const FVector Point=Player->GetActorLocation()+FVector(350,0,0);
  if(!GetWorld()->LineTraceSingleByChannel(Ground,Point+FVector(0,0,400),Point-FVector(0,0,800),ECC_Visibility,Query)){ConsoleCommand(TEXT("quit"));return;}
  FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  APiedmontExplorer* Person=nullptr;
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleCityReview"))){
   const FTransform T(FRotator::ZeroRotator,Ground.ImpactPoint+FVector(0,0,92));
   auto* Visitor=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
   Visitor->CityAppearanceVariant=FParse::Param(FCommandLine::Get(),TEXT("BattleCityFemale"))?1:0;
   Visitor->FinishSpawning(T);if(auto* AI=Cast<AAIController>(Visitor->GetController())){AI->StopMovement();AI->UnPossess();}
   Person=Visitor;
  }else Person=GetWorld()->SpawnActor<APiedmontExplorer>(Ground.ImpactPoint+FVector(0,0,92),FRotator::ZeroRotator,Params);
  Person->GetCharacterMovement()->bRunPhysicsWithNoController=true;
  Person->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
  LocoPerson=Person;LocoStart=Person->GetActorLocation();
  auto* Camera=GetWorld()->SpawnActor<ACameraActor>();Camera->GetCameraComponent()->SetFieldOfView(55);LocoCamera=Camera;SetViewTarget(Camera);
  if(GetHUD())GetHUD()->bShowHUD=false;
  LocoClock=0;LocoStage=1;
 }
 auto* Person=Cast<APiedmontExplorer>(LocoPerson.Get());if(!Person||!LocoCamera.IsValid())return;
 Person->GetCharacterMovement()->MaxWalkSpeed=Person->bNativeCrowdRig?135:(LocoClock<3?140:350);
 if(LocoClock<6)Person->AddMovementInput(FVector::ForwardVector,1,true);
 const FVector Look=Person->GetActorLocation()+FVector(0,0,5);
 const FVector CameraPosition=Look+FVector(170,-390,35);
 LocoCamera->SetActorLocation(CameraPosition);LocoCamera->SetActorRotation((Look-CameraPosition).Rotation());
 const int32 Knee=Person->Body->GetBoneIndex(Person->bNativeCrowdRig?TEXT("calf_l"):TEXT("LowerLeg_L"));
 if(Knee>=0){
  const FQuat Rotation=Person->Body->BoneSpaceTransforms[Knee].GetRotation();
  if(LocoClock<.1f)LocoInitialKnee=Rotation;
  else LocoKneeMotion=FMath::Max(LocoKneeMotion,FMath::RadiansToDegrees(LocoInitialKnee.AngularDistance(Rotation)));
 }
 auto Capture=[&](const TCHAR* Name){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);if(Folder.IsEmpty())Folder=FPaths::ProjectSavedDir()/TEXT("LocoReview");
  IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/Name,false,false);
 };
 if(LocoStage==1&&LocoClock>1){Capture(TEXT("walk.png"));LocoStage=2;}
 if(LocoStage==2&&LocoClock>1.35f){Capture(TEXT("walk-step.png"));LocoStage=3;}
 if(LocoStage==3&&LocoClock>4){Capture(TEXT("run.png"));LocoStage=4;}
 if(LocoStage==4&&LocoClock>7){Capture(TEXT("idle.png"));LocoStage=5;}
 if(LocoStage==5&&LocoClock>8){
  const float Distance=FVector::Dist2D(LocoStart,Person->GetActorLocation());
  const float FootGap=FMath::Min(Person->Body->GetSocketLocation(Person->bNativeCrowdRig?TEXT("ball_l"):TEXT("Foot_L")).Z,Person->Body->GetSocketLocation(Person->bNativeCrowdRig?TEXT("ball_r"):TEXT("Foot_R")).Z)-Person->GetCharacterMovement()->CurrentFloor.HitResult.ImpactPoint.Z-2.275f;
  UE_LOG(LogTemp,Display,TEXT("LocoReview: idle_foot_gap_cm=%.3f"),FootGap);
  float MinimumIdleFootDot=1;
  for(const TCHAR* Side:{TEXT("L"),TEXT("R")}){const FVector Ankle=Person->Body->GetSocketLocation(FName(FString(TEXT("Foot_"))+Side));const FVector Toe=Person->Body->GetSocketLocation(Person->bNativeCrowdRig?FName(FString(TEXT("ball_"))+Side):FName(FString(TEXT("Foot_"))+Side+TEXT("_end")));const float FootDot=FVector::DotProduct((Toe-Ankle).GetSafeNormal(),Person->GetActorForwardVector());MinimumIdleFootDot=FMath::Min(MinimumIdleFootDot,FootDot);UE_LOG(LogTemp,Display,TEXT("LocoReview: foot_%s_forward_dot=%.4f"),Side,FootDot);}

  if(FParse::Param(FCommandLine::Get(),TEXT("BattleCityReview"))){
   int32 Parts=0;TArray<USkeletalMeshComponent*> Meshes;Person->GetComponents(Meshes);
   for(auto* M:Meshes)if(M->GetName().StartsWith(TEXT("CityOutfit"))&&M->LeaderPoseComponent.Get()==Person->Body)Parts++;
   bool HairAttached=false;TArray<UStaticMeshComponent*> Props;Person->GetComponents(Props);
   for(auto* M:Props)if(M->GetName()==TEXT("CityHair"))HairAttached=M->GetAttachParent()==Person->Body&&M->GetAttachSocketName()==TEXT("head");
   const bool NativePass=Person->bNativeCrowdRig&&Parts==4&&HairAttached&&MinimumIdleFootDot>.5f&&FMath::Abs(FootGap)<12&&Person->bAuthoredLocomotion&&LocoKneeMotion>15&&Distance>400&&Person->GetCharacterMovement()->IsMovingOnGround();
   UE_LOG(LogTemp,Display,TEXT("CityLocoReview: pass=%d parts=%d hair=%d foot_gap=%.3f foot_dot=%.3f knee_motion=%.3f travel=%.3f"),NativePass,Parts,HairAttached,FootGap,MinimumIdleFootDot,LocoKneeMotion,Distance);
   ConsoleCommand(TEXT("quit"));LocoStage=6;return;
  }
  const bool Pass=MinimumIdleFootDot>.5f&&FMath::Abs(FootGap)<6&&Person->bAuthoredLocomotion&&LocoKneeMotion>15&&Distance>400&&Person->GetCharacterMovement()->IsMovingOnGround();
  UE_LOG(LogTemp,Display,TEXT("LocoReview: pass=%d knee_motion_deg=%.3f travel_cm=%.3f grounded=%d"),Pass,LocoKneeMotion,Distance,Person->GetCharacterMovement()->IsMovingOnGround());
  ConsoleCommand(TEXT("quit"));LocoStage=6;
 }
#endif
}
