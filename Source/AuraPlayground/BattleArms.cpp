#include "BattleRider.h"
#include "BattleInventory.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
void ABattleRider::PoseArms(float Dt){
 FirstPersonArms->SetVisibility(bWeaponDrawn);if(ArmRest.IsEmpty())return;TArray<FTransform> Pose=ArmRest;
 auto Index=[&](FName Name){return ArmNames.IndexOfByKey(Name);};
 auto Child=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=ArmParents[I];}return false;};
 auto Move=[&](int Root,FVector Target,FQuat Rotation){if(Root<0)return;const FVector Old=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();I++)if(Child(I,Root)){Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}};
 auto Limb=[&](const TCHAR* Suffix,FVector Target){
  int U=Index(*FString::Printf(TEXT("UpperArm_%s"),Suffix)),L=Index(*FString::Printf(TEXT("LowerArm_%s"),Suffix)),H=Index(*FString::Printf(TEXT("Hand_%s"),Suffix));if(U<0||L<0||H<0)return;
  const FVector Origin=Pose[U].GetLocation();const float A=FVector::Distance(Origin,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[H].GetLocation());
  const FVector Direction=(Target-Origin).GetSafeNormal();const float Distance=FMath::Clamp(FVector::Distance(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);Target=Origin+Direction*Distance;
  const float Along=(A*A-B*B+Distance*Distance)/(2*Distance),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));const FVector Bend(Suffix[0]=='R'?-1:1,0,-1);
  const FVector Joint=Origin+Direction*Along+(Bend-Direction*FVector::DotProduct(Bend,Direction)).GetSafeNormal()*Height;
  Move(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));Move(L,Joint,FQuat::FindBetweenVectors(Pose[H].GetLocation()-Pose[L].GetLocation(),Target-Joint));
  const int Middle=Index(*FString::Printf(TEXT("Middle2_%s"),Suffix));if(Middle>=0){const FQuat Align=FQuat::FindBetweenVectors(Pose[Middle].GetLocation()-Pose[H].GetLocation(),FVector(0,1,0));Move(H,Pose[H].GetLocation(),Align);}
  Move(H,Pose[H].GetLocation(),FQuat(FVector::RightVector,FMath::DegreesToRadians(Suffix[0]=='R'?-90.f:90.f)));
  // Preserve hand proportions by default; the view rig allows per-weapon fitting.
  for(int I=H;I<Pose.Num();I++)if(Child(I,H)){Pose[I].SetLocation(Pose[H].GetLocation()+(Pose[I].GetLocation()-Pose[H].GetLocation())*HandScale);Pose[I].SetScale3D(Pose[I].GetScale3D()*HandScale);}
  for(const TCHAR* Finger:{TEXT("Index"),TEXT("Middle"),TEXT("Ring"),TEXT("Pinky")})for(int JointNumber=2;JointNumber<=4;JointNumber++){
   const int F=Index(*FString::Printf(TEXT("%s%d_%s"),Finger,JointNumber,Suffix));if(F<0)continue;float Curl=JointNumber==2?35:65;if(FCString::Strcmp(Finger,TEXT("Index"))==0&&Suffix[0]=='R')Curl*=.5f;Move(F,Pose[F].GetLocation(),FQuat(Suffix[0]=='R'?FVector::UpVector:-FVector::UpVector,FMath::DegreesToRadians(-Curl)));
  }
 };
 const FQuat Motion=(Weapon->GetRelativeRotation()-GunRestRotation).Quaternion();const FVector Gun=Weapon->GetRelativeLocation();const float Reload=ReloadRemaining>0?FMath::Sin(PI*FMath::Clamp((BattleWeapons::ReloadSeconds(CurrentWeapon)-ReloadRemaining)/BattleWeapons::ReloadSeconds(CurrentWeapon),0.f,1.f)):0;
 FVector Right=Gun+Motion.RotateVector(RightWristOffset);FVector Left=Gun+Motion.RotateVector(LeftWristOffset)+FVector(-4,-6,-18)*Reload;
 if(CurrentWeapon>0)Left=Gun+Motion.RotateVector(FVector(12,-3,-5));
 if(CurrentWeapon==4){Right=Gun+Motion.RotateVector(FVector(-12,3,-16));Left=Gun+Motion.RotateVector(FVector(-12,-3,-14))+FVector(-6,-4,-6)*Reload;}
 if(MeleeRemaining>0){Right=MeleeRoot->GetRelativeTransform().TransformPosition(FVector(0,0,-31));Left=FVector(20,-20,-28);}
 RightGrip=FirstPersonArms->GetRelativeTransform().InverseTransformPosition(Right);LeftGrip=FirstPersonArms->GetRelativeTransform().InverseTransformPosition(Left);
 Limb(TEXT("R"),RightGrip);Limb(TEXT("L"),LeftGrip);
 for(int I=0;I<Pose.Num();I++)FirstPersonArms->BoneSpaceTransforms[I]=ArmParents[I]>=0?Pose[I].GetRelativeTransform(Pose[ArmParents[I]]):Pose[I];FirstPersonArms->MarkRefreshTransformDirty();
}
