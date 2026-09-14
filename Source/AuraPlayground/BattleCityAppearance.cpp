#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

void APiedmontPedestrian::InitializeCityAppearance(){
 // Specialized frisbee players and joggers retain their own animation sets until
 // appropriate native clips are integrated. This applies to ordinary walkers.
 if(GetClass()!=APiedmontPedestrian::StaticClass()||Kind!=EPiedmontPedestrianKind::Walker)return;
 const int32 Outfit=CityOutfitVariant>=0?CityOutfitVariant%6:FMath::RandHelper(6);
 const FLinearColor Shirts[]={FLinearColor(.65,.12,.08),FLinearColor(.12,.38,.2),FLinearColor(.9,.65,.24),FLinearColor(.12,.28,.65),FLinearColor(.3,.12,.35),FLinearColor(.7,.7,.65)};
 const bool Female=CityAppearanceVariant>=0?CityAppearanceVariant%2!=0:FMath::RandBool();
 const FString Base=TEXT("/Game/CitySampleCrowd/Character/");
 const FString Gender=Female?TEXT("Female"):TEXT("Male");
 const FString Prefix=Female?TEXT("f_tal_nrw"):TEXT("m_tal_nrw");
 const FString Face=Female?TEXT("f_001"):TEXT("m_001");
 const FString HairName=Female?TEXT("Hair_S_Coil"):TEXT("Hair_S_AfroFade");
 const FString Top=Outfit/2==0?(Female?TEXT("scoopneck"):TEXT("crewneck")):Outfit/2==1?TEXT("buttonOpen"):(Female?TEXT("scoopneck_croppedJacket"):TEXT("crewneck_blazer"));
 const FString MeshRoot=Base+Gender+TEXT("/NormalWeight/Meshes/")+Prefix+TEXT("_");
 const FString FaceRoot=Base+Gender+TEXT("/")+Face;
 auto* Mesh=LoadObject<USkeletalMesh>(nullptr,*(MeshRoot+TEXT("body")));
 const FString AnimRoot=Base+TEXT("Anims/Loco/");
 auto* Idle=LoadObject<UAnimSequence>(nullptr,*(AnimRoot+(Female?TEXT("FTN_Set/FTN_N_Idle_Base"):TEXT("MTN_N_Idle"))));
 auto* Walk=LoadObject<UAnimSequence>(nullptr,*(AnimRoot+(Female?TEXT("FTN_Set/FTN_N_Walk_F"):TEXT("MTN_N_Walk_F"))));
 auto* Run=LoadObject<UAnimSequence>(nullptr,*(TEXT("/Game/BattleRetarget/CrowdRun/")+Gender+TEXT("/M_Neutral_Run_Loop_F")));
 TArray<USkeletalMesh*> Parts;
 for(const FString& Path:TArray<FString>{MeshRoot+Top,MeshRoot+TEXT("jeans"),MeshRoot+TEXT("loafers"),FaceRoot+TEXT("/Face/")+Face+TEXT("_nrw_FaceMesh")}){
  auto* Part=LoadObject<USkeletalMesh>(nullptr,*Path);if(!Part)return;Parts.Add(Part);
 }
 auto* HairMesh=LoadObject<UStaticMesh>(nullptr,*(FaceRoot+TEXT("/Hair/Hair/")+HairName+TEXT("_CardsMesh_Group0_LOD0")));
 if(!Mesh||!Idle||!Walk||!Run||!HairMesh)return;
 BumpReaction=LoadObject<UAnimSequence>(nullptr,*(AnimRoot+TEXT("FTN_Set/FTN_N_BlockReact_Angry")));
 for(const TCHAR* Side:{TEXT("F"),TEXT("B"),TEXT("L"),TEXT("R")})
  RecoveryClips.Add(LoadObject<UAnimSequence>(nullptr,*(TEXT("/Game/BattleRetarget/City/")+Gender+TEXT("/M_ragdoll_getup_stand_")+Side)));
 Body->SetSkinnedAssetAndUpdate(Mesh);
 SetLocomotionClips(Idle,Walk,Run);bNativeCrowdRig=true;
 for(int32 Index=0;Index<Parts.Num();++Index){
  auto* Part=NewObject<USkeletalMeshComponent>(this,*FString::Printf(TEXT("CityOutfit%d"),Index));
  AddInstanceComponent(Part);Part->SetupAttachment(Body);
  Part->SetDisablePostProcessBlueprint(true);Part->SetSkeletalMeshAsset(Parts[Index]);
  Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);
  Part->SetLeaderPoseComponent(Body,true,false);Part->RegisterComponent();
  if(Index==0)for(int32 Slot=0;Slot<Part->GetNumMaterials();++Slot)if(auto* Material=Part->CreateDynamicMaterialInstance(Slot))Material->SetVectorParameterValue(TEXT("A_CrowdColor_main"),Shirts[Outfit]);
 }
 auto* Hair=NewObject<UStaticMeshComponent>(this,TEXT("CityHair"));AddInstanceComponent(Hair);
 Hair->SetMobility(EComponentMobility::Movable);Hair->SetStaticMesh(HairMesh);
 Hair->SetCollisionEnabled(ECollisionEnabled::NoCollision);Hair->SetCanEverAffectNavigation(false);
 Hair->SetupAttachment(Body);Hair->RegisterComponent();
 // Hair cards are authored in body coordinates. Preserve that bind offset while
 // attaching to the animated head, rather than leaving hair at the actor origin.
 Hair->AttachToComponent(Body,FAttachmentTransformRules::KeepWorldTransform,TEXT("head"));
 UE_LOG(LogTemp,Display,TEXT("CityVisitor: %s rig=%s parts=%d"),*GetName(),*Gender,Parts.Num());
}
