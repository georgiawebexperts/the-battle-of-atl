#include "PiedmontExplorer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "UObject/ObjectKey.h"

namespace {
// Share decoded clip samples across the crowd. Avoid decompressing every bone of
// every pedestrian every frame; interpolation still runs at the render rate.
struct FCrowdClip {
 int32 Bones=0,Frames=0;
 float TravelSpeed=0;
 TArray<FTransform> Samples;
};
TMap<FObjectKey,FCrowdClip> CrowdClips;
const FCrowdClip& Clip(UAnimSequence* Animation){
 const FObjectKey Key(Animation);
 if(const auto* Cached=CrowdClips.Find(Key))return *Cached;
 FCrowdClip Result;
 Result.Bones=Animation->GetSkeleton()->GetReferenceSkeleton().GetNum();
 Result.Frames=FMath::Max(2,FMath::CeilToInt(Animation->GetPlayLength()*30.f));
 Result.Samples.SetNum((Result.Frames+1)*Result.Bones);
 for(int32 Frame=0;Frame<=Result.Frames;Frame++){
  const FAnimExtractContext Context(double(Frame)*Animation->GetPlayLength()/Result.Frames);
  for(int32 Bone=0;Bone<Result.Bones;Bone++){
   auto& Transform=Result.Samples[Frame*Result.Bones+Bone];
   Transform=Animation->GetSkeleton()->GetReferenceSkeleton().GetRefBonePose()[Bone];
   Animation->GetBoneTransform(Transform,FSkeletonPoseBoneIndex(Bone),Context,false);
  }
 }
 // Measure authored root travel before root removal so in-place playback can
 // match world movement without assuming every source clip runs at crowd speed.
 const int32 MotionRoot=Animation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(TEXT("root"));
 if(MotionRoot>=0){
  double Distance=0;
  for(int32 Frame=1;Frame<=Result.Frames;Frame++)Distance+=FVector::Dist2D(Result.Samples[Frame*Result.Bones+MotionRoot].GetTranslation(),Result.Samples[(Frame-1)*Result.Bones+MotionRoot].GetTranslation());
  Result.TravelSpeed=Distance/Animation->GetPlayLength();
 }
#if !UE_BUILD_SHIPPING
 if(Animation->GetPathName().StartsWith(TEXT("/Game/BattleRetarget/Ellison/CityLocomotion/")))UE_LOG(LogTemp,Display,TEXT("DetailedClipTravel: clip=%s speed_cm_s=%.3f"),*Animation->GetName(),Result.TravelSpeed);
 // Compare decoded ankle rotations with the bind pose before correcting retargeting.
 for(const TCHAR* Name:{TEXT("UpperLeg_L"),TEXT("LowerLeg_L"),TEXT("Foot_L"),TEXT("Foot_L_end")}){
  const auto& Ref=Animation->GetSkeleton()->GetReferenceSkeleton();
  const int32 Bone=Ref.FindBoneIndex(Name);if(Bone<0)continue;
  UE_LOG(LogTemp,Display,TEXT("CrowdClipAxes: clip=%s bone=%s bind=%s sample0=%s"),*Animation->GetName(),Name,*Ref.GetRefBonePose()[Bone].ToString(),*Result.Samples[Bone].ToString());
 }
#endif
 UE_LOG(LogTemp,Display,TEXT("CrowdClip: cached %s bones=%d frames=%d"),*Animation->GetName(),Result.Bones,Result.Frames);
 return CrowdClips.Add(Key,MoveTemp(Result));
}
FTransform Sample(const FCrowdClip& Data,int32 Bone,float Phase){
 const float Frame=FMath::Frac(Phase)*Data.Frames;
 const int32 Index=FMath::FloorToInt(Frame);
 FTransform Pose;Pose.Blend(Data.Samples[Index*Data.Bones+Bone],Data.Samples[(Index+1)*Data.Bones+Bone],Frame-Index);return Pose;
}
}

bool APiedmontExplorer::SampleLocomotion(float Dt,TArray<FTransform>& Pose){
 if(BodySequence){
  const float Duration=BodySequence->GetPlayLength();
  if(Duration<=0){StopBodySequence();return false;}
  if(!bBodySequenceHeld)BodySequenceClock+=Dt;
  const float Time=bBodySequenceLoop?FMath::Fmod(BodySequenceClock,Duration):FMath::Min(BodySequenceClock,Duration);
  const auto& Data=Clip(BodySequence);
  const auto& Ref=BodySequence->GetSkeleton()->GetReferenceSkeleton();
  for(int32 I=0;I<Pose.Num();++I){
   const int32 Bone=Ref.FindBoneIndex(Bones[I]);
   if(Bone>=0)Pose[I]=Sample(Data,Bone,FMath::Min(Time/Duration,.999999f));
   // Player airborne travel belongs to CharacterMovement, including gravity.
   // Keep the fall pose but remove authored root travel; stationary NPC recovery
   // sequences below intentionally retain their own root posture.
   if(bDetailedPlayerRig&&GetCharacterMovement()->IsFalling()&&Parents[I]<0)Pose[I]=RestPose[I];
   // Stationary full-body clips carry posture in the root (including lying down).
   // Unlike walking, their authored root transform must remain on the visual mesh.
  }
  return true;
 }
 if(!IdleAnimation||!WalkAnimation||!RunAnimation)return false;
 if(BodyAction){BodyActionClock+=Dt;if(BodyActionClock>=BodyActionEnd)BodyAction=nullptr;else Clip(BodyAction);}
 // Fill the cache before retaining references: a TMap growth may relocate values.
 Clip(IdleAnimation);Clip(WalkAnimation);Clip(RunAnimation);
 const auto& Idle=Clip(IdleAnimation);const auto& Walk=Clip(WalkAnimation);const auto& Run=Clip(RunAnimation);
 const auto& IdleSkeleton=IdleAnimation->GetSkeleton()->GetReferenceSkeleton();
 const auto& WalkSkeleton=WalkAnimation->GetSkeleton()->GetReferenceSkeleton();
 const auto& RunSkeleton=RunAnimation->GetSkeleton()->GetReferenceSkeleton();
 const float Speed=GetVelocity().Size2D();
 LocomotionSpeed=FMath::Lerp(LocomotionSpeed,Speed,1.f-FMath::Exp(-10.f*Dt));
 const float RunWeight=bDetailedPlayerRig?FMath::Clamp((LocomotionSpeed-240.f)/200.f,0.f,1.f):FMath::Clamp((LocomotionSpeed-180.f)/170.f,0.f,1.f);
 const float MoveWeight=FMath::Clamp(LocomotionSpeed/45.f,0.f,1.f);
 // Dedicated crowd runs retain authored root travel for stride-rate matching.
 const bool MeasuredStride=bDetailedPlayerRig||RunAnimation->GetPathName().StartsWith(TEXT("/Game/BattleRetarget/CrowdRun/"));
 const float ReferenceSpeed=MeasuredStride?FMath::Lerp(Walk.TravelSpeed>1.f?Walk.TravelSpeed:140.f,Run.TravelSpeed>1.f?Run.TravelSpeed:350.f,RunWeight):FMath::Lerp(140.f,350.f,RunWeight);
 const float CycleSeconds=FMath::Lerp(WalkAnimation->GetPlayLength(),RunAnimation->GetPlayLength(),RunWeight);
 LocomotionPhase=FMath::Frac(LocomotionPhase+Dt*LocomotionSpeed/ReferenceSpeed/CycleSeconds);
 IdleClock=FMath::Fmod(IdleClock+Dt,IdleAnimation->GetPlayLength());
 const int32 RootIndex=Bones.IndexOfByKey(FName(TEXT("Root")));
 FQuat AuthoredRootRotation=FQuat::Identity;
 for(int32 I=0;I<Pose.Num();I++){
  // Each clip may use a different reference-skeleton ordering after retargeting.
  const int32 IdleBone=IdleSkeleton.FindBoneIndex(Bones[I]);
  const int32 WalkBone=WalkSkeleton.FindBoneIndex(Bones[I]);
  const int32 RunBone=RunSkeleton.FindBoneIndex(Bones[I]);
  const FTransform IdlePose=IdleBone>=0?Sample(Idle,IdleBone,IdleClock/IdleAnimation->GetPlayLength()):RestPose[I];
  const FTransform WalkPose=WalkBone>=0?Sample(Walk,WalkBone,LocomotionPhase):RestPose[I];
  const FTransform RunPose=RunBone>=0?Sample(Run,RunBone,LocomotionPhase):RestPose[I];
  FTransform Moving;Moving.Blend(WalkPose,RunPose,RunWeight);
  Pose[I].Blend(IdlePose,Moving,MoveWeight);
  if(BodyAction){
   const int32 ActionBone=BodyAction->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(Bones[I]);
   if(ActionBone>=0){
    const float Duration=BodyAction->GetPlayLength();
    const float Weight=FMath::Min(FMath::Clamp((BodyActionClock-BodyActionStart)/.12f,0.f,1.f),FMath::Clamp((BodyActionEnd-BodyActionClock)/.25f,0.f,1.f));
    FTransform ActionPose=Sample(Clip(BodyAction),ActionBone,FMath::Min(BodyActionClock/Duration,.99999f));
    if(BodyActionFromPose.IsValidIndex(I)&&BodyActionClock-BodyActionStart<.3f){
     FTransform Blended;Blended.Blend(BodyActionFromPose[I],ActionPose,FMath::Clamp((BodyActionClock-BodyActionStart)/.3f,0.f,1.f));Pose[I]=Blended;
    }else{FTransform Blended;Blended.Blend(Pose[I],ActionPose,Weight);Pose[I]=Blended;}
   }
  }
  // The animation-only FBX uses different bind translations. Retarget translation
  // and scale to this mesh's skeleton while preserving the authored rotations.
  // Native zombie glTF clips animate root-parented feet in translation; resetting
  // those tracks pins the shoes at rest while the shins continue moving.
  if(!bNativeCrowdRig&&!WalkAnimation->GetPathName().StartsWith(TEXT("/Game/BattleForTheA/Zombies/")))Pose[I].SetTranslation(RestPose[I].GetTranslation());
  Pose[I].SetScale3D(RestPose[I].GetScale3D());
  if(I==RootIndex)AuthoredRootRotation=Pose[I].GetRotation();
  // CharacterMovement owns world travel; retain hip/knee/ankle animation above it.
  if(Parents[I]<0||Bones[I]==TEXT("Root"))Pose[I]=RestPose[I];
 }
 // The legacy animation FBX parents its IK feet directly to Root, whereas the
 // character FBX parents each shoe to its lower leg. Imported tracks keep the
 // source local frame even when assigned to Casual_Skeleton. Convert the foot
 // rotation through its actual source parent before applying it to the mesh.
 // Native glTF character clips already match their hierarchy and skip this path.
 if(RootIndex>=0&&WalkAnimation->GetPathName().StartsWith(TEXT("/Game/PiedmontRide/Rider/Animations/"))){
  TArray<FQuat> ComponentRotations;ComponentRotations.SetNum(Pose.Num());
  for(int32 I=0;I<Pose.Num();I++)ComponentRotations[I]=Parents[I]>=0?ComponentRotations[Parents[I]]*Pose[I].GetRotation():Pose[I].GetRotation();
  const FQuat SourceRoot=(Parents[RootIndex]>=0?ComponentRotations[Parents[RootIndex]]:FQuat::Identity)*AuthoredRootRotation;
  for(const TCHAR* Name:{TEXT("Foot_L"),TEXT("Foot_R")}){
   const int32 I=Bones.IndexOfByKey(FName(Name));if(I<0||Parents[I]<0)continue;
   Pose[I].SetRotation((ComponentRotations[Parents[I]].Inverse()*SourceRoot*Pose[I].GetRotation()).GetNormalized());
  }
 }
 return true;
}

void APiedmontExplorer::SetLocomotionClips(UAnimSequence* Idle,UAnimSequence* Walk,UAnimSequence* Run){IdleAnimation=Idle;WalkAnimation=Walk;RunAnimation=Run;}

void APiedmontExplorer::PlayBodyAction(UAnimSequence* Animation,const TArray<FTransform>& FromPose,float StartTime,float EndTime){
 BodyAction=Animation;BodyActionFromPose=FromPose;
 const float Duration=Animation?Animation->GetPlayLength():0;
 BodyActionStart=BodyActionClock=FMath::Clamp(StartTime,0.f,Duration);
 BodyActionEnd=EndTime<0?Duration:FMath::Clamp(EndTime,BodyActionStart,Duration);
}

void APiedmontExplorer::SetBodySequence(UAnimSequence* Animation,bool bLoop){
 BodySequence=Animation;BodySequenceClock=0;bBodySequenceLoop=bLoop;bBodySequenceHeld=false;
 BodyAction=nullptr;BodyActionFromPose.Reset();
}
void APiedmontExplorer::StopBodySequence(){BodySequence=nullptr;BodySequenceClock=0;bBodySequenceHeld=false;}
void APiedmontExplorer::HoldBodySequenceAt(float Seconds){
 if(BodySequence){BodySequenceClock=FMath::Clamp(Seconds,0.f,BodySequence->GetPlayLength());bBodySequenceHeld=true;}
}
bool APiedmontExplorer::IsBodySequencePlaying() const{
 return BodySequence&&(bBodySequenceLoop||BodySequenceClock<BodySequence->GetPlayLength());
}
