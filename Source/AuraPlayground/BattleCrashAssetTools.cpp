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
 const TCHAR* Path=TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV5");
 if(LoadObject<UPhysicsAsset>(nullptr,TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV5.PA_EllisonCrashCandidateV5")))return nullptr;
 auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));if(!Mesh)return nullptr;
 auto* Asset=NewObject<UPhysicsAsset>(CreatePackage(Path),TEXT("PA_EllisonCrashCandidateV5"),RF_Public|RF_Standalone);
 FPhysAssetCreateParams Params;Params.MinBoneSize=.1f;Params.GeomType=EFG_Sphyl;Params.bCreateConstraints=true;Params.bDisableCollisionsByDefault=true;
 FText Error;if(!FPhysicsAssetUtils::CreateFromSkeletalMesh(Asset,Mesh,Params,Error,false)){UE_LOG(LogTemp,Error,TEXT("EllisonPhysics: %s"),*Error.ToString());return nullptr;}
 // The imported skeleton has a 100x armature scale. Auto-fit minimums produced
 // identical oversized capsules, so fit explicit anatomical radii in bone space.
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
  FVector End(0,0,.1);for(int J=I+1;J<Ref.GetNum();J++)if(Ref.GetParentIndex(J)==I){End=Global[I].InverseTransformPosition(Global[J].GetLocation());break;}
  float Radius=Name.Contains(TEXT("Leg"))?6.f:Name.Contains(TEXT("Arm"))?4.f:Name.Contains(TEXT("Hand"))?4.f:Name.Contains(TEXT("Foot"))?5.f:Name==TEXT("Head")?10.f:Name==TEXT("Hips")?13.f:Name==TEXT("Chest")?13.f:Name==TEXT("Torso")?11.f:Name==TEXT("Abdomen")?11.f:5.f;
  const float Scale=Global[I].GetScale3D().GetAbsMax();Radius/=FMath::Max(Scale,.001f);
  FKSphylElem Shape;Shape.Center=End*.5f;Shape.Rotation=FQuat::FindBetweenNormals(FVector::UpVector,End.GetSafeNormal()).Rotator();Shape.Radius=Radius;Shape.Length=FMath::Max(0.f,End.Size()-2*Radius);
  if(const auto* Cloud=Points.Find(I);Cloud&&Cloud->Num()){
   const FVector Axis=End.GetSafeNormal();float Low=TNumericLimits<float>::Max(),High=-Low,Radial=0;
   for(const FVector& P:*Cloud){const float Along=FVector::DotProduct(P,Axis);Low=FMath::Min(Low,Along);High=FMath::Max(High,Along);Radial=FMath::Max(Radial,float((P-Axis*Along).Size()));}
   const float Half=FMath::Max(0.f,(High-Low)*.5f),Center=(High+Low)*.5f;float FittedRadius=0;
   for(const FVector& P:*Cloud){const float Along=FMath::Clamp(float(FVector::DotProduct(P,Axis)),Center-Half,Center+Half);FittedRadius=FMath::Max(FittedRadius,float((P-Axis*Along).Size()));}
   Shape.Center=Axis*Center;Shape.Length=Half*2;Shape.Radius=FittedRadius+1.5f/FMath::Max(Scale,.001f);
   UE_LOG(LogTemp,Display,TEXT("EllisonFit: %s vertices=%d radius_cm=%.2f length_cm=%.2f"),*Name,Cloud->Num(),Shape.Radius*Scale,Shape.Length*Scale);
  }
  Setup->AggGeom.EmptyElements();Setup->AggGeom.SphylElems.Add(Shape);Setup->InvalidatePhysicsData();
 }
 UE_LOG(LogTemp,Display,TEXT("EllisonPhysics: bodies=%d constraints=%d"),Asset->SkeletalBodySetups.Num(),Asset->ConstraintSetup.Num());
 for(const auto& Body:Asset->SkeletalBodySetups)UE_LOG(LogTemp,Display,TEXT("EllisonPhysicsBone: %s"),*Body->BoneName.ToString());
 Asset->MarkPackageDirty();return Asset;
#else
 return nullptr;
#endif
}
