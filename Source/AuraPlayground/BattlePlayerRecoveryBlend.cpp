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
 const bool Detailed=Ref.FindBoneIndex(TEXT("pelvis"))>=0;
 const int32 Hip=Ref.FindBoneIndex(Detailed?TEXT("pelvis"):TEXT("Hips")),Head=Ref.FindBoneIndex(TEXT("Head"));if(Hip<0||Head<0)return false;
 TArray<FTransform> Landed;for(int32 I=0;I<Ref.GetNum();I++)Landed.Add(Display?Display->GetBoneTransform(I):Physics->GetBoneTransform(I));
 const FVector HipWorld=Landed[Hip].GetLocation(),Heading=(Landed[Head].GetLocation()-HipWorld).GetSafeNormal2D();
 float Best=TNumericLimits<float>::Max(),HeadingBest=TNumericLimits<float>::Max();FTransform Frame;
 TArray<int32> FitBones;for(const FName Name:{FName(Detailed?TEXT("pelvis"):TEXT("Hips")),FName(TEXT("Head")),FName(Detailed?TEXT("hand_l"):TEXT("Hand_L")),FName(Detailed?TEXT("hand_r"):TEXT("Hand_R")),FName(Detailed?TEXT("foot_l"):TEXT("Foot_L")),FName(Detailed?TEXT("foot_r"):TEXT("Foot_R"))}){const int32 Index=Ref.FindBoneIndex(Name);if(Index<0)return false;FitBones.Add(Index);}
 const TCHAR* Sides[]={TEXT("F"),TEXT("B"),TEXT("L"),TEXT("R")};
 for(int32 C=0;C<4;C++){
  const FString Path=Detailed?FString::Printf(TEXT("/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_%s"),Sides[C]):FString::Printf(TEXT("/Game/BattleRetarget/Ellison/RecoveryScaled/GetUp_%s.GetUp_%s"),Sides[C],Sides[C]);
  UAnimSequence* Anim=LoadObject<UAnimSequence>(nullptr,*Path);if(!Anim)continue;
  const auto Start=Sample(Anim,Ref,0,true);const FVector LocalHeading=(Start[Head].GetLocation()-Start[Hip].GetLocation()).GetSafeNormal2D();
  double Dot=0,Cross=0;
  for(int32 I:FitBones){const FVector A=Start[I].GetLocation()-Start[Hip].GetLocation(),B=Landed[I].GetLocation()-HipWorld;Dot+=A.X*B.X+A.Y*B.Y;Cross+=A.X*B.Y-A.Y*B.X;}
  const float HeadingYaw=Heading.Rotation().Yaw-LocalHeading.Rotation().Yaw;
  const float FitYaw=FMath::Abs(Dot)+FMath::Abs(Cross)>UE_SMALL_NUMBER?FMath::RadiansToDegrees(FMath::Atan2(Cross,Dot)):HeadingYaw;
  FVector LocalCenter=FVector::ZeroVector,WorldCenter=FVector::ZeroVector;
  for(int32 I:FitBones){LocalCenter+=Start[I].GetLocation();WorldCenter+=Landed[I].GetLocation();}LocalCenter/=FitBones.Num();WorldCenter/=FitBones.Num();
  double CenterDot=0,CenterCross=0;for(int32 I:FitBones){const FVector A=Start[I].GetLocation()-LocalCenter,B=Landed[I].GetLocation()-WorldCenter;CenterDot+=A.X*B.X+A.Y*B.Y;CenterCross+=A.X*B.Y-A.Y*B.X;}
  const float CenterYaw=FMath::Abs(CenterDot)+FMath::Abs(CenterCross)>UE_SMALL_NUMBER?FMath::RadiansToDegrees(FMath::Atan2(CenterCross,CenterDot)):HeadingYaw;
  // Compare the original candidate, hip-anchored rotation, and full planar rigid fit.
  // Every candidate still requires floor support and standing capsule clearance.
  for(int32 Fit=0;Fit<3;Fit++){
  const FRotator Rotation(0,Fit==2?CenterYaw:Fit==1?FitYaw:HeadingYaw,0);FVector Origin=Fit==2?WorldCenter-Rotation.RotateVector(LocalCenter):HipWorld-Rotation.RotateVector(Start[Hip].GetLocation());
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
   for(const int32 I:FitBones){Error+=FVector::DistSquared(Candidate.TransformPosition(Start[I].GetLocation()),Landed[I].GetLocation());}
   if(Fit==0)HeadingBest=FMath::Min(HeadingBest,Error);
   if(Error<Best){Best=Error;Choice=C;Frame=Candidate;Clip=Anim;FloorZ=Origin.Z;}
  }
  }
 }
 if(Choice<0)return false;
 UE_LOG(LogTemp,Display,TEXT("PlayerRecoveryFit: {\"heading_rms_cm\":%.3f,\"selected_rms_cm\":%.3f}"),HeadingBest<TNumericLimits<float>::Max()?FMath::Sqrt(HeadingBest/6):-1.f,FMath::Sqrt(Best/6));
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
 auto Local=Sample(Clip.Get(),Ref,FMath::Clamp(Clock-.35f,0.f,Clip->GetPlayLength()),false);const float Alpha=FMath::SmoothStep(0.f,1.f,FMath::Clamp(Clock/.35f,0.f,1.f));
 for(int32 I=0;I<Local.Num();I++){FTransform T;T.Blend(LandedLocal[I],Local[I],Alpha);Local[I]=T;}
 Pose->BoneSpaceTransforms=Local;Pose->MarkRefreshTransformDirty();Pose->RefreshBoneTransforms();return Clock>=Clip->GetPlayLength()+.5f;
}
