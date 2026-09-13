#include "BattleBike.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Misc/CommandLine.h"

void ABattleBike::InitializeDetailedRiderPreview(){
#if !UE_BUILD_SHIPPING
 // Appearance/pose prototype only. Existing crash and on-foot rigs still need migration.
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleDetailedRider")))return;
 const FString Base=TEXT("/Game/CitySampleCrowd/Character/Male/");
 const FString MeshRoot=Base+TEXT("NormalWeight/Meshes/m_tal_nrw_");
 auto* Body=LoadObject<USkeletalMesh>(nullptr,*(MeshRoot+TEXT("body")));
 auto* HairMesh=LoadObject<UStaticMesh>(nullptr,*(Base+TEXT("m_001/Hair/Hair/Hair_S_AfroFade_CardsMesh_Group0_LOD0")));
 TArray<USkeletalMesh*> Parts;
 for(const FString& Path:TArray<FString>{MeshRoot+TEXT("crewneck"),MeshRoot+TEXT("jeans"),MeshRoot+TEXT("loafers"),Base+TEXT("m_001/Face/m_001_nrw_FaceMesh")}){
  auto* Part=LoadObject<USkeletalMesh>(nullptr,*Path);if(!Part)return;Parts.Add(Part);
 }
 if(!Body||!HairMesh)return;
 Rider->SetSkinnedAssetAndUpdate(Body);Rider->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;Rider->RefreshBoneTransforms();
 for(int I=0;I<Parts.Num();I++){
  auto* Part=NewObject<USkeletalMeshComponent>(this,*FString::Printf(TEXT("DetailedRiderPart%d"),I));AddInstanceComponent(Part);Part->SetupAttachment(Rider);
  Part->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;Part->SetDisablePostProcessBlueprint(true);Part->SetSkeletalMeshAsset(Parts[I]);Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);Part->SetLeaderPoseComponent(Rider,true,false);Part->RegisterComponent();
 }
 auto* Hair=NewObject<UStaticMeshComponent>(this,TEXT("DetailedRiderHair"));AddInstanceComponent(Hair);Hair->SetMobility(EComponentMobility::Movable);Hair->SetStaticMesh(HairMesh);Hair->SetCollisionEnabled(ECollisionEnabled::NoCollision);Hair->SetCanEverAffectNavigation(false);Hair->SetupAttachment(Rider);Hair->RegisterComponent();Hair->AttachToComponent(Rider,FAttachmentTransformRules::KeepWorldTransform,TEXT("head"));
 bDetailedRiderPreview=true;
 UE_LOG(LogTemp,Display,TEXT("DetailedRiderPreview: body=%s outfit_parts=%d"),*Body->GetName(),Parts.Num());
#endif
}
