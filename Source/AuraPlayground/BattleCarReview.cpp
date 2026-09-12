#include "BattleRoadCar.h"
#include "Engine/World.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
void TickBattleCarReview(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<ABattleRoadCar> Driving;TWeakObjectPtr<ACameraActor> Camera;float Clock=0;int Phase=0,Paint=0;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Phase==3||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 const bool Palette=FParse::Param(FCommandLine::Get(),TEXT("BattleCarPaletteReview"));
 if(S.Phase==0){
  auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar"));
  FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(CarReviewFloor),true);
  if(!Mesh||!PC->GetWorld()->LineTraceSingleByChannel(Hit,FVector(-18600,13075,5000),FVector(-18600,13075,-5000),ECC_Visibility,Query)){PC->ConsoleCommand(TEXT("quit"));S.Phase=3;return;}
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleDrivingCarReview"))){
   auto* Driving=PC->GetWorld()->SpawnActor<ABattleRoadCar>();Driving->Route={FVector(-19800,13075,350),FVector(-15800,13015,300)};
   if(!Driving->StartRoute()){PC->ConsoleCommand(TEXT("quit"));S.Phase=3;return;}S.Driving=Driving;if(Palette)Driving->ApplyPaint(0);
  }else{
  auto* Car=PC->GetWorld()->SpawnActor<ASkeletalMeshActor>();auto* Body=Car->GetSkeletalMeshComponent();Body->SetMobility(EComponentMobility::Movable);Body->SetSkeletalMeshAsset(Mesh);Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);Body->UpdateBounds();
  auto AddPart=[&](const TCHAR* Path,FVector Position,FRotator Rotation){auto* Part=NewObject<UStaticMeshComponent>(Car);Part->SetupAttachment(Body);Part->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Path));Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetRelativeLocation(Position);Part->SetRelativeRotation(Rotation);Part->RegisterComponent();return Part;};
  AddPart(TEXT("/Game/Vehicles/SportsCar/SM_SportsCar.SM_SportsCar"),FVector::ZeroVector,FRotator::ZeroRotator);
  AddPart(TEXT("/Game/Vehicles/SportsCar/SM_SportsCar_Glass.SM_SportsCar_Glass"),FVector::ZeroVector,FRotator::ZeroRotator);
  float Bottom=BIG_NUMBER;
  for(const TCHAR* Bone:{TEXT("Phys_Wheel_FL"),TEXT("Phys_Wheel_BL"),TEXT("Phys_Wheel_FR"),TEXT("Phys_Wheel_BR")}){
   const FVector Position=Body->GetSocketTransform(Bone,RTS_Component).GetLocation();Bottom=FMath::Min(Bottom,float(Position.Z-39.2669));
   AddPart(TEXT("/Game/Vehicles/SportsCar/SM_SportsCar_Wheel.SM_SportsCar_Wheel"),Position,FRotator(0,Position.Y<0?180:0,0));
   UE_LOG(LogTemp,Display,TEXT("CarReviewWheel: bone=%s position=%s"),Bone,*Position.ToString());
  }
  Car->SetActorLocation(Hit.ImpactPoint-FVector(0,0,Bottom));
  UE_LOG(LogTemp,Display,TEXT("CarReview: static assembly ready"));
  }
  auto* Camera=PC->GetWorld()->SpawnActor<ACameraActor>();S.Camera=Camera;const FVector Look=Hit.ImpactPoint+FVector(0,0,65),Offset(650,-550,280);Camera->SetActorLocation(Look+Offset);Camera->SetActorRotation((-Offset).Rotation());Camera->GetCameraComponent()->SetFieldOfView(45);PC->SetViewTarget(Camera);if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;
  S.Phase=1;S.Clock=0;
 }
 if(S.Driving.IsValid()&&S.Camera.IsValid()){const FVector Offset(650,-550,200);S.Camera->SetActorLocation(S.Driving->GetActorLocation()+Offset);S.Camera->SetActorRotation((-Offset).Rotation());}
 // Gameplay can restore the rider camera; keep the opt-in review on its subject.
 if(S.Camera.IsValid())PC->SetViewTarget(S.Camera.Get());
 if(S.Phase==1&&S.Clock>3){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/(Palette?FString::Printf(TEXT("car-paint-%d.png"),S.Paint):FString(TEXT("car.png"))),false,false);S.Phase=2;
 }
 if(Palette&&S.Driving.IsValid()&&S.Phase==2&&S.Clock>3.5f&&S.Paint<5){++S.Paint;S.Driving->ApplyPaint(S.Paint);S.Phase=1;S.Clock=0;}
 if(S.Phase==2&&S.Clock>4){UE_LOG(LogTemp,Display,TEXT("CarReview: complete"));S.Phase=3;PC->ConsoleCommand(TEXT("quit"));}
#endif
}
