#include "BattleQuest.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleCheckpoints.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"

void ABattleQuest::SpawnCheckpointMarkers(){
 for(int32 I=0;I<CheckpointLocations.Num();I++){
  const auto& A=BattleCheckpoints::Anchors[I];const FRotator Heading(0,A.Yaw,0);
  const FVector Side=Heading.RotateVector(FVector(0,190,0));
  auto* Marker=GetWorld()->SpawnActor<AStaticMeshActor>(CheckpointLocations[I]+Side+FVector(0,0,100),Heading);
  if(!Marker)continue;
  auto* Mesh=Marker->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);
  Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
  Mesh->SetWorldScale3D(FVector(.18,.18,2));Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);Mesh->SetCanEverAffectNavigation(false);
  Mesh->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_ShotGlow.M_ShotGlow")));
  auto* Text=NewObject<UTextRenderComponent>(Marker);Text->SetupAttachment(Mesh);
  Text->SetAbsolute(false,true,true);Text->SetWorldRotation(Heading+FRotator(0,180,0));Text->SetWorldScale3D(FVector(1));
  Text->SetRelativeLocation(FVector(0,0,70));Text->SetText(FText::FromString(FString::Printf(TEXT("CHECKPOINT\n%s"),A.Name)));Text->SetWorldSize(30);Text->SetHorizontalAlignment(EHTA_Center);Text->SetTextRenderColor(FColor(255,205,70));Text->SetCollisionEnabled(ECollisionEnabled::NoCollision);Text->RegisterComponent();
  CheckpointMarkers.Add(Marker);
 }
}
void ABattleQuest::UpdateCheckpoints(float Dt){
 CheckpointNoticeTime=FMath::Max(0.f,CheckpointNoticeTime-Dt);
 if(!bCollected||!CheckpointLocations.IsValidIndex(NextCheckpoint))return;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);if(!Pawn)return;
 auto* Bike=Cast<ABattleBike>(Pawn);if(auto* Foot=Cast<ABattleRider>(Pawn))Bike=Foot->ParkedBike;
 if(!Bike||Bike->RiderHealth<=0||Bike->RespawnRemaining>0)return;
 const FVector Target=CheckpointLocations[NextCheckpoint];
 if(FVector::Dist2D(Pawn->GetActorLocation(),Target)>320||FMath::Abs(Pawn->GetActorLocation().Z-Target.Z)>180)return;
 FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleCheckpoint),false,Pawn);Q.AddIgnoredActor(Bike);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Pawn->GetActorLocation(),Target+FVector(0,0,98),ECC_Visibility,Q))return;
 const auto& Anchor=BattleCheckpoints::Anchors[NextCheckpoint];
 Bike->CheckpointTransform=FTransform(FRotator(0,Anchor.Yaw,0),Target+FVector(0,0,98));Bike->CheckpointName=Anchor.Name;
 CheckpointNotice=TEXT("Checkpoint: ")+Bike->CheckpointName;CheckpointNoticeTime=4;
 NextCheckpoint++;
 RouteEndIndex=CheckpointIndices.IsValidIndex(NextCheckpoint)?CheckpointIndices[NextCheckpoint]:Mainline.Num()-1;
 RouteTargetLocation=Mainline.IsValidIndex(RouteEndIndex)?Mainline[RouteEndIndex]:ExitLocation;RefreshRoute();
 UE_LOG(LogTemp,Display,TEXT("BattleCheckpoint: %s; completed=%d"),*Bike->CheckpointName,NextCheckpoint);
}
