#pragma once
#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/SkeletalMesh.h"

/**
 * A person standing on a scooter.
 *
 * Why this exists: every scooter in the project was a vehicle with nobody on it.
 * The actor the moving traffic spawns, `BP_ScooterRider`, holds exactly one
 * component - a StaticMeshComponent pointing at `/Engine/BasicShapes/Cylinder`,
 * which is why it used to read as a grey tube - and `BattleScooterProp.h` only
 * ever built the deck, stem and wheels. So the scooters glided along the trail
 * by themselves. That is what "there are no people on the scooters" means.
 *
 * The body is the same `/Game/PiedmontRide/Rider/Casual` mesh the on-foot player
 * and the park pedestrians wear, posed with the same two-bone limb solver
 * `ABattleBike::PoseRider` uses to put its rider's hands on the bar and its
 * shoes on the pedals. The pose is solved once, when the scooter is placed: it
 * never changes, only the actor moves.
 *
 * Spaces, because this is the part that goes wrong:
 *
 *   - `Spot` and `Rotation` are the scooter's own frame (+X forward, +Z up) at
 *     the deck plane, the same anchor `BuildBattleScooter` takes.
 *   - The mesh faces +Y, so it hangs off an anchor with a fixed -90 yaw.
 *     `ScooterToRider` converts a scooter-space point into mesh space; every
 *     target below is written in scooter space and converted.
 *   - The mesh origin is at the soles, so it is lifted to the top of the grip
 *     tape and the feet solve to deck targets at z = 0.
 */
inline FVector ScooterToRider(const FVector& Scooter){return FVector(-Scooter.Y,Scooter.X,Scooter.Z);}

inline int32 BuildBattleScooterRider(AActor* Owner,USceneComponent* Attach,const FVector& Spot,const FRotator& Rotation,int32 Variant=0)
{
 if(!Owner||!Attach)return 0;
 auto* Human=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));
 if(!Human)return 0;
 // Unique names per actor, not per person: several scooters share one owner and
 // a repeated FName would make the engine rename the parts.
 static int32 Scaffold=0;
 auto* Anchor=NewObject<USceneComponent>(Owner,*FString::Printf(TEXT("ScooterRiderAnchor%d"),++Scaffold));
 Anchor->SetupAttachment(Attach);
 Anchor->RegisterComponent();
 Anchor->SetWorldLocationAndRotation(Spot,Rotation);
 auto* Body=NewObject<UPoseableMeshComponent>(Owner,*FString::Printf(TEXT("ScooterRiderBody%d"),++Scaffold));
 Body->SetupAttachment(Anchor);
 Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Body->SetCanEverAffectNavigation(false);
 Body->SetRelativeRotation(FRotator(0,-90,0));
 // The person stands on the grip tape: the deck is 7 cm thick and centred on the
 // anchor, and the tape sits on top of it, so the soles go at +4.5.
 Body->SetRelativeLocation(FVector(0,0,4.5f));
 Body->SetSkinnedAssetAndUpdate(Human);
 Body->RegisterComponent();
 Body->RefreshBoneTransforms();
 const auto& Ref=Human->GetRefSkeleton();
 if(Ref.GetNum()==0)return 0;
 TArray<int32> Parents;TArray<FName> Names;TArray<FTransform> ReferencePose;
 for(int32 I=0;I<Ref.GetNum();++I){
  Parents.Add(Ref.GetParentIndex(I));Names.Add(Ref.GetBoneName(I));
  FTransform T=Ref.GetRefBonePose()[I];if(Parents[I]>=0)T=T*ReferencePose[Parents[I]];
  ReferencePose.Add(T);
 }
 TArray<FTransform> Pose=ReferencePose;
 auto Index=[&](const TCHAR* Name){return Names.IndexOfByKey(FName(Name));};
 auto Descendant=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Parents[I];}return false;};
 auto MoveBranch=[&](int Root,const FVector& Target,const FQuat& Rotation){
  if(Root<0)return;
  const FVector Old=Pose[Root].GetLocation();
  for(int I=Root;I<Pose.Num();++I)if(Descendant(I,Root)){
   Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));
   Pose[I].SetRotation(Rotation*Pose[I].GetRotation());
  }
 };
 auto Limb=[&](const TCHAR* UpperName,const TCHAR* LowerName,const TCHAR* EndName,const FVector& Target,const FVector& Bend){
  const int U=Index(UpperName),L=Index(LowerName),E=Index(EndName);
  if(U<0||L<0||E<0)return;
  const FVector Origin=Pose[U].GetLocation();
  const float A=FVector::Distance(Origin,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[E].GetLocation());
  const FVector Direction=(Target-Origin).GetSafeNormal();
  const float D=FMath::Clamp(FVector::Distance(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);
  const FVector Goal=Origin+Direction*D;
  const float Along=(A*A-B*B+D*D)/(2*D),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));
  const FVector BendNormal=(Bend-Direction*FVector::DotProduct(Bend,Direction)).GetSafeNormal();
  const FVector Joint=Origin+Direction*Along+BendNormal*Height;
  MoveBranch(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));
  MoveBranch(L,Joint,FQuat::FindBetweenVectors(Pose[E].GetLocation()-Pose[L].GetLocation(),Goal-Joint));
 };
 // Points the solver aims at, all in scooter space, converted once.
 // A staggered stance, both soles flat on the deck between the wheels. The
 // stance and the lean walk a little with the variant so a line of riders does
 // not read as one person copied eight times.
 const float Stagger=7.f+3.f*float(((Variant%4)+4)%4);
 const FVector LeftSole=ScooterToRider(FVector(Stagger,-8,0));
 const FVector RightSole=ScooterToRider(FVector(-Stagger,8,0));
 // Wrists at the grips: 5 cm tubes at x=+48, y=+/-18, z=100 (deck plane).
 const FVector LeftWrist=ScooterToRider(FVector(46,-17,100));
 const FVector RightWrist=ScooterToRider(FVector(46,17,100));
 // Lean the whole body in from the hips. A negative turn about the character's
 // own left axis is forward, the same sense the bike's rider leans with.
 const float Lean=-(7.f+1.5f*float(((Variant%4)+4)%4));
 const int Hips=Index(TEXT("Hips"));
 if(Hips>=0)MoveBranch(Hips,Pose[Hips].GetLocation(),FQuat(FVector(1,0,0),FMath::DegreesToRadians(Lean)));
 // Then plant the feet again, so the lean moves the chest and leaves the soles
 // where they were. The ankle sits 8 cm above and 12 cm behind the sole, the
 // same offset the bike's solver uses on this mesh.
 const FVector AnkleOffset(0,-12,8);
 Limb(TEXT("UpperLeg_L"),TEXT("LowerLeg_L"),TEXT("Foot_L"),LeftSole+AnkleOffset,FVector(0,-1,0));
 Limb(TEXT("UpperLeg_R"),TEXT("LowerLeg_R"),TEXT("Foot_R"),RightSole+AnkleOffset,FVector(0,-1,0));
 // Leg IK locates the ankles but rotates the attached shoes with them. Put each
 // shoe back in its bind orientation so it stays flat on the deck.
 for(const TCHAR* Name:{TEXT("Foot_L"),TEXT("Foot_R")}){
  const int Foot=Index(Name);
  if(Foot>=0)MoveBranch(Foot,Pose[Foot].GetLocation(),ReferencePose[Foot].GetRotation()*Pose[Foot].GetRotation().Inverse());
 }
 for(int Sign:{-1,1}){
  const TCHAR* Side=Sign>0?TEXT("L"):TEXT("R");
  const FVector Wrist=Sign>0?LeftWrist:RightWrist;
  const FString S(Side);
  Limb(*(TEXT("UpperArm_")+S),*(TEXT("LowerArm_")+S),*(TEXT("Hand_")+S),Wrist,FVector(Sign*1.f,0,-.5f));
  // The hands are left in the bind pose. They land on the grips as open, which
  // reads as a hand resting on a bar; a curl would need the tube axis solved per
  // hand and is the first thing to look at if the grip read is wrong.
 }
 for(int I=0;I<Pose.Num();++I)Body->BoneSpaceTransforms[I]=Parents[I]>=0?Pose[I].GetRelativeTransform(Pose[Parents[I]]):Pose[I];
 Body->MarkRefreshTransformDirty();
 Body->RefreshBoneTransforms();
 return 1;
}
