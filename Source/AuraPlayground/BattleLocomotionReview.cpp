#include "BattleMacController.h"
#include "PiedmontExplorer.h"
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
 if(LocoStage==0&&LocoClock>6){
  APawn* Player=GetPawn();if(!Player)return;
  FHitResult Ground;FCollisionQueryParams Query(SCENE_QUERY_STAT(LocoReview),false,Player);
  const FVector Point=Player->GetActorLocation()+FVector(350,0,0);
  if(!GetWorld()->LineTraceSingleByChannel(Ground,Point+FVector(0,0,400),Point-FVector(0,0,800),ECC_Visibility,Query)){ConsoleCommand(TEXT("quit"));return;}
  FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  auto* Person=GetWorld()->SpawnActor<APiedmontExplorer>(Ground.ImpactPoint+FVector(0,0,92),FRotator::ZeroRotator,Params);
  Person->GetCharacterMovement()->bRunPhysicsWithNoController=true;
  Person->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
  LocoPerson=Person;LocoStart=Person->GetActorLocation();
  auto* Camera=GetWorld()->SpawnActor<ACameraActor>();Camera->GetCameraComponent()->SetFieldOfView(55);LocoCamera=Camera;SetViewTarget(Camera);
  if(GetHUD())GetHUD()->bShowHUD=false;
  LocoClock=0;LocoStage=1;
 }
 auto* Person=Cast<APiedmontExplorer>(LocoPerson.Get());if(!Person||!LocoCamera.IsValid())return;
 Person->GetCharacterMovement()->MaxWalkSpeed=LocoClock<3?140:350;
 if(LocoClock<6)Person->AddMovementInput(FVector::ForwardVector,1,true);
 const FVector Look=Person->GetActorLocation()+FVector(0,0,5);
 const FVector CameraPosition=Look+FVector(170,-390,35);
 LocoCamera->SetActorLocation(CameraPosition);LocoCamera->SetActorRotation((Look-CameraPosition).Rotation());
 const int32 Knee=Person->Body->GetBoneIndex(TEXT("LowerLeg_L"));
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
  const float FootGap=FMath::Min(Person->Body->GetSocketLocation(TEXT("Foot_L")).Z,Person->Body->GetSocketLocation(TEXT("Foot_R")).Z)-Person->GetCharacterMovement()->CurrentFloor.HitResult.ImpactPoint.Z-2.275f;
  UE_LOG(LogTemp,Display,TEXT("LocoReview: idle_foot_gap_cm=%.3f"),FootGap);
  const bool Pass=FMath::Abs(FootGap)<6&&Person->bAuthoredLocomotion&&LocoKneeMotion>15&&Distance>400&&Person->GetCharacterMovement()->IsMovingOnGround();
  UE_LOG(LogTemp,Display,TEXT("LocoReview: pass=%d knee_motion_deg=%.3f travel_cm=%.3f grounded=%d"),Pass,LocoKneeMotion,Distance,Person->GetCharacterMovement()->IsMovingOnGround());
  ConsoleCommand(TEXT("quit"));LocoStage=6;
 }
#endif
}
