#include "PiedmontExplorer.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "UObject/ObjectKey.h"

namespace {
// Share decoded clip samples across the crowd. Avoid decompressing every bone of
// every pedestrian every frame; interpolation still runs at the render rate.
struct FCrowdClip {
 int32 Bones=0,Frames=0;
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
 if(!IdleAnimation||!WalkAnimation||!RunAnimation)return false;
 // Fill the cache before retaining references: a TMap growth may relocate values.
 Clip(IdleAnimation);Clip(WalkAnimation);Clip(RunAnimation);
 const auto& Idle=Clip(IdleAnimation);const auto& Walk=Clip(WalkAnimation);const auto& Run=Clip(RunAnimation);
 const auto& Skeleton=WalkAnimation->GetSkeleton()->GetReferenceSkeleton();
 const float Speed=GetVelocity().Size2D();
 LocomotionSpeed=FMath::Lerp(LocomotionSpeed,Speed,1.f-FMath::Exp(-10.f*Dt));
 const float RunWeight=FMath::Clamp((LocomotionSpeed-180.f)/170.f,0.f,1.f);
 const float MoveWeight=FMath::Clamp(LocomotionSpeed/45.f,0.f,1.f);
 const float ReferenceSpeed=FMath::Lerp(140.f,350.f,RunWeight);
 const float CycleSeconds=FMath::Lerp(WalkAnimation->GetPlayLength(),RunAnimation->GetPlayLength(),RunWeight);
 LocomotionPhase=FMath::Frac(LocomotionPhase+Dt*LocomotionSpeed/ReferenceSpeed/CycleSeconds);
 IdleClock=FMath::Fmod(IdleClock+Dt,IdleAnimation->GetPlayLength());
 for(int32 I=0;I<Pose.Num();I++){
  const int32 Bone=Skeleton.FindBoneIndex(Bones[I]);if(Bone<0)continue;
  FTransform Moving;Moving.Blend(Sample(Walk,Bone,LocomotionPhase),Sample(Run,Bone,LocomotionPhase),RunWeight);
  Pose[I].Blend(Sample(Idle,Bone,IdleClock/IdleAnimation->GetPlayLength()),Moving,MoveWeight);
  // The animation-only FBX uses different bind translations. Retarget translation
  // and scale to this mesh's skeleton while preserving the authored rotations.
  Pose[I].SetTranslation(RestPose[I].GetTranslation());
  Pose[I].SetScale3D(RestPose[I].GetScale3D());
  // CharacterMovement owns world travel; retain hip/knee/ankle animation above it.
  if(Parents[I]<0||Bones[I]==TEXT("Root"))Pose[I]=RestPose[I];
 }
 return true;
}
