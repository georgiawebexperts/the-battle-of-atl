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
 struct FState{TWeakObjectPtr<UWorld> World;float Clock=0;int Phase=0;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Phase==3||PC->GetWorld()->GetTimeSeconds()<5)return;S.Clock+=Dt;
 if(S.Phase==0){
  auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar"));
  FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(CarReviewFloor),true);
  if(!Mesh||!PC->GetWorld()->LineTraceSingleByChannel(Hit,FVector(-18600,13075,5000),FVector(-18600,13075,-5000),ECC_Visibility,Query)){PC->ConsoleCommand(TEXT("quit"));S.Phase=3;return;}
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
  auto* Camera=PC->GetWorld()->SpawnActor<ACameraActor>();const FVector Look=Hit.ImpactPoint+FVector(0,0,65),Offset(650,-550,280);Camera->SetActorLocation(Look+Offset);Camera->SetActorRotation((-Offset).Rotation());Camera->GetCameraComponent()->SetFieldOfView(45);PC->SetViewTarget(Camera);if(PC->GetHUD())PC->GetHUD()->bShowHUD=false;
  UE_LOG(LogTemp,Display,TEXT("CarReview: mesh=%s size=%s ground=%s actor=%s"),*Mesh->GetName(),*(Body->Bounds.BoxExtent*2).ToString(),*Hit.ImpactPoint.ToString(),*Car->GetActorLocation().ToString());S.Phase=1;S.Clock=0;
 }
 if(S.Phase==1&&S.Clock>3){
  FString Folder;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Folder);IFileManager::Get().MakeDirectory(*Folder,true);FScreenshotRequest::RequestScreenshot(Folder/TEXT("car.png"),false,false);S.Phase=2;
 }
 if(S.Phase==2&&S.Clock>4){UE_LOG(LogTemp,Display,TEXT("CarReview: complete"));S.Phase=3;PC->ConsoleCommand(TEXT("quit"));}
#endif
}
