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
 // Everything below is written in the scooter's own frame and converted once.
 // The numbers read against the prop: deck 110 long and 20 wide, bar at x=48.
 const float Drift=float(((Variant%4)+4)%4);
 // A scooter stance, not a stride. The front foot goes forward on the tape and
 // the rear foot turns across it; two parallel feet under a straight-legged
 // body is a person halted mid-walk, which is what this read as.
 const FVector FrontSole=ScooterToRider(FVector(25.f+Drift,-5,0));
 const FVector RearSole =ScooterToRider(FVector(-21.f+Drift,5,0));
 // Grips are 5 cm tubes at x=48, y=+/-18, z=100 in scooter space. The soles
 // ride 4.5 above the anchor, so the tube is 95.5 above the mesh origin.
 const FVector LeftGrip =ScooterToRider(FVector(48,-18,95.5f));
 const FVector RightGrip=ScooterToRider(FVector(48,18,95.5f));
 // The tube runs along the scooter's Y, which is the mesh's X. Fingers curl
 // about that, the way ABattleBike curls them about its own bar.
 const FVector GripAxis(1,0,0);
 // Crouch and lean from the hips, then re-solve the legs below, so the knees
 // carry the weight and the soles stay where they were put.
 const float Lean=-(15.f+2.f*Drift);
 const int Hips=Index(TEXT("Hips"));
 if(Hips>=0)MoveBranch(Hips,Pose[Hips].GetLocation()+FVector(0,-4,-13),
  FQuat(FVector(1,0,0),FMath::DegreesToRadians(Lean)));
 // Plant the feet. The ankle sits 8 cm above and 12 cm behind the sole - the
 // offset ABattleBike uses on this mesh - and the bend is FORWARD, the same
 // hint the bike passes. Bend(0,-1,0) bows both knees backwards, and a rider
 // with backward knees standing bolt upright is most of why this did not read
 // as riding a scooter.
 const FVector AnkleOffset(0,-12,8);
 Limb(TEXT("UpperLeg_L"),TEXT("LowerLeg_L"),TEXT("Foot_L"),FrontSole+AnkleOffset,FVector(0,1,0));
 Limb(TEXT("UpperLeg_R"),TEXT("LowerLeg_R"),TEXT("Foot_R"),RearSole+AnkleOffset,FVector(0,1,0));
 // Leg IK carries the shoe round with the ankle. Put each shoe back flat on the
 // tape, then turn the rear one across the deck the way a rider stands.
 for(const TCHAR* Name:{TEXT("Foot_L"),TEXT("Foot_R")}){
  const int Foot=Index(Name);
  if(Foot>=0)MoveBranch(Foot,Pose[Foot].GetLocation(),ReferencePose[Foot].GetRotation()*Pose[Foot].GetRotation().Inverse());
 }
 const int RearFoot=Index(TEXT("Foot_R"));
 if(RearFoot>=0)MoveBranch(RearFoot,Pose[RearFoot].GetLocation(),FQuat(FVector(0,0,1),FMath::DegreesToRadians(48.f)));
 for(int Sign:{-1,1}){
  const TCHAR* Side=Sign>0?TEXT("L"):TEXT("R");
  const FString S(Side);
  const FVector Grip=Sign>0?LeftGrip:RightGrip;
  const int Upper=Index(*(TEXT("UpperArm_")+S));
  // Stop the wrist short of the tube so the palm, not the wrist, meets the bar.
  const FVector Wrist=Upper>=0?Grip-(Grip-Pose[Upper].GetLocation()).GetSafeNormal()*10.f:Grip;
  Limb(*(TEXT("UpperArm_")+S),*(TEXT("LowerArm_")+S),*(TEXT("Hand_")+S),Wrist,FVector(Sign,.5f,-1.f));
  // Then close the hand over it. Fingers left open beside a pole is the other
  // half of why this did not read as riding.
  for(const TCHAR* Finger:{TEXT("Index"),TEXT("Middle"),TEXT("Ring"),TEXT("Pinky")})
   for(int Joint=2;Joint<=4;Joint++){
    const int I=Index(*FString::Printf(TEXT("%s%d_%s"),Finger,Joint,Side));
    if(I>=0)MoveBranch(I,Pose[I].GetLocation(),FQuat(GripAxis,FMath::DegreesToRadians(Joint==2?-55.f:Joint==3?-65.f:-35.f)));
   }
  for(int Joint=2;Joint<=3;Joint++){
   const int I=Index(*FString::Printf(TEXT("Thumb%d_%s"),Joint,Side));
   if(I>=0)MoveBranch(I,Pose[I].GetLocation(),FQuat(GripAxis,FMath::DegreesToRadians(-30.f)));
  }
 }
 // The lean tips the whole body. A rider looks where he is going, not at the
 // deck, so put the head back level.
 const int Head=Index(TEXT("Head"));
 if(Head>=0)MoveBranch(Head,Pose[Head].GetLocation(),FQuat(FVector(1,0,0),FMath::DegreesToRadians(Lean*-.9f)));
 for(int I=0;I<Pose.Num();++I)Body->BoneSpaceTransforms[I]=Parents[I]>=0?Pose[I].GetRelativeTransform(Pose[Parents[I]]):Pose[I];
 Body->MarkRefreshTransformDirty();
 Body->RefreshBoneTransforms();
#if !UE_BUILD_SHIPPING
 // The one thing a render cannot settle comfortably: are the soles on the tape?
 // Mesh z=0 is the sole plane and the ankle solves 8 above it, so both feet
 // should land near 8 with no shortfall. A shortfall means the leg was asked to
 // reach further than it is long and the solver clamped, which leaves a rider
 // standing on air above the deck.
 const int FL=Index(TEXT("Foot_L")),FR=Index(TEXT("Foot_R"));
 if(FL>=0&&FR>=0)UE_LOG(LogTemp,Display,TEXT("ScooterRider: variant=%d ankle_z=%.1f/%.1f short=%.1f/%.1f head_z=%.1f"),
  Variant,Pose[FL].GetLocation().Z,Pose[FR].GetLocation().Z,
  FVector::Dist(Pose[FL].GetLocation(),FrontSole+AnkleOffset),FVector::Dist(Pose[FR].GetLocation(),RearSole+AnkleOffset),
  Head>=0?Pose[Head].GetLocation().Z:0.f);
#endif
 return 1;
}
