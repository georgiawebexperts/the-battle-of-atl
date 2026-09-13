#include "BattleBike.h"
#include "BattleDetailedRider.h"
#include "BattleRider.h"
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

void ABattleBike::AttachDetailedRiderParts(USkinnedMeshComponent* Leader,bool Visible){
 if(!bDetailedRiderPreview||!Leader)return;
 TArray<USkeletalMeshComponent*> Parts;GetComponents(Parts);
 for(auto* Part:Parts)if(Part->GetName().StartsWith(TEXT("DetailedRiderPart"))){
  Part->AttachToComponent(Leader,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  Part->SetLeaderPoseComponent(Leader,true,false);Part->SetVisibility(Visible);
 }
 TArray<UStaticMeshComponent*> Props;GetComponents(Props);
 for(auto* Part:Props)if(Part->GetName()==TEXT("DetailedRiderHair")){
  Part->AttachToComponent(Leader,FAttachmentTransformRules::KeepRelativeTransform,TEXT("head"));Part->SetVisibility(Visible);
 }
 Leader->RefreshFollowerComponents();
}

FName BattleDetailedBone(FName Name,bool Detailed){
 if(!Detailed)return Name;
 static const TMap<FName,FName> Aliases={{TEXT("Hips"),TEXT("pelvis")},{TEXT("Abdomen"),TEXT("spine_01")},{TEXT("UpperLeg_L"),TEXT("thigh_l")},{TEXT("LowerLeg_L"),TEXT("calf_l")},{TEXT("Foot_L"),TEXT("foot_l")},{TEXT("UpperArm_L"),TEXT("upperarm_l")},{TEXT("LowerArm_L"),TEXT("lowerarm_l")},{TEXT("Hand_L"),TEXT("hand_l")},{TEXT("Index2_L"),TEXT("index_01_l")},{TEXT("Index3_L"),TEXT("index_02_l")},{TEXT("Index4_L"),TEXT("index_03_l")},{TEXT("Middle2_L"),TEXT("middle_01_l")},{TEXT("Middle3_L"),TEXT("middle_02_l")},{TEXT("Middle4_L"),TEXT("middle_03_l")},{TEXT("Ring2_L"),TEXT("ring_01_l")},{TEXT("Ring3_L"),TEXT("ring_02_l")},{TEXT("Ring4_L"),TEXT("ring_03_l")},{TEXT("Pinky2_L"),TEXT("pinky_01_l")},{TEXT("Pinky3_L"),TEXT("pinky_02_l")},{TEXT("Pinky4_L"),TEXT("pinky_03_l")},{TEXT("Thumb2_L"),TEXT("thumb_01_l")},{TEXT("Thumb3_L"),TEXT("thumb_02_l")},{TEXT("UpperLeg_R"),TEXT("thigh_r")},{TEXT("LowerLeg_R"),TEXT("calf_r")},{TEXT("Foot_R"),TEXT("foot_r")},{TEXT("UpperArm_R"),TEXT("upperarm_r")},{TEXT("LowerArm_R"),TEXT("lowerarm_r")},{TEXT("Hand_R"),TEXT("hand_r")},{TEXT("Index2_R"),TEXT("index_01_r")},{TEXT("Index3_R"),TEXT("index_02_r")},{TEXT("Index4_R"),TEXT("index_03_r")},{TEXT("Middle2_R"),TEXT("middle_01_r")},{TEXT("Middle3_R"),TEXT("middle_02_r")},{TEXT("Middle4_R"),TEXT("middle_03_r")},{TEXT("Ring2_R"),TEXT("ring_01_r")},{TEXT("Ring3_R"),TEXT("ring_02_r")},{TEXT("Ring4_R"),TEXT("ring_03_r")},{TEXT("Pinky2_R"),TEXT("pinky_01_r")},{TEXT("Pinky3_R"),TEXT("pinky_02_r")},{TEXT("Pinky4_R"),TEXT("pinky_03_r")},{TEXT("Thumb2_R"),TEXT("thumb_01_r")},{TEXT("Thumb3_R"),TEXT("thumb_02_r")}};
 if(const FName* Found=Aliases.Find(Name))return *Found;return Name;
}

void ABattleRider::InitializeDetailedArmsPreview(){
#if !UE_BUILD_SHIPPING
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleDetailedRider")))return;
 auto* Hands=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/BattleForTheA/Rider/Detailed/SK_DetailedHands"));
 auto* Sleeves=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/BattleForTheA/Rider/Detailed/SK_DetailedSleeves"));
 if(!Hands||!Sleeves)return;
 FirstPersonArms->SetSkinnedAssetAndUpdate(Hands);
 UE_LOG(LogTemp,Display,TEXT("DetailedHandsMaterialBefore: %s"),*GetNameSafe(FirstPersonArms->GetMaterial(0)));
 FirstPersonArms->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/CitySampleCrowd/Character/Male/NormalWeight/Materials/M_BodySynthesized")));
 auto* Part=NewObject<USkeletalMeshComponent>(this,TEXT("DetailedFirstPersonSleeves"));AddInstanceComponent(Part);Part->SetupAttachment(FirstPersonArms);
 Part->SetDisablePostProcessBlueprint(true);Part->SetSkeletalMeshAsset(Sleeves);Part->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/CitySampleCrowd/Character/Male/NormalWeight/Materials/MI_m_nrw_crewneck")));Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);
 Part->SetOnlyOwnerSee(true);Part->SetCastShadow(false);Part->SetBoundsScale(10);Part->SetLeaderPoseComponent(FirstPersonArms,true,false);Part->RegisterComponent();
 UE_LOG(LogTemp,Display,TEXT("DetailedArmsPreview: hands=%s sleeves=%s"),*Hands->GetName(),*Sleeves->GetName());
#endif
}
