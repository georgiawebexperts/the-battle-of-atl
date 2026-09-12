#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "UnrealClient.h"
void TickBattleCanopyReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ACameraActor> Camera;int Phase=0;double Start=0,Previous=0;TArray<double> Frames;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Phase==4||PC->GetWorld()->GetTimeSeconds()<5)return;
 const double Now=FPlatformTime::Seconds();
 if(S.Phase==3){if(Now-S.Start>2){S.Phase=4;UE_LOG(LogTemp,Display,TEXT("CanopyRuntime: complete"));PC->ConsoleCommand(TEXT("quit"));}return;}
 if(!S.Camera.IsValid())S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();
 if(S.Start==0){
  const FVector XY[]={FVector(-17400,-5200,0),FVector(-6000,2000,0),FVector(0,-4000,0)};
  const FVector Target[]={FVector(-14000,-5200,0),FVector(0,-4000,0),FVector(-6000,2000,0)};
  FHitResult Hit;FCollisionQueryParams Q;Q.bTraceComplex=true;if(PC->GetPawn())Q.AddIgnoredActor(PC->GetPawn());
  if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,XY[S.Phase]+FVector(0,0,7000),XY[S.Phase]-FVector(0,0,7000),ECC_WorldStatic,Q)){S.Phase=4;UE_LOG(LogTemp,Error,TEXT("CanopyRuntime: missing floor"));PC->ConsoleCommand(TEXT("quit"));return;}
  FVector Position=Hit.ImpactPoint+FVector(0,0,170);FVector Look=Target[S.Phase];Look.Z=Position.Z;
  S.Camera->SetActorLocationAndRotation(Position,(Look-Position).Rotation());S.Camera->GetCameraComponent()->SetFieldOfView(85);
  if(APawn* Pawn=PC->GetPawn()){Pawn->SetActorLocation(Hit.ImpactPoint+FVector(0,0,98),false,nullptr,ETeleportType::TeleportPhysics);Pawn->SetActorHiddenInGame(true);if(auto* Character=Cast<ACharacter>(Pawn))Character->GetCharacterMovement()->DisableMovement();}
  S.Start=Now;S.Previous=Now;S.Frames.Reset();
 }
 PC->SetViewTarget(S.Camera.Get());if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;PC->PlayerCameraManager->UpdateCamera(0);
 if(Now-S.Start>5&&S.Previous>0)S.Frames.Add((Now-S.Previous)*1000);S.Previous=Now;
 if(Now-S.Start>9){
  S.Frames.Sort();if(S.Frames.IsEmpty())return;const double P50=S.Frames[S.Frames.Num()/2],P95=S.Frames[FMath::Min(S.Frames.Num()-1,FMath::FloorToInt(S.Frames.Num()*.95))];
  UE_LOG(LogTemp,Display,TEXT("CanopyFrame: {\"view\":%d,\"samples\":%d,\"p50_ms\":%.3f,\"p95_ms\":%.3f}"),S.Phase+1,S.Frames.Num(),P50,P95);
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("canopy-%d.png"),S.Phase+1),false,false);
  ++S.Phase;S.Start=S.Phase==3?Now:0;
 }
#endif
}
