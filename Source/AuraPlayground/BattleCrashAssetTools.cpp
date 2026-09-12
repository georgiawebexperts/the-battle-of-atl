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
 const TCHAR* Path=TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV3");
 if(LoadObject<UPhysicsAsset>(nullptr,TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV3.PA_EllisonCrashCandidateV3")))return nullptr;
 auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));if(!Mesh)return nullptr;
 auto* Asset=NewObject<UPhysicsAsset>(CreatePackage(Path),TEXT("PA_EllisonCrashCandidateV3"),RF_Public|RF_Standalone);
 FPhysAssetCreateParams Params;Params.MinBoneSize=.1f;Params.GeomType=EFG_Sphyl;Params.bCreateConstraints=true;Params.bDisableCollisionsByDefault=true;
 FText Error;if(!FPhysicsAssetUtils::CreateFromSkeletalMesh(Asset,Mesh,Params,Error,false)){UE_LOG(LogTemp,Error,TEXT("EllisonPhysics: %s"),*Error.ToString());return nullptr;}
 // The imported skeleton has a 100x armature scale. Auto-fit minimums produced
 // identical oversized capsules, so fit explicit anatomical radii in bone space.
 const auto& Ref=Mesh->GetRefSkeleton();TArray<FTransform> Global;
 for(int I=0;I<Ref.GetNum();I++){FTransform T=Ref.GetRefBonePose()[I];const int Parent=Ref.GetParentIndex(I);if(Parent>=0)T=T*Global[Parent];Global.Add(T);}
 for(auto& Setup:Asset->SkeletalBodySetups){
  const FString Name=Setup->BoneName.ToString();const int I=Ref.FindBoneIndex(Setup->BoneName);if(I<0)continue;
  FVector End(0,0,.1);for(int J=I+1;J<Ref.GetNum();J++)if(Ref.GetParentIndex(J)==I){End=Global[I].InverseTransformPosition(Global[J].GetLocation());break;}
  float Radius=Name.Contains(TEXT("Leg"))?6.f:Name.Contains(TEXT("Arm"))?4.f:Name.Contains(TEXT("Hand"))?4.f:Name.Contains(TEXT("Foot"))?5.f:Name==TEXT("Head")?10.f:Name==TEXT("Hips")?13.f:Name==TEXT("Chest")?13.f:Name==TEXT("Torso")?11.f:Name==TEXT("Abdomen")?11.f:5.f;
  const float Scale=Global[I].GetScale3D().GetAbsMax();Radius/=FMath::Max(Scale,.001f);
  FKSphylElem Shape;Shape.Center=End*.5f;Shape.Rotation=FQuat::FindBetweenNormals(FVector::UpVector,End.GetSafeNormal()).Rotator();Shape.Radius=Radius;Shape.Length=FMath::Max(0.f,End.Size()-2*Radius);
  Setup->AggGeom.EmptyElements();Setup->AggGeom.SphylElems.Add(Shape);Setup->InvalidatePhysicsData();
 }
 UE_LOG(LogTemp,Display,TEXT("EllisonPhysics: bodies=%d constraints=%d"),Asset->SkeletalBodySetups.Num(),Asset->ConstraintSetup.Num());
 for(const auto& Body:Asset->SkeletalBodySetups)UE_LOG(LogTemp,Display,TEXT("EllisonPhysicsBone: %s"),*Body->BoneName.ToString());
 Asset->MarkPackageDirty();return Asset;
#else
 return nullptr;
#endif
}
