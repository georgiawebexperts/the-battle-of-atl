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
 if(!bParkDancer||!bNativeCrowdRig||bDead||bSwimming||StumbleRemaining>0||PanicRemaining>0||bIncidentPosing)return;
 DanceClock+=Dt;
 const float Phase=DanceClock*(2.8f+.22f*(DanceVariant%3))+DanceVariant*1.17f;
 auto Rotate=[&](const TCHAR* Name,FRotator Delta){const int32 I=Body->GetBoneIndex(Name);if(I<0)return;Body->BoneSpaceTransforms[I].SetRotation((Delta.Quaternion()*Body->BoneSpaceTransforms[I].GetRotation()).GetNormalized());};
 if(const int32 Hip=Body->GetBoneIndex(TEXT("pelvis"));Hip>=0){
  auto& T=Body->BoneSpaceTransforms[Hip];T.AddToTranslation(FVector(0,0,3.5f*FMath::Abs(FMath::Sin(Phase))));
  T.SetRotation((FRotator(0,8.f*FMath::Sin(Phase*.5f),5.f*FMath::Sin(Phase)).Quaternion()*T.GetRotation()).GetNormalized());
 }
 const float Side=FMath::Sin(Phase),Other=FMath::Sin(Phase+PI*.5f);
 Rotate(TEXT("spine_02"),FRotator(4.f*Other,10.f*Side,7.f*Side));
 Rotate(TEXT("spine_05"),FRotator(-3.f*Other,7.f*Side,-5.f*Side));
 if(DanceVariant%3==0){
  Rotate(TEXT("upperarm_l"),FRotator(-42.f-20.f*Side,-18.f,28.f+14.f*Other));
  Rotate(TEXT("upperarm_r"),FRotator(-42.f+20.f*Side,18.f,-28.f-14.f*Other));
  Rotate(TEXT("lowerarm_l"),FRotator(-25.f+12.f*Other,0,0));Rotate(TEXT("lowerarm_r"),FRotator(-25.f-12.f*Other,0,0));
 }else if(DanceVariant%3==1){
  Rotate(TEXT("upperarm_l"),FRotator(-78.f,5.f,22.f+22.f*Side));Rotate(TEXT("upperarm_r"),FRotator(-18.f+24.f*Other,-8.f,-30.f));
  Rotate(TEXT("lowerarm_l"),FRotator(-45.f+18.f*Side,0,0));Rotate(TEXT("lowerarm_r"),FRotator(-55.f-16.f*Side,0,0));
 }else{
  Rotate(TEXT("upperarm_l"),FRotator(-25.f+28.f*Side,-12.f,48.f));Rotate(TEXT("upperarm_r"),FRotator(-25.f-28.f*Side,12.f,-48.f));
  Rotate(TEXT("lowerarm_l"),FRotator(-68.f,0,12.f*Other));Rotate(TEXT("lowerarm_r"),FRotator(-68.f,0,-12.f*Other));
 }
 Rotate(TEXT("thigh_l"),FRotator(9.f*Side,0,5.f*Other));Rotate(TEXT("thigh_r"),FRotator(-9.f*Side,0,-5.f*Other));
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
