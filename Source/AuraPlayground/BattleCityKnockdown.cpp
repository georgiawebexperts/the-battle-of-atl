#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Misc/CommandLine.h"

void APiedmontPedestrian::AnimateBody(float Dt){
 if(KnockdownPhase==1)return;
 Super::AnimateBody(Dt);
 if((!bParkDancer&&!bParkMusician&&!bPicnicChiller)||!bNativeCrowdRig||bDead||bSwimming||StumbleRemaining>0||PanicRemaining>0||bIncidentPosing)return;
 auto Rotate=[&](const TCHAR* Name,FRotator Delta){const int32 I=Body->GetBoneIndex(Name);if(I<0)return;Body->BoneSpaceTransforms[I].SetRotation((Delta.Quaternion()*Body->BoneSpaceTransforms[I].GetRotation()).GetNormalized());};
 auto RotateBranch=[&](const TCHAR* Name,FRotator Delta){const FName Root(Name);const int32 RootIndex=Body->GetBoneIndex(Root);if(RootIndex<0)return;const FQuat Q=Delta.Quaternion();const FVector Origin=Body->BoneSpaceTransforms[RootIndex].GetLocation();for(int32 I=0;I<Body->BoneSpaceTransforms.Num();I++){FName Bone=Body->GetBoneName(I);bool Descendant=false;while(!Bone.IsNone()){if(Bone==Root){Descendant=true;break;}Bone=Body->GetParentBone(Bone);}if(!Descendant)continue;auto& T=Body->BoneSpaceTransforms[I];T.SetLocation(Origin+Q.RotateVector(T.GetLocation()-Origin));T.SetRotation((Q*T.GetRotation()).GetNormalized());}};
 if(bPicnicChiller){
  ChillClock+=Dt;const float Breath=FMath::Sin(ChillClock*1.8f),Gesture=FMath::Sin(ChillClock*.8f+PicnicPose);
  // SEATED on the cloth with the legs out, which is what a picnic is.
  // The three earlier attempts all failed the same way: the hip pitch was applied
  // in *bone* space, and this crowd rig's bone axes are not the actor's axes, so
  // the femur swung out of the sagittal plane - legs through the cloth, knees in
  // the air, and finally a standing pair nobody asked for. The rotations below are
  // built in component space about the actor's own right axis, so "forward" here
  // is forward on screen whatever the rig's rest orientation is. The body is then
  // dropped until the lowest leg sample sits on the cloth - measured, not guessed -
  // so the pair read as sitting on the blanket at any terrain height.
  const USkeletalMesh* PicnicMesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset());
  const FReferenceSkeleton* PicnicRef=PicnicMesh?&PicnicMesh->GetRefSkeleton():nullptr;
  if(!PicnicRef||PicnicRef->FindBoneIndex(TEXT("thigh_l"))<0||PicnicRef->FindBoneIndex(TEXT("foot_l"))<0){
   Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();return;
  }
  const FTransform CompXf=Body->GetComponentTransform();
  const FVector Fwd=GetActorForwardVector(),Right=GetActorRightVector(),Up=GetActorUpVector();
  const FVector CompRight=CompXf.InverseTransformVectorNoScale(Right).GetSafeNormal();
  const FVector CompUp=CompXf.InverseTransformVectorNoScale(Up).GetSafeNormal();
  auto CompPos=[&](const TCHAR* Name){return CompXf.InverseTransformPositionNoScale(Body->GetBoneLocation(Name,EBoneSpaces::WorldSpace));};
  // Accumulated component-space rotation of a bone, from the bone-space transforms.
  auto CompRot=[&](int32 I){TArray<int32,TFixedAllocator<24>> Chain;for(int32 C=I;C>=0;C=PicnicRef->GetParentIndex(C))Chain.Add(C);FQuat R=FQuat::Identity;for(int32 K=Chain.Num()-1;K>=0;--K)R=R*Body->BoneSpaceTransforms[Chain[K]].GetRotation().GetNormalized();return R;};
  // Apply a rotation about the actor's own axis, in component space, about this
  // bone's own origin - so the limb swings where the eye expects it to.
  auto TurnBone=[&](const TCHAR* Name,const FQuat& CompDelta){const int32 I=Body->GetBoneIndex(Name);if(I<0)return;const int32 P=PicnicRef->GetParentIndex(I);const FQuat ParentComp=P>=0?CompRot(P):FQuat::Identity;Body->BoneSpaceTransforms[I].SetRotation((ParentComp.Inverse()*CompDelta*ParentComp*Body->BoneSpaceTransforms[I].GetRotation()).GetNormalized());};
  auto LeanBone=[&](const TCHAR* Name,float Degrees){TurnBone(Name,FQuat(CompRight,FMath::DegreesToRadians(Degrees)));};
  auto YawBone=[&](const TCHAR* Name,float Degrees){TurnBone(Name,FQuat(CompUp,FMath::DegreesToRadians(Degrees)));};
  // Point a bone at an absolute world direction, whatever the clip left behind.
  auto AimBone=[&](const TCHAR* Name,const TCHAR* Tip,const FVector& WorldTarget,float Blend){
   const int32 I=Body->GetBoneIndex(Name);if(I<0)return;
   const FVector D=CompPos(Tip)-CompPos(Name);if(D.SizeSquared()<1.f)return;
   const FVector T=CompXf.InverseTransformVectorNoScale(WorldTarget).GetSafeNormal();
   TurnBone(Name,FQuat::Slerp(FQuat::Identity,FQuat::FindBetweenNormals(D.GetSafeNormal(),T),Blend));
  };
  const float Lean=9.f+2.f*PicnicPose;                                    // degrees of backward lean
  const float LegDrop=4.f+2.f*PicnicPose;                                 // thigh below horizontal
  const float ShinDrop=LegDrop+3.f;                                       // shin below horizontal
  const float Splay=6.f;                                                  // knees apart, not fused
  auto Drop=[](float Deg){return FMath::Tan(FMath::DegreesToRadians(Deg));};
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
  // The crowd rig's rest yaw is not the actor's yaw, so aiming the legs along
  // the actor's forward while the torso kept its rest yaw drew a person whose
  // legs went one way and whose chest faced another. Measure the offset once
  // from the shoulders and turn the pelvis by it, then aim everything else in
  // absolute world directions.
  if(!bPicnicFaceSolved){
   const FVector ShoulderL=Body->GetBoneLocation(TEXT("upperarm_l"),EBoneSpaces::WorldSpace);
   const FVector ShoulderR=Body->GetBoneLocation(TEXT("upperarm_r"),EBoneSpaces::WorldSpace);
   FVector BodyRight=ShoulderR-ShoulderL;BodyRight.Z=0;
   if(BodyRight.Normalize(.1f)){
    const FVector Current=FVector::CrossProduct(BodyRight,Up).GetSafeNormal();
    PicnicFaceYaw=FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(Current,Fwd),Up),FVector::DotProduct(Current,Fwd)));
   }
   bPicnicFaceSolved=true;
  }
  YawBone(TEXT("pelvis"),PicnicFaceYaw);
  LeanBone(TEXT("pelvis"),-Lean);
  AimBone(TEXT("thigh_l"),TEXT("calf_l"),(Fwd-Up*Drop(LegDrop)+Right*(Splay/90.f)).GetSafeNormal(),1.f);
  AimBone(TEXT("thigh_r"),TEXT("calf_r"),(Fwd-Up*Drop(LegDrop)-Right*(Splay/90.f)).GetSafeNormal(),1.f);
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
  AimBone(TEXT("calf_l"),TEXT("foot_l"),(Fwd-Up*Drop(ShinDrop)+Right*.05f).GetSafeNormal(),1.f);
  AimBone(TEXT("calf_r"),TEXT("foot_r"),(Fwd-Up*Drop(ShinDrop)-Right*.05f).GetSafeNormal(),1.f);
  // Arms down and a little back, hands on the cloth beside the hips, elbows soft.
  AimBone(TEXT("upperarm_l"),TEXT("lowerarm_l"),(-Fwd*.26f-Up*.95f+Right*.20f).GetSafeNormal(),1.f);
  AimBone(TEXT("upperarm_r"),TEXT("lowerarm_r"),(-Fwd*.26f-Up*.95f-Right*.20f).GetSafeNormal(),1.f);
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
  AimBone(TEXT("lowerarm_l"),TEXT("hand_l"),(-Fwd*.70f-Up*.68f+Right*.22f).GetSafeNormal(),1.f);
  AimBone(TEXT("lowerarm_r"),TEXT("hand_r"),(-Fwd*.70f-Up*.68f-Right*.22f).GetSafeNormal(),1.f);
  // Breathing in the spine, and one hand gesturing: this is also what the picnic
  // audit measures when it requires the hands to move.
  LeanBone(TEXT("spine_01"),1.6f+1.2f*Breath);
  LeanBone(TEXT("spine_02"),1.2f+1.f*Breath);
  LeanBone(TEXT("upperarm_r"),-5.f-4.f*Gesture);
  LeanBone(TEXT("upperarm_l"),-3.f+2.f*Gesture);
  const float HalfHeight=GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  if(!bPicnicPoseSolved){
   Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
   float Low=TNumericLimits<float>::Max();
   const TCHAR* LowBone=TEXT("none");
   for(const TCHAR* Name:{TEXT("pelvis"),TEXT("thigh_l"),TEXT("thigh_r"),TEXT("calf_l"),TEXT("calf_r"),TEXT("foot_l"),TEXT("foot_r")}){
    if(Body->GetBoneIndex(Name)<0)continue;
    const FString Bone(Name);
    const float Radius=Bone.Contains(TEXT("pelvis"))?9.f:(Bone.Contains(TEXT("thigh"))?6.f:(Bone.Contains(TEXT("calf"))?5.f:3.f));
    const float Sample=float(Body->GetBoneLocation(Name,EBoneSpaces::WorldSpace).Z)-Radius;
    if(Sample<Low){Low=Sample;LowBone=Name;}
   }
   const float Cloth=bPicnicClothSet?PicnicClothZ:GetActorLocation().Z-HalfHeight;
   PicnicBodyZ=-HalfHeight+(Cloth-Low);bPicnicPoseSolved=true;
   UE_LOG(LogTemp,Display,TEXT("BattlePicnicSeat: actor=%s pose=%d cloth=%.1f low=%.1f low_bone=%s body_z=%.1f half=%.1f actor_z=%.1f"),
    *GetName(),PicnicPose,Cloth,Low,LowBone,PicnicBodyZ,HalfHeight,GetActorLocation().Z);
  }
  Body->SetRelativeLocation(FVector(0,0,PicnicBodyZ));
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();return;
 }
 if(bParkMusician){
  MusicClock+=Dt;const float Beat=FMath::Sin(MusicClock*(MusicianKind==0?10.f:6.f));
  // The idle clip animates both arms, so multiplying a hold on top of it made
  // the arm roots whip like fast logs and left the hands nowhere near the
  // instrument. Replace the clip's arm rotation with the reference pose plus
  // one authored hold, faded in over the first second of playing.
  const float Hold=FMath::Clamp((MusicClock-.2f)/.7f,0.f,1.f);
  const USkeletalMesh* HoldingMesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset());
  const FReferenceSkeleton* HoldRef=HoldingMesh?&HoldingMesh->GetRefSkeleton():nullptr;
  auto HoldBone=[&](const TCHAR* Name,FRotator Pose){
   const int32 I=Body->GetBoneIndex(Name);if(I<0||!HoldRef||I>=HoldRef->GetNum())return;
   const FQuat Target=(Pose.Quaternion()*HoldRef->GetRefBonePose()[I].GetRotation()).GetNormalized();
   const FQuat Current=Body->BoneSpaceTransforms[I].GetRotation().GetNormalized();
   Body->BoneSpaceTransforms[I].SetRotation(Hold>=1.f?Target:FQuat::Slerp(Current,Target,Hold));
  };
  if(const int32 Hip=Body->GetBoneIndex(TEXT("pelvis"));Hip>=0)Body->BoneSpaceTransforms[Hip].AddToTranslation(FVector(0,0,1.5f*FMath::Abs(Beat)));
  Rotate(TEXT("spine_02"),FRotator(4.f,0,(MusicianKind==0?-5.f:2.f*Beat)*Hold));
  if(MusicianKind==0){
   // Guitarist: left hand frets up the neck, right hand strums the sound hole.
   // The strum is a few degrees of wrist and forearm, not a full arm swing.
   HoldBone(TEXT("upperarm_l"),GuitarArmL);HoldBone(TEXT("lowerarm_l"),FRotator(GuitarForeL.Pitch+3.f*Beat,GuitarForeL.Yaw,GuitarForeL.Roll));
   HoldBone(TEXT("upperarm_r"),GuitarArmR);HoldBone(TEXT("lowerarm_r"),FRotator(GuitarForeR.Pitch+7.f*Beat,GuitarForeR.Yaw,GuitarForeR.Roll));
  }else{
   // Saxophonist: both hands on the body, upper hand above the lower one.
   HoldBone(TEXT("upperarm_l"),SaxArmL);HoldBone(TEXT("lowerarm_l"),FRotator(SaxForeL.Pitch+3.f*Beat,SaxForeL.Yaw,SaxForeL.Roll));
   HoldBone(TEXT("upperarm_r"),SaxArmR);HoldBone(TEXT("lowerarm_r"),FRotator(SaxForeR.Pitch+3.f*Beat,SaxForeR.Yaw,SaxForeR.Roll));
  }
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();return;
 }
 DanceClock+=Dt;
 // Six dances, not three, and no two dancers share a beat: a stable per-actor
 // seed gives every one of them a different offset and a slightly different
 // tempo, so a ring of the same variant stops reading as one animation copied
 // around the circle. Elliott: "the people are all dancing alike its a mess".
 const int32 Variant=((DanceVariant%6)+6)%6;
 const float Seed=float(GetUniqueID()%997)/997.f;
 const float Tempo=(Variant==3?3.6f:Variant==5?3.3f:2.6f+.22f*(Variant%3))*(.9f+.2f*Seed);
 const float Phase=DanceClock*Tempo+Seed*6.2832f;
 // Sharp on the beat for the popping dance, smooth for the rest.
 const float Snap=FMath::Sin(Phase)>0.f?1.f:-1.f;
 if(const int32 Hip=Body->GetBoneIndex(TEXT("pelvis"));Hip>=0){
  auto& T=Body->BoneSpaceTransforms[Hip];
  const float Bounce=Variant==3?2.6f*FMath::Abs(Snap):(Variant==4?1.4f:3.5f)*FMath::Abs(FMath::Sin(Phase));
  T.AddToTranslation(FVector(0,0,Bounce));
  const float Twist=Variant==5?.5f*FMath::Sin(Phase):(Variant==3?4.f*Snap:8.f*FMath::Sin(Phase*.5f));
  T.SetRotation((FRotator(0,Twist,(Variant==4?2.f:5.f)*FMath::Sin(Phase)).Quaternion()*T.GetRotation()).GetNormalized());
 }
 const float Side=FMath::Sin(Phase),Other=FMath::Sin(Phase+PI*.5f);
 Rotate(TEXT("spine_02"),FRotator((Variant==4?9.f:4.f)*Other,10.f*Side,7.f*Side));
 Rotate(TEXT("spine_05"),FRotator((Variant==4?6.f:-3.f)*Other,7.f*Side,-5.f*Side));
 auto Arms=[&](float L,float R){Rotate(TEXT("upperarm_l"),FRotator(L,0,0));Rotate(TEXT("upperarm_r"),FRotator(R,0,0));};
 switch(Variant){
  case 0: // Two-step: arms low and swinging, shoulders leading the hips.
   Rotate(TEXT("upperarm_l"),FRotator(-42.f-20.f*Side,-18.f,28.f+14.f*Other));
   Rotate(TEXT("upperarm_r"),FRotator(-42.f+20.f*Side,18.f,-28.f-14.f*Other));
   Rotate(TEXT("lowerarm_l"),FRotator(-25.f+12.f*Other,0,0));Rotate(TEXT("lowerarm_r"),FRotator(-25.f-12.f*Other,0,0));
   break;
  case 1: // One arm up, waving on the off beat.
   Rotate(TEXT("upperarm_l"),FRotator(-78.f,5.f,22.f+22.f*Side));Rotate(TEXT("upperarm_r"),FRotator(-18.f+24.f*Other,-8.f,-30.f));
   Rotate(TEXT("lowerarm_l"),FRotator(-45.f+18.f*Side,0,0));Rotate(TEXT("lowerarm_r"),FRotator(-55.f-16.f*Side,0,0));
   break;
  case 2: // Elbows out at shoulder height, both forearms pumping.
   Rotate(TEXT("upperarm_l"),FRotator(-25.f+28.f*Side,-12.f,48.f));Rotate(TEXT("upperarm_r"),FRotator(-25.f-28.f*Side,12.f,-48.f));
   Rotate(TEXT("lowerarm_l"),FRotator(-68.f,0,12.f*Other));Rotate(TEXT("lowerarm_r"),FRotator(-68.f,0,-12.f*Other));
   break;
  case 3: // Robot: both arms at right angles, snapping between two positions.
   Rotate(TEXT("upperarm_l"),FRotator(-55.f-25.f*Snap,-10.f,34.f));Rotate(TEXT("upperarm_r"),FRotator(-55.f+25.f*Snap,10.f,-34.f));
   Rotate(TEXT("lowerarm_l"),FRotator(-70.f-28.f*Snap,0,0));Rotate(TEXT("lowerarm_r"),FRotator(-70.f+28.f*Snap,0,0));
   Rotate(TEXT("head"),FRotator(0,12.f*Snap,0));
   break;
  case 4: // Headbang: deep torso dip, arms hanging and swinging through.
   Arms(-38.f+26.f*Side,-38.f-26.f*Side);
   Rotate(TEXT("head"),FRotator(14.f+10.f*Other,0,0));
   Rotate(TEXT("lowerarm_l"),FRotator(-30.f-18.f*Side,0,0));Rotate(TEXT("lowerarm_r"),FRotator(-30.f+18.f*Side,0,0));
   break;
  default: // DJ pump: both fists up on the beat, knees alternating.
   Rotate(TEXT("upperarm_l"),FRotator(-96.f+22.f*Side,-14.f,26.f));Rotate(TEXT("upperarm_r"),FRotator(-96.f-22.f*Side,14.f,-26.f));
   Rotate(TEXT("lowerarm_l"),FRotator(-30.f+16.f*Other,0,0));Rotate(TEXT("lowerarm_r"),FRotator(-30.f-16.f*Other,0,0));
   Rotate(TEXT("thigh_l"),FRotator(-28.f*FMath::Max(0.f,Side),0,6.f));Rotate(TEXT("thigh_r"),FRotator(-28.f*FMath::Max(0.f,-Side),0,-6.f));
   Rotate(TEXT("calf_l"),FRotator(-42.f*FMath::Max(0.f,Side),0,0));Rotate(TEXT("calf_r"),FRotator(-42.f*FMath::Max(0.f,-Side),0,0));
   break;
 }
 const float Step=Variant==3?7.f*Snap:9.f*Side;
 Rotate(TEXT("thigh_l"),FRotator(Step,0,5.f*Other));Rotate(TEXT("thigh_r"),FRotator(-Step,0,-5.f*Other));
 Rotate(TEXT("calf_l"),FRotator(-10.f*FMath::Max(0.f,Side),0,0));Rotate(TEXT("calf_r"),FRotator(10.f*FMath::Min(0.f,Side),0,0));
 Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
}

void APiedmontPedestrian::AttachCityParts(USkinnedMeshComponent* Leader){
 TArray<USkeletalMeshComponent*> Parts;GetComponents(Parts);
 for(auto* Part:Parts)if(Part->GetName().StartsWith(TEXT("CityOutfit"))){
  Part->AttachToComponent(Leader,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  Part->SetLeaderPoseComponent(Leader,true,false);
 }
 TArray<UStaticMeshComponent*> Props;GetComponents(Props);
 for(auto* Part:Props)if(Part->GetName()==TEXT("CityHair"))
  Part->AttachToComponent(Leader,FAttachmentTransformRules::KeepRelativeTransform,TEXT("head"));
}

bool APiedmontPedestrian::BeginKnockdown(float Speed,FVector Direction){
 if(!bNativeCrowdRig||KnockdownPhase||RecoveryClips.Num()!=4)return false;
 for(const auto& Clip:RecoveryClips)if(!Clip)return false;
 auto* Mesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset());if(!Mesh||!Mesh->GetPhysicsAsset())return false;
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleCityKnockdownReview"))){
  auto* Asset=Mesh->GetPhysicsAsset();
  UE_LOG(LogTemp,Display,TEXT("CityPhysicsAsset: asset=%s bodies=%d constraints=%d"),*Asset->GetName(),Asset->SkeletalBodySetups.Num(),Asset->ConstraintSetup.Num());
  for(const auto& Setup:Asset->SkeletalBodySetups){
   UE_LOG(LogTemp,Display,TEXT("CityPhysicsShape: bone=%s capsules=%d spheres=%d boxes=%d"),*Setup->BoneName.ToString(),Setup->AggGeom.SphylElems.Num(),Setup->AggGeom.SphereElems.Num(),Setup->AggGeom.BoxElems.Num());
   for(const auto& Shape:Setup->AggGeom.SphylElems)UE_LOG(LogTemp,Display,TEXT("CityPhysicsCapsule: bone=%s radius=%.3f length=%.3f center=%s"),*Setup->BoneName.ToString(),Shape.Radius,Shape.Length,*Shape.Center.ToString());
  }
 }
#endif
 PlayBodyAction(nullptr);bPlayingBumpReaction=false;
 StandingPelvis=Body->GetSocketQuaternion(TEXT("pelvis"));StandingForward=GetActorForwardVector();StandingRight=GetActorRightVector();
 PhysicsBody=NewObject<USkeletalMeshComponent>(this);AddInstanceComponent(PhysicsBody);
 PhysicsBody->SetSkeletalMeshAsset(Mesh);PhysicsBody->SetDisablePostProcessBlueprint(true);
 // The source collision hull fits the underlying body closely. Add garment/face
 // clearance to this ragdoll instance, without changing shared downloaded assets.
 auto* CollisionAsset=DuplicateObject<UPhysicsAsset>(Mesh->GetPhysicsAsset(),this,MakeUniqueObjectName(this,UPhysicsAsset::StaticClass(),TEXT("CityRagdollPhysics")));
 for(const auto& Setup:CollisionAsset->SkeletalBodySetups){
  const float Padding=Setup->BoneName==TEXT("head")?2.f:((Setup->BoneName==TEXT("pelvis")||Setup->BoneName==TEXT("spine_02")||Setup->BoneName==TEXT("spine_05"))?1.5f:0.f);
  for(auto& Shape:Setup->AggGeom.SphylElems)Shape.Radius+=Padding;
 }
 PhysicsBody->SetPhysicsAsset(CollisionAsset,true);
 PhysicsBody->SetWorldTransform(Body->GetComponentTransform());
 PhysicsBody->SetCollisionProfileName(TEXT("Ragdoll"));
 PhysicsBody->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
 PhysicsBody->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 PhysicsBody->SetCanEverAffectNavigation(false);
 PhysicsBody->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
 PhysicsBody->RegisterComponent();PhysicsBody->RefreshBoneTransforms();
 GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->DisableMovement();
 GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 PhysicsBody->SetAllBodiesSimulatePhysics(true);PhysicsBody->SetSimulatePhysics(true);
 // Initialize each physical bone at the current animated pose instead of snapping
 // into a reference T-pose when switching to physics.
 const auto& Ref=Mesh->GetRefSkeleton();
 for(int32 I=0;I<Ref.GetNum();++I)if(auto* Instance=PhysicsBody->GetBodyInstance(Ref.GetBoneName(I)))
  Instance->SetBodyTransform(Body->GetBoneTransform(I),ETeleportType::TeleportPhysics);
 PhysicsBody->SetAllPhysicsLinearVelocity(Direction.GetSafeNormal2D()*FMath::Clamp(Speed*.45f,180.f,500.f)+FVector(0,0,80));
 PhysicsBody->AddImpulse(Direction.GetSafeNormal2D()*100.f,TEXT("spine_05"),true);
 AttachCityParts(PhysicsBody);Body->SetVisibility(false,false);
 KnockdownPhase=1;KnockdownClock=0;RecoveryRetry=0;StumbleRemaining=10;
 UE_LOG(LogTemp,Display,TEXT("CityKnockdown: started actor=%s speed=%.1f"),*GetName(),Speed);
 return true;
}

bool APiedmontPedestrian::BeginRecovery(){
 if(!PhysicsBody)return false;
 const FVector Pelvis=PhysicsBody->GetSocketLocation(TEXT("pelvis"));
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleCityKnockdownReview"))){
  for(const FName Bone:{FName(TEXT("pelvis")),FName(TEXT("spine_02")),FName(TEXT("spine_05")),FName(TEXT("head"))}){
   const FVector Point=PhysicsBody->GetSocketLocation(Bone);
   for(bool Complex:{false,true}){
    FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(CityPhysicsFloor),Complex,this);
    if(GetWorld()->LineTraceSingleByChannel(Hit,Point+FVector(0,0,150),Point-FVector(0,0,250),ECC_Visibility,Q)){
     auto* Instance=PhysicsBody->GetBodyInstance(Bone);float ShapeBottom=99999;
     if(Instance)if(auto* Setup=Cast<UBodySetup>(Instance->BodySetup.Get()))ShapeBottom=Setup->AggGeom.CalcAABB(Instance->GetUnrealWorldTransform()).Min.Z;
     UE_LOG(LogTemp,Display,TEXT("CityPhysicsFloor: bone=%s complex=%d bone_z=%.3f shape_bottom=%.3f floor_z=%.3f actor=%s component=%s collision=%d"),*Bone.ToString(),Complex,Point.Z,ShapeBottom,Hit.ImpactPoint.Z,*GetNameSafe(Hit.GetActor()),*GetNameSafe(Hit.GetComponent()),Hit.GetComponent()?int32(Hit.GetComponent()->GetCollisionEnabled()):-1);
    }
   }
  }
 }
#endif
 const FQuat RotationDelta=PhysicsBody->GetSocketQuaternion(TEXT("pelvis"))*StandingPelvis.Inverse();
 const float Facing=RotationDelta.RotateVector(StandingForward).Z;
 const float Side=RotationDelta.RotateVector(StandingRight).Z;
 const int32 Choice=FMath::Abs(Facing)>.5f?(Facing<0?0:1):(Side>0?2:3);
 UAnimSequence* Animation=RecoveryClips[Choice];
 const auto& Ref=Cast<USkeletalMesh>(Body->GetSkinnedAsset())->GetRefSkeleton();
 const auto& Source=Animation->GetSkeleton()->GetReferenceSkeleton();
 TArray<FTransform> Start;Start.SetNum(Ref.GetNum());
 for(int32 I=0;I<Start.Num();++I){
  Start[I]=Ref.GetRefBonePose()[I];const int32 Bone=Source.FindBoneIndex(Ref.GetBoneName(I));
  if(Bone>=0)Animation->GetBoneTransform(Start[I],FSkeletonPoseBoneIndex(Bone),FAnimExtractContext(0.0),false);
  const int32 Parent=Ref.GetParentIndex(I);if(Parent>=0)Start[I]=Start[I]*Start[Parent];else Start[I]=FTransform::Identity;
 }
 const int32 HipIndex=Ref.FindBoneIndex(TEXT("pelvis")),HeadIndex=Ref.FindBoneIndex(TEXT("head"));
 if(HipIndex<0||HeadIndex<0)return false;
 const FVector DesiredHeading=(PhysicsBody->GetSocketLocation(TEXT("head"))-Pelvis).GetSafeNormal2D();
 const FVector ClipHeading=FRotator(0,-90,0).RotateVector(Start[HeadIndex].GetLocation()-Start[HipIndex].GetLocation()).GetSafeNormal2D();
 const float Yaw=DesiredHeading.Rotation().Yaw-ClipHeading.Rotation().Yaw;
 const FRotator MeshRotation(0,Yaw-90,0);
 FVector Origin=Pelvis-MeshRotation.RotateVector(Start[HipIndex].GetLocation());
 FHitResult Floor;FCollisionQueryParams Query(SCENE_QUERY_STAT(CityRecovery),false,this);
 if(!GetWorld()->LineTraceSingleByChannel(Floor,Origin+FVector(0,0,250),Origin-FVector(0,0,600),ECC_WorldStatic,Query)||Floor.ImpactNormal.Z<.65f)return false;
 const float Half=GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
 const FVector StandingLocation(Floor.ImpactPoint.X,Floor.ImpactPoint.Y,Floor.ImpactPoint.Z+Half+2);
 if(GetWorld()->OverlapBlockingTestByChannel(StandingLocation,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(),Half),Query))return false;
 // Express the settled physical pose in the recovery mesh frame for a short blend.
 const FTransform NewBody(MeshRotation,FVector(StandingLocation.X,StandingLocation.Y,Floor.ImpactPoint.Z));
 TArray<FTransform> Components,Local;Components.SetNum(Ref.GetNum());Local.SetNum(Ref.GetNum());
 for(int32 I=0;I<Ref.GetNum();++I)Components[I]=Ref.GetParentIndex(I)<0?FTransform::Identity:PhysicsBody->GetBoneTransform(I).GetRelativeTransform(NewBody);
 for(int32 I=0;I<Ref.GetNum();++I){const int32 Parent=Ref.GetParentIndex(I);Local[I]=Parent>=0?Components[I].GetRelativeTransform(Components[Parent]):Ref.GetRefBonePose()[I];}
 SetActorLocationAndRotation(StandingLocation,FRotator(0,Yaw,0),false,nullptr,ETeleportType::TeleportPhysics);
 Body->SetWorldTransform(NewBody);Body->BoneSpaceTransforms=Local;Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
 PhysicsBody->SetSimulatePhysics(false);AttachCityParts(Body);PhysicsBody->DestroyComponent();PhysicsBody=nullptr;
 Body->SetVisibility(true,false);PlayBodyAction(Animation,Local);
 GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 GetCharacterMovement()->SetMovementMode(MOVE_Walking);
 KnockdownPhase=2;StumbleRemaining=Animation->GetPlayLength();
 const TCHAR* Names[]={TEXT("F"),TEXT("B"),TEXT("L"),TEXT("R")};RecoveryDirection=Names[Choice];
 UE_LOG(LogTemp,Display,TEXT("CityKnockdown: recovery actor=%s direction=%s"),*GetName(),*RecoveryDirection);
 return true;
}

void APiedmontPedestrian::TickKnockdown(float Dt){
 if(bDead)return;
 KnockdownClock+=Dt;
 if(KnockdownPhase==1){
  RecoveryRetry-=Dt;
  if(KnockdownClock>1.5f&&RecoveryRetry<=0&&PhysicsBody){
   RecoveryRetry=.5f;
   if(PhysicsBody->GetPhysicsLinearVelocity(TEXT("pelvis")).Size()<70||KnockdownClock>4)BeginRecovery();
  }
 }else if(KnockdownPhase==2){
  GetCharacterMovement()->StopMovementImmediately();StumbleRemaining=FMath::Max(0.f,StumbleRemaining-Dt);
  if(StumbleRemaining<=0){KnockdownPhase=0;CompletedRecoveries++;ThinkRemaining=.3f;UE_LOG(LogTemp,Display,TEXT("CityKnockdown: complete actor=%s"),*GetName());}
 }
}
