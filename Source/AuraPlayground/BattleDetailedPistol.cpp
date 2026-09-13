#include "BattleRider.h"
#include "BattleInventory.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"

void ABattleRider::InitializeDetailedPistolPreview(){
 if(!bDetailedPlayerRig)return;
 auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/M1911/Meshes/SkeletalMesh/Rigged_M1911"));
 if(!Mesh)return;
 DetailedPistol=NewObject<UPoseableMeshComponent>(this,TEXT("DetailedM1911"));AddInstanceComponent(DetailedPistol);
 DetailedPistol->SetupAttachment(Weapon);DetailedPistol->SetSkinnedAssetAndUpdate(Mesh);
 DetailedPistol->SetRelativeRotation(FRotator(0,180,0));DetailedPistol->SetOnlyOwnerSee(true);
 DetailedPistol->SetCollisionEnabled(ECollisionEnabled::NoCollision);DetailedPistol->SetCanEverAffectNavigation(false);DetailedPistol->SetCastShadow(false);
 DetailedPistol->RegisterComponent();DetailedPistol->RefreshBoneTransforms();
 DetailedMagazineRest=DetailedPistol->GetBoneLocationByName(TEXT("Mag"),EBoneSpaces::ComponentSpace);
 Weapon->SetStaticMesh(nullptr);Weapon->SetRelativeScale3D(FVector(1));Weapon->SetRelativeRotation(FRotator::ZeroRotator);
 GunAimPosition.Z=-6.4f;
 UE_LOG(LogTemp,Display,TEXT("DetailedM1911: loaded magazine_rest=%s"),*DetailedMagazineRest.ToString());
}
void ABattleRider::UpdateDetailedPistol(){
 if(!DetailedPistol)return;
 DetailedPistol->SetVisibility(bWeaponDrawn&&CurrentWeapon==0&&MeleeRemaining<=0);
 const float Phase=ReloadRemaining>0?FMath::Clamp(1.f-ReloadRemaining/BattleWeapons::ReloadSeconds(CurrentWeapon),0.f,1.f):0;
 const float Pull=ReloadRemaining>0?FMath::Min(FMath::Clamp(Phase/.35f,0.f,1.f),FMath::Clamp((.9f-Phase)/.25f,0.f,1.f)):0;
 DetailedPistol->SetBoneLocationByName(TEXT("Mag"),DetailedMagazineRest-FVector(0,0,14*Pull),EBoneSpaces::ComponentSpace);
}
