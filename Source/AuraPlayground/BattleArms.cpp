#include "BattleRider.h"
#include "BattleDetailedRider.h"
#include "BattleInventory.h"
#include "BattleBike.h"
#include "Camera/CameraComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
void ABattleRider::PoseArms(float Dt){
 FirstPersonArms->SetVisibility(Health>0&&!bDetailedBodyReview,true);
 ArmPoseBlend=FMath::FInterpTo(ArmPoseBlend,(!bSwimming&&(bWeaponDrawn||MeleeRemaining>0))?1.f:0.f,Dt,12);if(ArmRest.IsEmpty())return;TArray<FTransform> Pose=ArmRest;
 const bool Detailed=ArmNames.Contains(TEXT("pelvis"));
 // The legacy arms are cut at the shoulders. Keep those open ends behind the
 // swimming camera; wrist targets remain expressed in camera space below.
 const FVector RigPosition=!Detailed&&bSwimming?FVector(-24,0,-165):FVector(22,0,-175);
 FirstPersonArms->SetRelativeLocation(FMath::VInterpTo(FirstPersonArms->GetRelativeLocation(),RigPosition,Dt,12));

 auto Index=[&](FName Name){return ArmNames.IndexOfByKey(BattleDetailedBone(Name,Detailed));};
 auto Child=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=ArmParents[I];}return false;};
 auto Move=[&](int Root,FVector Target,FQuat Rotation){if(Root<0)return;const FVector Old=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();I++)if(Child(I,Root)){Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}};
 auto Limb=[&](const TCHAR* Suffix,FVector Target){
  int U=Index(*FString::Printf(TEXT("UpperArm_%s"),Suffix)),L=Index(*FString::Printf(TEXT("LowerArm_%s"),Suffix)),H=Index(*FString::Printf(TEXT("Hand_%s"),Suffix));if(U<0||L<0||H<0)return;
  const FVector Origin=Pose[U].GetLocation();const float A=FVector::Distance(Origin,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[H].GetLocation());
  const FVector Direction=(Target-Origin).GetSafeNormal();const float Distance=FMath::Clamp(FVector::Distance(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);Target=Origin+Direction*Distance;
  const float Along=(A*A-B*B+Distance*Distance)/(2*Distance),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));const FVector Bend(Suffix[0]=='R'?-1:1,0,-1);
  const FVector Joint=Origin+Direction*Along+(Bend-Direction*FVector::DotProduct(Bend,Direction)).GetSafeNormal()*Height;
  Move(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));Move(L,Joint,FQuat::FindBetweenVectors(Pose[H].GetLocation()-Pose[L].GetLocation(),Target-Joint));
  const int Middle=Index(*FString::Printf(TEXT("Middle2_%s"),Suffix));const FVector FingerDirection=bSwimming?FVector(0,1,-.15f).GetSafeNormal():FMath::Lerp(FVector(0,.65f,-.76f).GetSafeNormal(),FVector(0,1,0),ArmPoseBlend).GetSafeNormal();
  if(Middle>=0){const FQuat Align=FQuat::FindBetweenVectors(Pose[Middle].GetLocation()-Pose[H].GetLocation(),FingerDirection);Move(H,Pose[H].GetLocation(),Align);}
  if(bSwimming){
   const int IndexKnuckle=Index(*FString::Printf(TEXT("Index2_%s"),Suffix)),PinkyKnuckle=Index(*FString::Printf(TEXT("Pinky2_%s"),Suffix));
   if(IndexKnuckle>=0&&PinkyKnuckle>=0){
    const FVector Across=FVector::VectorPlaneProject(Pose[PinkyKnuckle].GetLocation()-Pose[IndexKnuckle].GetLocation(),FingerDirection).GetSafeNormal();
    const FVector Desired(Suffix[0]=='R'?-1.f:1.f,0,0);
    const float Twist=FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(Across,Desired),FingerDirection),FVector::DotProduct(Across,Desired));
    Move(H,Pose[H].GetLocation(),FQuat(FingerDirection,Twist));
   }
  }else Move(H,Pose[H].GetLocation(),FQuat(FingerDirection,FMath::DegreesToRadians(Suffix[0]=='R'?-90.f:90.f)));
  // Preserve hand proportions by default; the view rig allows per-weapon fitting.
  for(int I=H;I<Pose.Num();I++)if(Child(I,H)){Pose[I].SetLocation(Pose[H].GetLocation()+(Pose[I].GetLocation()-Pose[H].GetLocation())*HandScale);Pose[I].SetScale3D(Pose[I].GetScale3D()*HandScale);}
  for(const TCHAR* Finger:{TEXT("Index"),TEXT("Middle"),TEXT("Ring"),TEXT("Pinky")})for(int JointNumber=2;JointNumber<=4;JointNumber++){
   const int F=Index(*FString::Printf(TEXT("%s%d_%s"),Finger,JointNumber,Suffix));if(F<0)continue;float Curl=bSwimming?8.f:FMath::Lerp(JointNumber==2?18.f:32.f,JointNumber==2?35.f:65.f,ArmPoseBlend);if(FCString::Strcmp(Finger,TEXT("Index"))==0&&Suffix[0]=='R')Curl*=.5f;Move(F,Pose[F].GetLocation(),FQuat(Suffix[0]=='R'?FVector::UpVector:-FVector::UpVector,FMath::DegreesToRadians(-Curl)));
  }
  if(Detailed&&CurrentWeapon==0&&MeleeRemaining<=0){
   const int Thumb=Index(*FString::Printf(TEXT("Thumb2_%s"),Suffix));
   const int Tip=ArmNames.IndexOfByKey(*FString::Printf(TEXT("thumb_03_%s"),Suffix[0]=='R'?TEXT("r"):TEXT("l")));
   if(Thumb>=0&&Tip>=0){
    const FVector ThumbDirection=FVector(0,1,.12f).GetSafeNormal();
    const FQuat Align=FQuat::FindBetweenVectors(Pose[Tip].GetLocation()-Pose[Thumb].GetLocation(),ThumbDirection);
    Move(Thumb,Pose[Thumb].GetLocation(),FQuat::Slerp(FQuat::Identity,Align,ArmPoseBlend));
   }
  }
  if(Detailed&&CurrentWeapon==0&&Suffix[0]=='R'&&MeleeRemaining<=0){
   const FQuat ViewToMesh=FirstPersonArms->GetRelativeRotation().Quaternion().Inverse();
   const FQuat WeaponMotion=ViewToMesh*(Weapon->GetRelativeRotation()-GunRestRotation).Quaternion()*ViewToMesh.Inverse();
   Move(H,Pose[H].GetLocation(),FQuat::Slerp(FQuat::Identity,WeaponMotion,ArmPoseBlend));
  }
 };
 const FQuat Motion=(Weapon->GetRelativeRotation()-GunRestRotation).Quaternion();const FVector Gun=Weapon->GetRelativeLocation();const float Reload=ReloadRemaining>0?FMath::Sin(PI*FMath::Clamp((BattleWeapons::ReloadSeconds(CurrentWeapon)-ReloadRemaining)/BattleWeapons::ReloadSeconds(CurrentWeapon),0.f,1.f)):0;
 FVector Right=Gun+Motion.RotateVector(RightWristOffset);FVector Left=Gun+Motion.RotateVector(LeftWristOffset)+FVector(-4,-6,bDetailedPlayerRig?-10.f:-18.f)*Reload;
 if(DetailedPistol&&CurrentWeapon==0&&ReloadRemaining>0){
  DetailedPistol->RefreshBoneTransforms();
  Left=Camera->GetComponentTransform().InverseTransformPosition(DetailedPistol->GetBoneLocationByName(TEXT("Mag"),EBoneSpaces::WorldSpace))+FVector(-6,-4,-2);
 }
 if(CurrentWeapon>0)Left=Gun+Motion.RotateVector(FVector(12,-3,-5));
 if(CurrentWeapon==1){Right=Gun+Motion.RotateVector(FVector(-39,3,-4));Left=Gun+Motion.RotateVector(FVector(10,-3,-5))+FVector(-6,-4,-8)*Reload;}
 if(CurrentWeapon==4){Right=Gun+Motion.RotateVector(FVector(-12,3,-16));Left=Gun+Motion.RotateVector(FVector(-12,-3,-14))+FVector(-6,-4,-6)*Reload;}
 if(MeleeRemaining>0){Right=MeleeRoot->GetRelativeTransform().TransformPosition(FVector(0,0,-31));Left=FVector(20,-20,-28);}
 const float Moving=GetCharacterMovement()->IsMovingOnGround()?FMath::Clamp(GetVelocity().Size2D()/(bDetailedPlayerRig?200.f:520.f),0.f,1.5f):0;
 EmptyArmMotion=FMath::FInterpTo(EmptyArmMotion,Moving,Dt,7);
 const float Swing=FMath::Sin(SwayTime),Lift=FMath::Square(FMath::Cos(SwayTime));
 const FVector FreeRight(38+Swing*16*EmptyArmMotion,25,-30+(8+7*Lift)*EmptyArmMotion);
 const FVector FreeLeft(38-Swing*16*EmptyArmMotion,-25,-30+(8+7*Lift)*EmptyArmMotion);
 Right=FMath::Lerp(FreeRight,Right,ArmPoseBlend);Left=FMath::Lerp(FreeLeft,Left,ArmPoseBlend);
 if(bSwimming){
  const float Effort=FMath::Clamp(GetVelocity().Size2D()/200.f,0.f,1.f);
  const bool Stunned=ParkedBike&&ParkedBike->StunRemaining>0;
  if(!Stunned)SwimViewPhase+=Dt*FMath::Lerp(1.5f,3.8f,Effort);
  auto Stroke=[&](float Side){const float Phase=SwimViewPhase+(Side<0?PI:0);return FVector((Detailed?38.f:22.f)+FMath::Cos(Phase)*FMath::Lerp(5.f,Detailed?20.f:8.f,Effort),Side*((Detailed?22.f:14.f)+FMath::Sin(Phase)*FMath::Lerp(5.f,Detailed?11.f:5.f,Effort)),-18+FMath::Sin(Phase)*FMath::Lerp(3.f,9.f,Effort));};
  Right=Stroke(1);Left=Stroke(-1);
 }
 RightGrip=FirstPersonArms->GetRelativeTransform().InverseTransformPosition(Right);LeftGrip=FirstPersonArms->GetRelativeTransform().InverseTransformPosition(Left);
 Limb(TEXT("R"),RightGrip);Limb(TEXT("L"),LeftGrip);
 for(int I=0;I<Pose.Num();I++)FirstPersonArms->BoneSpaceTransforms[I]=ArmParents[I]>=0?Pose[I].GetRelativeTransform(Pose[ArmParents[I]]):Pose[I];FirstPersonArms->MarkRefreshTransformDirty();
}
