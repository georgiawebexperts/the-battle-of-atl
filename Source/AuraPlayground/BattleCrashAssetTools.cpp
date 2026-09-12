#include "PiedmontWorldTools.h"
#include "PhysicsEngine/PhysicsAsset.h"
#if WITH_EDITOR
#include "PhysicsAssetUtils.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "UObject/Package.h"
#endif
UPhysicsAsset* UPiedmontWorldTools::CreatePlayerCrashPhysics(){
#if WITH_EDITOR
 const TCHAR* Path=TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV6");
 if(LoadObject<UPhysicsAsset>(nullptr,TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV6.PA_EllisonCrashCandidateV6")))return nullptr;
 auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));if(!Mesh)return nullptr;
 auto* Asset=NewObject<UPhysicsAsset>(CreatePackage(Path),TEXT("PA_EllisonCrashCandidateV6"),RF_Public|RF_Standalone);
 FPhysAssetCreateParams Params;Params.MinBoneSize=.1f;Params.GeomType=EFG_Sphyl;Params.bCreateConstraints=true;Params.bDisableCollisionsByDefault=true;
 FText Error;if(!FPhysicsAssetUtils::CreateFromSkeletalMesh(Asset,Mesh,Params,Error,false)){UE_LOG(LogTemp,Error,TEXT("EllisonPhysics: %s"),*Error.ToString());return nullptr;}
 // The imported skeleton has a 100x armature scale. Auto-fit minimums produced
 // identical oversized capsules, so fit the weighted mesh surface in bone space.
 const auto& Ref=Mesh->GetRefSkeleton();TArray<FTransform> Global;
 for(int I=0;I<Ref.GetNum();I++){FTransform T=Ref.GetRefBonePose()[I];const int Parent=Ref.GetParentIndex(I);if(Parent>=0)T=T*Global[Parent];Global.Add(T);}
 TMap<int32,TArray<FVector>> Points;TSet<int32> Physical;
 for(const auto& Setup:Asset->SkeletalBodySetups)Physical.Add(Ref.FindBoneIndex(Setup->BoneName));
 if(!Mesh->GetImportedModel()||Mesh->GetImportedModel()->LODModels.Num()==0)return nullptr;
 for(const auto& Section:Mesh->GetImportedModel()->LODModels[0].Sections)for(const auto& Vertex:Section.SoftVertices){
  int32 Strongest=0;for(int32 I=1;I<MAX_TOTAL_INFLUENCES;I++)if(Vertex.InfluenceWeights[I]>Vertex.InfluenceWeights[Strongest])Strongest=I;
  int32 Bone=Section.BoneMap[Vertex.InfluenceBones[Strongest]];while(Bone>=0&&!Physical.Contains(Bone))Bone=Ref.GetParentIndex(Bone);
  if(Bone>=0)Points.FindOrAdd(Bone).Add(Global[Bone].InverseTransformPosition(FVector(Vertex.Position)));
 }
 for(auto& Setup:Asset->SkeletalBodySetups){
  const FString Name=Setup->BoneName.ToString();const int I=Ref.FindBoneIndex(Setup->BoneName);if(I<0)continue;
  const auto* Cloud=Points.Find(I);if(!Cloud||Cloud->Num()<4)return nullptr;
  FVector Center=FVector::ZeroVector;for(const FVector& P:*Cloud)Center+=P;Center/=Cloud->Num();
  const float Scale=Global[I].GetScale3D().GetAbsMax(),Margin=1.5f/FMath::Max(Scale,.001f);
  FKConvexElem Hull;for(const FVector& P:*Cloud)Hull.VertexData.Add(P+(P-Center).GetSafeNormal()*Margin);Hull.UpdateElemBox();
  Setup->AggGeom.EmptyElements();Setup->AggGeom.ConvexElems.Add(Hull);Setup->InvalidatePhysicsData();
  UE_LOG(LogTemp,Display,TEXT("EllisonFit: %s vertices=%d hull_size_cm=%s"),*Name,Cloud->Num(),*(Hull.ElemBox.GetSize()*Scale).ToString());
 }
 UE_LOG(LogTemp,Display,TEXT("EllisonPhysics: bodies=%d constraints=%d"),Asset->SkeletalBodySetups.Num(),Asset->ConstraintSetup.Num());
 for(const auto& Body:Asset->SkeletalBodySetups)UE_LOG(LogTemp,Display,TEXT("EllisonPhysicsBone: %s"),*Body->BoneName.ToString());
 Asset->MarkPackageDirty();return Asset;
#else
 return nullptr;
#endif
}
