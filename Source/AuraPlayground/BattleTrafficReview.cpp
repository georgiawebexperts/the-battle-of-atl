#include "BattleRoadCar.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void TickBattleTrafficReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ACameraActor> Camera;TWeakObjectPtr<ABattleRoadCar> Car;float Clock=0;int Phase=0;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Phase==5||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 if(!S.Camera.IsValid()){S.Camera=PC->GetWorld()->SpawnActor<ACameraActor>();S.Camera->SetActorLocation(FVector(-9000,21000,1800));S.Camera->SetActorRotation(FRotator(0,90,0));}
 PC->SetViewTarget(S.Camera.Get());if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;
 if(S.Phase==0&&S.Clock>30){float Best=BIG_NUMBER;for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("AmbientRoadCar"))&&!I->bRouteFinished){float D=FVector::DistSquared2D(I->GetActorLocation(),FVector(-10000,12500,0));if(D<Best){Best=D;S.Car=*I;}}if(!S.Car.IsValid()){S.Phase=5;PC->ConsoleCommand(TEXT("quit"));return;}S.Phase=1;}
 if(S.Car.IsValid()){
  const FVector Offset=S.Phase<3?S.Car->GetActorRotation().RotateVector(FVector(-700,-450,250)):FVector(0,3500,1800);
  S.Camera->SetActorLocation(S.Car->GetActorLocation()+Offset);S.Camera->SetActorRotation((-Offset).Rotation());S.Camera->GetCameraComponent()->SetFieldOfView(S.Phase<3?50:90);
 }
 PC->PlayerCameraManager->UpdateCamera(0);
 const float Times[]={32,34,36};
 if(S.Phase>=1&&S.Phase<=3&&S.Clock>Times[S.Phase-1]){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/FString::Printf(TEXT("traffic-%d.png"),S.Phase),false,false);
  int32 Count=0;for(TActorIterator<ABattleRoadCar> I(PC->GetWorld());I;++I)if(I->ActorHasTag(TEXT("AmbientRoadCar")))++Count;
  UE_LOG(LogTemp,Display,TEXT("TrafficVisual: {\"frame\":%d,\"cars\":%d,\"distance\":%.3f,\"speed\":%.3f}"),S.Phase,Count,S.Car->DistanceTravelled,S.Car->Speed);++S.Phase;
 }
 if(S.Phase==4&&S.Clock>37){S.Phase=5;UE_LOG(LogTemp,Display,TEXT("TrafficReview: complete"));PC->ConsoleCommand(TEXT("quit"));}
#endif
}
