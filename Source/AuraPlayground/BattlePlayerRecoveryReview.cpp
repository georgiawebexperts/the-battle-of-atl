#include "BattleBike.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void TickBattlePlayerRecoveryReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<USkeletalMeshComponent> Body;TWeakObjectPtr<ACameraActor> Camera;float Clock=0,Floor=0;int Clip=0,Shot=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());if(!Bike)return;
 if(!S.Body.IsValid()){
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;Q.AddIgnoredActor(Bike);FVector P(-18000,13048,400);
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,P+FVector(0,0,1000),P-FVector(0,0,1000),ECC_WorldStatic,Q)){S.Done=true;PC->ConsoleCommand(TEXT("quit"));return;}
  Bike->Ride->StopMovementImmediately();Bike->Ride->DisableMovement();Bike->SetActorHiddenInGame(true);S.Floor=Hit.ImpactPoint.Z;
  auto* Body=NewObject<USkeletalMeshComponent>(Bike);Bike->AddInstanceComponent(Body);Body->SetSkeletalMeshAsset(Cast<USkeletalMesh>(Bike->Rider->GetSkinnedAsset()));Body->SetWorldLocationAndRotation(Hit.ImpactPoint,FRotator(0,-90,0));Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);Body->RegisterComponent();Body->SetHiddenInGame(false);S.Body=Body;
  // The component belongs to a visible holder so hiding the bike cannot hide it.
  Bike->SetActorHiddenInGame(false);Bike->Visual->SetVisibility(false,true);Body->SetVisibility(true);
  S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();const FVector Target=Hit.ImpactPoint+FVector(0,0,75),Offset(-260,-350,190);S.Camera->SetActorLocationAndRotation(Target+Offset,(-Offset).Rotation());
 }
 if(S.Clock==0){const TCHAR* Sides[]={TEXT("F"),TEXT("B"),TEXT("L"),TEXT("R")};const FString Path=FString::Printf(TEXT("/Game/BattleRetarget/Ellison/RecoveryScaled/GetUp_%s.GetUp_%s"),Sides[S.Clip],Sides[S.Clip]);auto* Anim=LoadObject<UAnimSequence>(nullptr,*Path);if(!Anim){S.Done=true;PC->ConsoleCommand(TEXT("quit"));return;}S.Body->PlayAnimation(Anim,false);}
 S.Clock+=Dt;PC->SetViewTarget(S.Camera.Get());if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;PC->PlayerCameraManager->UpdateCamera(0);
 const float Times[]={.15f,1.5f,3.f,4.9f};if(S.Shot<4&&S.Clock>Times[S.Shot]){FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("getup-%d-%d.png"),S.Clip,++S.Shot),false,false);}
 if(S.Clock>5.5f){UE_LOG(LogTemp,Display,TEXT("PlayerRecoveryReview: {\"clip\":%d,\"head_height\":%.3f,\"left_foot_height\":%.3f,\"right_foot_height\":%.3f}"),S.Clip,S.Body->GetSocketLocation(TEXT("Head")).Z-S.Floor,S.Body->GetSocketLocation(TEXT("Foot_L")).Z-S.Floor,S.Body->GetSocketLocation(TEXT("Foot_R")).Z-S.Floor);S.Clip++;S.Clock=0;S.Shot=0;if(S.Clip==4){S.Done=true;PC->ConsoleCommand(TEXT("quit"));}}
#endif
}
