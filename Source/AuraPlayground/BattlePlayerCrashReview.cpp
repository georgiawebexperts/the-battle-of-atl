#include "BattleBike.h"
#include "BattleFallenBike.h"
#include "Components/BoxComponent.h"
#include "BattlePlayerRecoveryBlend.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void TickBattlePlayerCrashReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<USkeletalMeshComponent> Body;TWeakObjectPtr<ACameraActor> Camera;TWeakObjectPtr<ABattleFallenBike> FallenBike;FVector Hip;float Clock=0,MaxSpan=0;int Shot=0;bool Done=false,Recovering=false;FBattlePlayerRecoveryBlend Blend;int RecoveryShot=0;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(!Bike)return;
 if(!S.Body.IsValid()){
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;Q.AddIgnoredActor(Bike);FVector P(-18000,13048,400);
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,1000),P-FVector(0,0,1000),ECC_WorldStatic,Q)){S.Done=true;PC->ConsoleCommand(TEXT("quit"));return;}
  Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->SetActorLocationAndRotation(Hit.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->DisableMovement();Bike->RefreshRiderPose();
  auto* Mesh=Cast<USkeletalMesh>(Bike->Rider->GetSkinnedAsset());auto* Physics=LoadObject<UPhysicsAsset>(nullptr,TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV3.PA_EllisonCrashCandidateV3"));if(!Mesh||!Physics){S.Done=true;PC->ConsoleCommand(TEXT("quit"));return;}
  auto* Body=NewObject<USkeletalMeshComponent>(Bike);Bike->AddInstanceComponent(Body);Body->SetSkeletalMeshAsset(Mesh);Body->SetPhysicsAsset(Physics,true);Body->SetWorldTransform(Bike->Rider->GetComponentTransform());Body->SetCollisionProfileName(TEXT("Ragdoll"));Body->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);Body->SetCanEverAffectNavigation(false);Body->RegisterComponent();Body->RefreshBoneTransforms();Body->SetAllBodiesSimulatePhysics(true);Body->SetSimulatePhysics(true);
  const auto& Ref=Mesh->GetRefSkeleton();for(int I=0;I<Ref.GetNum();I++)if(auto* B=Body->GetBodyInstance(Ref.GetBoneName(I)))B->SetBodyTransform(Bike->Rider->GetBoneTransform(I),ETeleportType::TeleportPhysics);
  for(const auto& Setup:Physics->SkeletalBodySetups)for(const auto& Shape:Setup->AggGeom.SphylElems){const auto* B=Body->GetBodyInstance(Setup->BoneName);UE_LOG(LogTemp,Display,TEXT("CrashShape: %s radius=%.4f length=%.4f scale=%s"),*Setup->BoneName.ToString(),Shape.Radius,Shape.Length,B?*B->Scale3D.ToString():TEXT("missing"));}
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleFallenBikeReview"))){S.FallenBike=PC->GetWorld()->SpawnActor<ABattleFallenBike>();S.FallenBike->InitializeFrom(Bike,FVector(220,-80,20));}
  S.Hip=Bike->Rider->GetSocketLocation(TEXT("Hips"));Body->SetAllPhysicsLinearVelocity(FVector(300,90,60));Bike->Rider->SetVisibility(false);S.Body=Body;
  S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();const FVector Target=Hit.ImpactPoint+FVector(160,0,70);const FVector Offset(-350,-600,280);S.Camera->SetActorLocationAndRotation(Target+Offset,(-Offset).Rotation());
 }
 PC->SetViewTarget(S.Camera.Get());if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;PC->PlayerCameraManager->UpdateCamera(0);
 if(S.Recovering){
  const bool Finished=S.Blend.Tick(Dt);const float Times[]={.01f,.18f,.36f,1.8f,3.5f,5.4f};
  if(S.RecoveryShot<6&&S.Blend.Clock>Times[S.RecoveryShot]){FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("player-recovery-%d.png"),++S.RecoveryShot),false,false);}
  if(Finished){if(S.FallenBike.IsValid()){const float Up=FMath::Abs(S.FallenBike->GetActorUpVector().Z),Speed=S.FallenBike->Frame->GetPhysicsLinearVelocity().Size();UE_LOG(LogTemp,Display,TEXT("FallenBikeReview: {\"passed\":%s,\"upright_dot\":%.3f,\"speed\":%.3f,\"parts\":%d}"),Up<.5f&&Speed<20&&S.FallenBike->PartCount>10?TEXT("true"):TEXT("false"),Up,Speed,S.FallenBike->PartCount);}const float Head=S.Blend.Pose->GetSocketLocation(TEXT("Head")).Z-S.Blend.FloorZ,Left=S.Blend.Pose->GetSocketLocation(TEXT("Foot_L")).Z-S.Blend.FloorZ,Right=S.Blend.Pose->GetSocketLocation(TEXT("Foot_R")).Z-S.Blend.FloorZ;const bool Pass=S.Blend.TransferError<1&&Head>130&&Head<200&&Left>-2&&Left<10&&Right>-2&&Right<10;UE_LOG(LogTemp,Display,TEXT("PlayerRecoveryComplete: {\"passed\":%s,\"head_height\":%.3f,\"left_foot_height\":%.3f,\"right_foot_height\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Head,Left,Right);S.Done=true;PC->ConsoleCommand(TEXT("quit"));}return;
 }
 S.Clock+=Dt;
 const FVector Hip=S.Body->GetSocketLocation(TEXT("Hips"));for(const FName Bone:{FName(TEXT("Head")),FName(TEXT("Hand_L")),FName(TEXT("Hand_R")),FName(TEXT("Foot_L")),FName(TEXT("Foot_R"))})S.MaxSpan=FMath::Max(S.MaxSpan,float(FVector::Dist(Hip,S.Body->GetSocketLocation(Bone))));
 const float Times[]={.15f,.7f,1.6f,3.5f};if(S.Shot<4&&S.Clock>Times[S.Shot]){FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("player-fall-%d.png"),++S.Shot),false,false);}
 if(S.Clock>4.5f){
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;Q.AddIgnoredActor(Bike);const bool Floor=PC->GetWorld()->LineTraceSingleByChannel(Hit,Hip+FVector(0,0,100),Hip-FVector(0,0,300),ECC_WorldStatic,Q);
  const float Drop=S.Hip.Z-Hip.Z,Clearance=Floor?Hip.Z-Hit.ImpactPoint.Z:-999,Speed=S.Body->GetPhysicsLinearVelocity(TEXT("Hips")).Size();const bool Pass=Floor&&Drop>35&&S.MaxSpan<300&&Clearance>=0&&Clearance<100&&Speed<150;
  UE_LOG(LogTemp,Display,TEXT("PlayerCrashReview: {\"passed\":%s,\"hip_drop_cm\":%.3f,\"max_limb_span_cm\":%.3f,\"hip_floor_cm\":%.3f,\"final_speed\":%.3f}"),Pass?TEXT("true"):TEXT("false"),Drop,S.MaxSpan,Clearance,Speed);if(FParse::Param(FCommandLine::Get(),TEXT("BattlePlayerRecoveryBlend"))&&Pass&&S.Blend.Begin(Bike,S.Body.Get()))S.Recovering=true;else{S.Done=true;PC->ConsoleCommand(TEXT("quit"));}
 }
#endif
}
