#include "BattlePlayerRecoveryBlend.h"
#include "BattleBike.h"
#include "BattleFallenBike.h"
#include "EngineUtils.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
namespace {
TArray<FTransform> Sample(UAnimSequence* Clip,const FReferenceSkeleton& Ref,float Time,bool Components){
 TArray<FTransform> Result;Result.SetNum(Ref.GetNum());
 for(int32 I=0;I<Ref.GetNum();I++){
  Result[I]=Ref.GetRefBonePose()[I];const int32 Index=Clip->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Ref.GetBoneName(I));
  if(Index>=0)Clip->GetBoneTransform(Result[I],FSkeletonPoseBoneIndex(Index),FAnimExtractContext(double(Time)),false);
  const int32 Parent=Ref.GetParentIndex(I);if(Components&&Parent>=0)Result[I]=Result[I]*Result[Parent];
 }return Result;
}
}
void FBattlePlayerRecoveryBlend::Reset(){
 if(Pose.IsValid())Pose->DestroyComponent();
 Pose.Reset();Clip.Reset();LandedLocal.Empty();Clock=TransferError=FloorZ=0;Choice=-1;
}
bool FBattlePlayerRecoveryBlend::Begin(ABattleBike* Bike,USkeletalMeshComponent* Physics,UPoseableMeshComponent* Display){
 Reset();
 if(!IsValid(Bike)||!IsValid(Physics)||!Bike->GetWorld())return false;
 USkeletalMesh* Mesh=Physics->GetSkeletalMeshAsset();if(!Mesh)return false;const auto& Ref=Mesh->GetRefSkeleton();
 const int32 Hip=Ref.FindBoneIndex(TEXT("Hips")),Head=Ref.FindBoneIndex(TEXT("Head"));if(Hip<0||Head<0)return false;
 TArray<FTransform> Landed;for(int32 I=0;I<Ref.GetNum();I++)Landed.Add(Display?Display->GetBoneTransform(I):Physics->GetBoneTransform(I));
 const FVector HipWorld=Landed[Hip].GetLocation(),Heading=(Landed[Head].GetLocation()-HipWorld).GetSafeNormal2D();
 float Best=TNumericLimits<float>::Max();FTransform Frame;
 const TCHAR* Sides[]={TEXT("F"),TEXT("B"),TEXT("L"),TEXT("R")};
 for(int32 C=0;C<4;C++){
  UAnimSequence* Anim=LoadObject<UAnimSequence>(nullptr,*FString::Printf(TEXT("/Game/BattleRetarget/Ellison/RecoveryScaled/GetUp_%s.GetUp_%s"),Sides[C],Sides[C]));if(!Anim)continue;
  const auto Start=Sample(Anim,Ref,0,true);const FVector LocalHeading=(Start[Head].GetLocation()-Start[Hip].GetLocation()).GetSafeNormal2D();
  const FRotator Rotation(0,Heading.Rotation().Yaw-LocalHeading.Rotation().Yaw,0);FVector Origin=HipWorld-Rotation.RotateVector(Start[Hip].GetLocation());
  const FVector BaseOrigin=Origin;
  for(const FVector Offset:{FVector::ZeroVector,FVector(80,0,0),FVector(-80,0,0),FVector(0,80,0),FVector(0,-80,0),FVector(160,0,0),FVector(-160,0,0),FVector(0,160,0),FVector(0,-160,0)}){
   Origin=BaseOrigin+Offset;
   FHitResult Hit;FCollisionQueryParams FloorQuery;FloorQuery.bTraceComplex=true;FloorQuery.AddIgnoredActor(Bike);FloorQuery.AddIgnoredActor(Physics->GetOwner());
   for(TActorIterator<ABattleFallenBike> It(Bike->GetWorld());It;++It)FloorQuery.AddIgnoredActor(*It);
   if(!Bike->GetWorld()->LineTraceSingleByChannel(Hit,Origin+FVector(0,0,200),Origin-FVector(0,0,400),ECC_WorldStatic,FloorQuery)||Hit.ImpactNormal.Z<.65f)continue;
   Origin.Z=Hit.ImpactPoint.Z;
   FCollisionQueryParams ClearanceQuery;ClearanceQuery.AddIgnoredActor(Bike);ClearanceQuery.AddIgnoredActor(Physics->GetOwner());FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_WorldStatic);Objects.AddObjectTypesToQuery(ECC_WorldDynamic);Objects.AddObjectTypesToQuery(ECC_PhysicsBody);Objects.AddObjectTypesToQuery(ECC_Pawn);
   if(Bike->GetWorld()->OverlapAnyTestByObjectType(Origin+FVector(0,0,98),FQuat::Identity,Objects,FCollisionShape::MakeCapsule(30,96),ClearanceQuery))continue;
   const FTransform Candidate(Rotation,Origin);float Error=0;
   for(const FName Bone:{FName(TEXT("Hips")),FName(TEXT("Head")),FName(TEXT("Hand_L")),FName(TEXT("Hand_R")),FName(TEXT("Foot_L")),FName(TEXT("Foot_R"))}){const int32 I=Ref.FindBoneIndex(Bone);Error+=FVector::DistSquared(Candidate.TransformPosition(Start[I].GetLocation()),Landed[I].GetLocation());}
   if(Error<Best){Best=Error;Choice=C;Frame=Candidate;Clip=Anim;FloorZ=Origin.Z;}
  }
 }
 if(Choice<0)return false;
 UPoseableMeshComponent* Body=NewObject<UPoseableMeshComponent>(Bike);Bike->AddInstanceComponent(Body);Body->SetSkinnedAssetAndUpdate(Mesh);Body->SetWorldTransform(Frame);Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);Body->RegisterComponent();
 TArray<FTransform> Components;for(const FTransform& T:Landed)Components.Add(T.GetRelativeTransform(Frame));
 for(int32 I=0;I<Ref.GetNum();I++){const int32 Parent=Ref.GetParentIndex(I);LandedLocal.Add(Parent>=0?Components[I].GetRelativeTransform(Components[Parent]):Components[I]);}
 Body->BoneSpaceTransforms=LandedLocal;Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
 for(int32 I=0;I<Ref.GetNum();I++)TransferError=FMath::Max(TransferError,float(FVector::Dist(Body->GetBoneTransform(I).GetLocation(),Landed[I].GetLocation())));
 if(Display)Display->SetVisibility(false);Physics->SetSimulatePhysics(false);Physics->SetVisibility(false);Physics->SetCollisionEnabled(ECollisionEnabled::NoCollision);Pose=Body;
 UE_LOG(LogTemp,Display,TEXT("PlayerRecoveryTransfer: {\"choice\":%d,\"max_transfer_error_cm\":%.4f,\"initial_pose_rms_cm\":%.3f}"),Choice,TransferError,FMath::Sqrt(Best/6));return true;
}
bool FBattlePlayerRecoveryBlend::Tick(float Dt){
 if(!Pose.IsValid()||!Clip.IsValid())return false;Clock+=Dt;const auto& Ref=Cast<USkeletalMesh>(Pose->GetSkinnedAsset())->GetRefSkeleton();
 auto Local=Sample(Clip.Get(),Ref,FMath::Max(0.f,Clock-.35f),false);const float Alpha=FMath::SmoothStep(0.f,1.f,FMath::Clamp(Clock/.35f,0.f,1.f));
 for(int32 I=0;I<Local.Num();I++){FTransform T;T.Blend(LandedLocal[I],Local[I],Alpha);Local[I]=T;}
 Pose->BoneSpaceTransforms=Local;Pose->MarkRefreshTransformDirty();Pose->RefreshBoneTransforms();return Clock>=Clip->GetPlayLength()+.5f;
}
