#include "BattleTrafficSignal.h"
#include "BattleRoadCrossing.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif
void TickBattleTrafficSignalReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadCrossing> Gate;float Clock=0;int Phase=0;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Phase==7||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 if(S.Phase==0){
  FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(TrafficSignalReview),true);if(!PC->GetWorld()->LineTraceSingleByChannel(Hit,FVector(-18500,13900,3000),FVector(-18500,13900,-3000),ECC_Visibility,Q)){S.Phase=7;PC->ConsoleCommand(TEXT("quit"));return;}
  auto* Gate=PC->GetWorld()->SpawnActor<ABattleRoadCrossing>();Gate->SetActorLocation(Hit.ImpactPoint+FVector(1000,0,130));S.Gate=Gate;
  auto* Signal=PC->GetWorld()->SpawnActor<ABattleTrafficSignal>();Signal->SetActorLocation(Hit.ImpactPoint);Signal->Crossing=Gate;
  #if WITH_EDITOR
  if(GShaderCompilingManager)GShaderCompilingManager->FinishAllCompilation();
  #endif
  auto* Camera=PC->GetWorld()->SpawnActor<ACameraActor>();const FVector Offset(-950,-420,60),Look=Hit.ImpactPoint+FVector(0,0,230);Camera->SetActorLocation(Look+Offset);Camera->SetActorRotation((-Offset).Rotation());Camera->GetCameraComponent()->SetFieldOfView(48);PC->SetViewTarget(Camera);if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;S.Phase=1;S.Clock=0;
 }
 FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);
 if(S.Phase==1&&S.Clock>2){IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/TEXT("signal-red.png"),false,false);S.Phase=2;}
 if(S.Phase==2&&S.Clock>3){S.Gate->bVehicleAmber=true;S.Phase=3;}
 if(S.Phase==3&&S.Clock>4){FScreenshotRequest::RequestScreenshot(Folder/TEXT("signal-amber.png"),false,false);S.Phase=4;}
 if(S.Phase==4&&S.Clock>5){S.Gate->bVehicleAmber=false;S.Gate->bVehicleGreen=true;S.Phase=5;}
 if(S.Phase==5&&S.Clock>6){FScreenshotRequest::RequestScreenshot(Folder/TEXT("signal-green.png"),false,false);S.Phase=6;}
 if(S.Phase==6&&S.Clock>7){S.Phase=7;UE_LOG(LogTemp,Display,TEXT("TrafficSignalReview: complete"));PC->ConsoleCommand(TEXT("quit"));}
#endif
}
