#include "PiedmontWorldTools.h"
#include "PhysicsEngine/PhysicsAsset.h"
#if WITH_EDITOR
#include "PhysicsAssetUtils.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "UObject/Package.h"
#endif
UPhysicsAsset* UPiedmontWorldTools::CreatePlayerCrashPhysics(){
#if WITH_EDITOR
 const TCHAR* Path=TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV2");
 if(LoadObject<UPhysicsAsset>(nullptr,TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV2.PA_EllisonCrashCandidateV2")))return nullptr;
 auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));if(!Mesh)return nullptr;
 auto* Asset=NewObject<UPhysicsAsset>(CreatePackage(Path),TEXT("PA_EllisonCrashCandidateV2"),RF_Public|RF_Standalone);
 FPhysAssetCreateParams Params;Params.MinBoneSize=.1f;Params.GeomType=EFG_Sphyl;Params.bCreateConstraints=true;Params.bDisableCollisionsByDefault=true;
 FText Error;if(!FPhysicsAssetUtils::CreateFromSkeletalMesh(Asset,Mesh,Params,Error,false)){UE_LOG(LogTemp,Error,TEXT("EllisonPhysics: %s"),*Error.ToString());return nullptr;}
 UE_LOG(LogTemp,Display,TEXT("EllisonPhysics: bodies=%d constraints=%d"),Asset->SkeletalBodySetups.Num(),Asset->ConstraintSetup.Num());
 for(const auto& Body:Asset->SkeletalBodySetups)UE_LOG(LogTemp,Display,TEXT("EllisonPhysicsBone: %s"),*Body->BoneName.ToString());
 Asset->MarkPackageDirty();return Asset;
#else
 return nullptr;
#endif
}
