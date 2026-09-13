#include "PiedmontExplorer.h"
#include "BattleDetailedRider.h"
#include "Animation/AnimSequence.h"
#include "PiedmontBike.h"
#include "PiedmontCombat.h"
#include "PiedmontBlood.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
APiedmontExplorer::APiedmontExplorer(){
 PrimaryActorTick.bCanEverTick=true;
 static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleClip(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Idle.Animations_CharacterArmature_Idle"));
 static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkClip(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Walk.Animations_CharacterArmature_Walk"));
 static ConstructorHelpers::FObjectFinder<UAnimSequence> RunClip(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Run.Animations_CharacterArmature_Run"));
 IdleAnimation=IdleClip.Object;WalkAnimation=WalkClip.Object;RunAnimation=RunClip.Object;
 GetCapsuleComponent()->InitCapsuleSize(30,88);GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 GetCharacterMovement()->MaxWalkSpeed=380;
 GetCharacterMovement()->MaxStepHeight=35;
 GetCharacterMovement()->BrakingDecelerationWalking=1600;
 bUseControllerRotationYaw=true;
 GetMesh()->SetVisibility(false);GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Body=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("WalkingRider"));Body->SetupAttachment(GetCapsuleComponent());
 Body->SetRelativeLocation(FVector(0,0,-88));Body->SetRelativeRotation(FRotator(0,-90,0));Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> Human(TEXT("/Game/PiedmontRide/Rider/Casual.Casual"));Body->SetSkinnedAssetAndUpdate(Human.Object);
 CameraArm=CreateDefaultSubobject<USpringArmComponent>(TEXT("ExplorerArm"));CameraArm->SetupAttachment(GetCapsuleComponent());CameraArm->TargetArmLength=320;CameraArm->SocketOffset=FVector(0,55,15);CameraArm->SetRelativeLocation(FVector(0,0,65));CameraArm->bUsePawnControlRotation=true;
 Weapon=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DevelopmentSidearm"));Weapon->SetupAttachment(GetCapsuleComponent());
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));Weapon->SetStaticMesh(Cube.Object);Weapon->SetRelativeLocation(FVector(48,15,42));Weapon->SetRelativeScale3D(FVector(.35,.05,.08));Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);Weapon->SetVisibility(false);
 Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("ExplorerCamera"));Camera->SetupAttachment(CameraArm);Camera->FieldOfView=85;
}
void APiedmontExplorer::BeginPlay(){
 Super::BeginPlay();
 if(auto* Gun=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/PiedmontRide/Bike/SM_Pistol.SM_Pistol"),nullptr,LOAD_NoWarn)){
  const FVector Size=Gun->GetBounds().BoxExtent*2;const float Scale=28.f/FMath::Max(Size.X,Size.Y);const FRotator Rotation(0,Size.Y>Size.X?-90.f:0.f,0);
  Weapon->SetStaticMesh(Gun);Weapon->SetRelativeScale3D(FVector(Scale));Weapon->SetRelativeRotation(Rotation);Weapon->SetRelativeLocation(FVector(48,15,42)-Rotation.RotateVector(Gun->GetBounds().Origin)*Scale);
 }
 for(TActorIterator<AActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("RideWater")))GetCapsuleComponent()->IgnoreActorWhenMoving(*It,true);
 if(auto* Asset=Cast<USkeletalMesh>(Body->GetSkinnedAsset())){
  const auto& Ref=Asset->GetRefSkeleton();
  for(int32 I=0;I<Ref.GetNum();++I){Parents.Add(Ref.GetParentIndex(I));Bones.Add(Ref.GetBoneName(I));RestPose.Add(Ref.GetRefBonePose()[I]);}
 }
}
void APiedmontExplorer::SetupPlayerInputComponent(UInputComponent* Input){Super::SetupPlayerInputComponent(Input);Input->BindKey(EKeys::E,IE_Pressed,this,&APiedmontExplorer::Interact);
 Input->BindKey(EKeys::One,IE_Pressed,this,&APiedmontExplorer::ToggleWeapon);
 Input->BindKey(EKeys::LeftMouseButton,IE_Pressed,this,&APiedmontExplorer::PullTrigger);Input->BindKey(EKeys::LeftMouseButton,IE_Released,this,&APiedmontExplorer::ReleaseTrigger);
 Input->BindKey(EKeys::RightMouseButton,IE_Pressed,this,&APiedmontExplorer::AimOn);Input->BindKey(EKeys::RightMouseButton,IE_Released,this,&APiedmontExplorer::AimOff);
 Input->BindKey(EKeys::R,IE_Pressed,this,&APiedmontExplorer::Reload);
}
void APiedmontExplorer::Interact(){Remount();}
bool APiedmontExplorer::Remount(){return !bDead&&!bSwimming&&IsValid(Bike)&&Bike->Remount(this);}
void APiedmontExplorer::Tick(float Dt){
 Super::Tick(Dt);if(bDead){Weapon->SetVisibility(false);return;}UpdateSwimming(Dt);
 FireCooldown=FMath::Max(0.f,FireCooldown-Dt);
 if(!CanUseWeapon()){bWeaponDrawn=false;bAiming=false;bTriggerHeld=false;ReloadRemaining=0;}
 if(ReloadRemaining>0){ReloadRemaining=FMath::Max(0.f,ReloadRemaining-Dt);if(ReloadRemaining<=0)FinishReload();}
 if(bTriggerHeld)Fire();Weapon->SetVisibility(bWeaponDrawn);
 CameraArm->TargetArmLength=FMath::FInterpTo(CameraArm->TargetArmLength,bAiming?180.f:320.f,Dt,10);
 Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView,bAiming?AimedFieldOfView():85.f,Dt,10));
 if(auto* PC=Cast<APlayerController>(GetController())){
  // GetInputMouseDelta already includes the project 0.07 axis sensitivity.
  // Read device delta once so the pawn does not attenuate mouse look twice.
  const float X=PC->PlayerInput?PC->PlayerInput->GetRawKeyValue(EKeys::MouseX):0.f;
  const float Y=PC->PlayerInput?PC->PlayerInput->GetRawKeyValue(EKeys::MouseY):0.f;
  AddControllerYawInput(X*.03f);AddControllerPitchInput(Y*-.02f);
  const float Forward=(PC->IsInputKeyDown(EKeys::W)||PC->IsInputKeyDown(EKeys::Up)?1.f:0.f)-(PC->IsInputKeyDown(EKeys::S)||PC->IsInputKeyDown(EKeys::Down)?1.f:0.f);
  const float Side=(PC->IsInputKeyDown(EKeys::D)||PC->IsInputKeyDown(EKeys::Right)?1.f:0.f)-(PC->IsInputKeyDown(EKeys::A)||PC->IsInputKeyDown(EKeys::Left)?1.f:0.f);
  FRotator Heading(0,PC->GetControlRotation().Yaw,0);FVector Move=Heading.Vector()*Forward+FRotationMatrix(Heading).GetUnitAxis(EAxis::Y)*Side;
  AddMovementInput(Move.GetSafeNormal(),FMath::Min(1.f,Move.Size()));
 }
 AnimateBody(Dt);
}
void APiedmontExplorer::AnimateBody(float Dt){
 if(RestPose.IsEmpty())return;
 // Native crowd meshes place their origin at the soles. Align that origin to
 // CharacterMovement's actual floor; the legacy mesh uses its own pose correction.
 float BodyHeight=-88.f;
 if(bNativeCrowdRig){
  BodyHeight=-GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  const auto* Movement=GetCharacterMovement();
  if(Movement->IsMovingOnGround()&&Movement->CurrentFloor.IsWalkableFloor())
   // A sweep can report an edge/contact point above the supporting plane.
   // Floor distance is measured from the capsule base and remains valid there.
   BodyHeight-=Movement->CurrentFloor.GetDistanceToFloor();
 }
 SwimPoseBlend=FMath::FInterpTo(SwimPoseBlend,bSwimming?FMath::Clamp(GetVelocity().Size2D()/160.f,0.f,1.f):0.f,Dt,4.f);
 if(bSwimming&&bDetailedPlayerRig){
  const FVector Treading(0,0,-150+1.5f*FMath::Sin(Gait));
  Body->SetRelativeLocation(FMath::Lerp(Treading,FVector(-65,0,-75+SwimStandingHalfHeight-35),SwimPoseBlend));
  Body->SetRelativeRotation(FRotator(0,-90,90*SwimPoseBlend));
 }else{
  Body->SetRelativeLocation(bSwimming?FVector(-65,0,-75+SwimStandingHalfHeight-35):FVector(0,0,BodyHeight));
  Body->SetRelativeRotation(bSwimming?FRotator(0,-90,90):FRotator(0,-90,0));
 }
 Gait+=bSwimming?Dt*FMath::Lerp(2.3f,4.6f,SwimPoseBlend):GetVelocity().Size2D()*Dt/85.f;
 const float Blend=FMath::Clamp(GetVelocity().Size2D()/220.f,0.f,1.f);
 TArray<FTransform> Pose=RestPose;
 bAuthoredLocomotion=!bSwimming&&SampleLocomotion(Dt,Pose);
 for(int32 I=0;I<Pose.Num();++I)if(Parents[I]>=0)Pose[I]=Pose[I]*Pose[Parents[I]];
 auto Index=[&](const TCHAR* Name){return Bones.IndexOfByKey(BattleDetailedBone(FName(Name),bDetailedPlayerRig));};
 auto Descendant=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Parents[I];}return false;};
 auto MoveBranch=[&](int Root,FVector Target,FQuat Rotation){
  if(Root<0)return;const FVector Old=Pose[Root].GetLocation();
  for(int I=Root;I<Pose.Num();++I)if(Descendant(I,Root)){Pose[I].SetLocation(Target+Rotation.RotateVector(Pose[I].GetLocation()-Old));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}
 };
 auto Limb=[&](const TCHAR* Upper,const TCHAR* Lower,const TCHAR* End,FVector Target,FVector Bend){
  const int U=Index(Upper),L=Index(Lower),E=Index(End);if(U<0||L<0||E<0)return;
  const FVector Origin=Pose[U].GetLocation();const float A=FVector::Distance(Origin,Pose[L].GetLocation()),B=FVector::Distance(Pose[L].GetLocation(),Pose[E].GetLocation());
  const FVector Direction=(Target-Origin).GetSafeNormal();const float D=FMath::Clamp(FVector::Distance(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);Target=Origin+Direction*D;
  const float Along=(A*A-B*B+D*D)/(2*D),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));
  const FVector Joint=Origin+Direction*Along+(Bend-Direction*FVector::DotProduct(Bend,Direction)).GetSafeNormal()*Height;
  MoveBranch(U,Origin,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Joint-Origin));
  MoveBranch(L,Joint,FQuat::FindBetweenVectors(Pose[E].GetLocation()-Pose[L].GetLocation(),Target-Joint));
 };
 const float Stroke=.5f+.5f*FMath::Sin(Gait);
 for(int Side:{1,-1}){
  FVector Hand=bSwimming?FVector(Side*(20+35*Stroke),20+10*FMath::Cos(Gait),175-45*Stroke):FVector(Side*26,FMath::Sin(Gait)*Side*18*Blend,80);
  if(bSwimming&&bDetailedPlayerRig){
   const float Phase=Gait+(Side<0?PI:0.f);
   const FVector Tread(Side*(35+8*FMath::Sin(Gait)),20+12*FMath::Cos(Gait),100+5*FMath::Sin(Gait));
   const FVector Crawl(Side*(24+8*FMath::Sin(Phase)),10+25*FMath::Sin(Phase),145+40*FMath::Cos(Phase));
   Hand=FMath::Lerp(Tread,Crawl,SwimPoseBlend);
  }
  if(bWeaponDrawn)Hand=Side==1?FVector(5,40,128):FVector(-15,45,130);
  const int HandIndex=Index(Side==1?TEXT("Hand_L"):TEXT("Hand_R"));
  if(bAuthoredLocomotion&&!bWeaponDrawn&&HandIndex>=0)Hand=Pose[HandIndex].GetLocation();
  if(!bWeaponDrawn&&!bSwimming)Hand=AdjustVisitorHand(Side,Hand);
  const bool OverrideHand=!bAuthoredLocomotion||bWeaponDrawn||HandIndex<0||!Hand.Equals(Pose[HandIndex].GetLocation(),.01f);
  if(OverrideHand){
  if(Side==1)Limb(TEXT("UpperArm_L"),TEXT("LowerArm_L"),TEXT("Hand_L"),Hand,FVector(1,1,0));
  else Limb(TEXT("UpperArm_R"),TEXT("LowerArm_R"),TEXT("Hand_R"),Hand,FVector(-1,1,0));
  }
  if(bSwimming&&bDetailedPlayerRig&&HandIndex>=0){
   const TCHAR* Suffix=Side==1?TEXT("l"):TEXT("r");
   auto Finger=[&](const TCHAR* Name){return Bones.IndexOfByKey(FName(*FString::Printf(TEXT("%s_01_%s"),Name,Suffix)));};
   const int Middle=Finger(TEXT("middle")),First=Finger(TEXT("index")),Last=Finger(TEXT("pinky"));
   if(Middle>=0&&First>=0&&Last>=0){
    const FVector Direction=FMath::Lerp(FVector(0,1,0),FVector(0,0,1),SwimPoseBlend).GetSafeNormal();
    MoveBranch(HandIndex,Pose[HandIndex].GetLocation(),FQuat::FindBetweenVectors(Pose[Middle].GetLocation()-Pose[HandIndex].GetLocation(),Direction));
    const FVector Across=FVector::VectorPlaneProject(Pose[Last].GetLocation()-Pose[First].GetLocation(),Direction).GetSafeNormal(),Target(Side,0,0);
    MoveBranch(HandIndex,Pose[HandIndex].GetLocation(),FQuat(Direction,FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(Across,Target),Direction),FVector::DotProduct(Across,Target))));
   }
  }
  const int Leg=Index(Side==1?TEXT("UpperLeg_L"):TEXT("UpperLeg_R"));
  if(Leg>=0&&!bAuthoredLocomotion)MoveBranch(Leg,Pose[Leg].GetLocation(),FQuat(FVector::ForwardVector,FMath::DegreesToRadians(Side*FMath::Sin(Gait*(bSwimming&&bDetailedPlayerRig?2.f:1.f))*(bSwimming?FMath::Lerp(5.f,12.f,SwimPoseBlend):28.f*Blend))));
 }
 // Keep the support foot down after translation retargeting; fade this correction
 // out for running, where both feet may legitimately be airborne in the stride.
 float HeightTarget=0;
 if(bAuthoredLocomotion&&!bNativeCrowdRig&&GetCharacterMovement()->IsMovingOnGround()){
  float FootHeight=BIG_NUMBER;
  for(const TCHAR* Name:{TEXT("Foot_L"),TEXT("Foot_R"),TEXT("Foot_L_end"),TEXT("Foot_R_end")}){const int32 I=Index(Name);if(I>=0)FootHeight=FMath::Min(FootHeight,float(Pose[I].GetLocation().Z));}
  if(FootHeight<BIG_NUMBER)HeightTarget=FMath::Clamp(2.275f-FootHeight,-35.f,15.f)*(1.f-FMath::Clamp((LocomotionSpeed-180.f)/170.f,0.f,1.f));
 }
 GroundPoseOffset=FMath::Lerp(GroundPoseOffset,HeightTarget,1.f-FMath::Exp(-20.f*Dt));
 if(!bSwimming)Body->AddLocalOffset(FVector(0,0,GroundPoseOffset));
 for(int I=0;I<Pose.Num();++I)Body->BoneSpaceTransforms[I]=Parents[I]>=0?Pose[I].GetRelativeTransform(Pose[Parents[I]]):Pose[I];
 Body->MarkRefreshTransformDirty();
}
void APiedmontExplorer::ValidationKey(FName Key,bool Pressed){
#if WITH_EDITOR
 if(auto* PC=Cast<APlayerController>(GetController()))PC->InputKey(FInputKeyParams(FKey(Key),Pressed?IE_Pressed:IE_Released,Pressed?1.0:0.0));
#endif
}

void APiedmontExplorer::BeginSurfaceSwimming(float SurfaceZ){
 if(!bSwimming){UnCrouch();GetCharacterMovement()->UnCrouch(false);SwimStandingHalfHeight=GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();}
 GetCapsuleComponent()->SetCapsuleHalfHeight(35);
 SetActorLocation(FVector(GetActorLocation().X,GetActorLocation().Y,SurfaceZ+35),false,nullptr,ETeleportType::TeleportPhysics);
 bSwimming=true;GetCharacterMovement()->SetMovementMode(MOVE_Flying);GetCharacterMovement()->MaxFlySpeed=200;GetCharacterMovement()->BrakingDecelerationFlying=1200;
}
void APiedmontExplorer::UpdateSwimming(float Dt){
 constexpr float SwimHalfHeight=35;
 APiedmontWaterHazard* Water=nullptr;
 const FVector Here=GetActorLocation();
 for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It){
  const float Level=It->GetActorLocation().Z;
  if(It->ContainsBike(FVector(Here.X,Here.Y,Level+1))&&Here.Z-(bSwimming?SwimHalfHeight:GetCapsuleComponent()->GetScaledCapsuleHalfHeight())<Level+25){Water=*It;break;}
 }
 const bool InWater=Water!=nullptr;
 if(InWater!=bSwimming){
  if(InWater){
   BeginSurfaceSwimming(Water->GetActorLocation().Z);
  }else{
   const FVector Standing=Here+FVector(0,0,SwimStandingHalfHeight-SwimHalfHeight);
   FCollisionQueryParams Q(SCENE_QUERY_STAT(SwimStandClearance),false,this);
   for(auto* Ignored:GetCapsuleComponent()->GetMoveIgnoreActors())Q.AddIgnoredActor(Ignored);
   if(GetWorld()->OverlapBlockingTestByChannel(Standing,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(),SwimStandingHalfHeight),Q))return;
   SetActorLocation(Standing,false,nullptr,ETeleportType::TeleportPhysics);
   GetCapsuleComponent()->SetCapsuleHalfHeight(SwimStandingHalfHeight);
  }
  bSwimming=InWater;GetCharacterMovement()->SetMovementMode(bSwimming?MOVE_Flying:MOVE_Walking);
  GetCharacterMovement()->MaxFlySpeed=200;GetCharacterMovement()->BrakingDecelerationFlying=1200;
 }
 if(bSwimming){
  const FVector Position=GetActorLocation();const float Target=Water->GetActorLocation().Z+SwimHalfHeight;
  FHitResult Hit;SetActorLocation(FVector(Position.X,Position.Y,FMath::FInterpTo(Position.Z,Target,Dt,8)),true,&Hit);
  GetCharacterMovement()->Velocity.Z=0;
 }
}

bool APiedmontExplorer::CanUseWeapon() const{return !bDead&&!bSwimming&&GetController()&&GetCharacterMovement()->IsMovingOnGround();}
void APiedmontExplorer::ToggleWeapon(){if(CanUseWeapon()){bWeaponDrawn=!bWeaponDrawn;if(!bWeaponDrawn){bTriggerHeld=false;bAiming=false;ReloadRemaining=0;}}}
void APiedmontExplorer::PullTrigger(){bTriggerHeld=true;Fire();}void APiedmontExplorer::ReleaseTrigger(){bTriggerHeld=false;}
void APiedmontExplorer::AimOn(){bAiming=bWeaponDrawn&&CanUseWeapon();}void APiedmontExplorer::AimOff(){bAiming=false;}
void APiedmontExplorer::Reload(){if(CanUseWeapon()&&bWeaponDrawn&&Ammo<12&&ReloadRemaining<=0)ReloadRemaining=1.5f;}
bool APiedmontExplorer::Fire(){
 if(!CanUseWeapon()||!bWeaponDrawn||FireCooldown>0||ReloadRemaining>0||Ammo<=0)return false;
 Ammo--;ShotsFired++;FireCooldown=.28f;
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/PiedmontRide/Audio/S_Gunshot.S_Gunshot"),nullptr,LOAD_NoWarn))UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation());
 auto* PC=Cast<APlayerController>(GetController());FVector Eye;FRotator Aim;PC->GetPlayerViewPoint(Eye,Aim);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(PiedmontGunshot),true,this);
 const FVector Muzzle=GetActorLocation()+GetActorRotation().RotateVector(FVector(68,15,42));
 FHitResult CameraHit,Hit;const FVector Far=Eye+Aim.Vector()*15000;
 GetWorld()->LineTraceSingleByChannel(CameraHit,Eye,Far,ECC_Visibility,Q);
 const FVector Target=CameraHit.bBlockingHit?CameraHit.ImpactPoint:Far;
 if(!GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,42),Muzzle,ECC_Visibility,Q))GetWorld()->LineTraceSingleByChannel(Hit,Muzzle,Target+(Target-Muzzle).GetSafeNormal()*3.f,ECC_Visibility,Q);LastShotEnd=Hit.bBlockingHit?Hit.ImpactPoint:Target;
 DrawDebugLine(GetWorld(),Muzzle,LastShotEnd,FColor(255,185,70),false,.08f,0,1.5f);
 if(Hit.GetActor())UGameplayStatics::ApplyPointDamage(Hit.GetActor(),34,(Target-Muzzle).GetSafeNormal(),Hit,PC,this,UPiedmontBulletDamage::StaticClass());
 PC->AddPitchInput(-.7f);return true;
}
float APiedmontExplorer::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(Amount<=0||bDead)return 0;
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));if(!Mode||Mode->bRunEnded)return 0;
 APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,35),FVector::UpVector);
 if(Event.DamageTypeClass&&Event.DamageTypeClass->IsChildOf(UPiedmontKnifeDamage::StaticClass())){
  if(bKnifeWounded)Mode->EndRun(TEXT("Second stab — run ended"));else {bKnifeWounded=true;GetCharacterMovement()->StopMovementImmediately();}
 }else if(Event.DamageTypeClass&&Event.DamageTypeClass->IsChildOf(UPiedmontBulletDamage::StaticClass()))Mode->EndRun(TEXT("Shot — run ended"));
 return Amount;
}

void APiedmontExplorer::AimAtForValidation(AActor* Target){
#if WITH_EDITOR
 if(!IsValid(Target))return;if(auto* PC=Cast<APlayerController>(GetController())){FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);PC->SetControlRotation((Target->GetActorLocation()+FVector(0,0,25)-Eye).Rotation());}
#endif
}
