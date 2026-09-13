#include "BattleGunman.h"
#include "BattleDetailedRider.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/CommandLine.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleShot.h"
#include "PiedmontBlood.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Components/PoseableMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "UObject/ConstructorHelpers.h"
ABattleGunman::ABattleGunman(){
 Tags.Add(TEXT("PiedmontHostile"));Tags.Add(TEXT("BattleHostile"));Tags.Add(TEXT("BattleGunman"));
 static ConstructorHelpers::FObjectFinder<UAnimSequence> AimClip(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Idle_Gun_Shoot.Animations_CharacterArmature_Idle_Gun_Shoot"));GunIdle=AimClip.Object;
 static ConstructorHelpers::FObjectFinder<UAnimSequence> RestClip(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Idle.Animations_CharacterArmature_Idle"));RelaxedIdle=RestClip.Object;
 static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkClip(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Walk.Animations_CharacterArmature_Walk"));GunWalk=WalkClip.Object;
 static ConstructorHelpers::FObjectFinder<UAnimSequence> RunClip(TEXT("/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Run.Animations_CharacterArmature_Run"));GunRun=RunClip.Object;
 bUseControllerRotationYaw=false;PrimaryActorTick.TickGroup=TG_PostPhysics;
 static ConstructorHelpers::FObjectFinder<UPhysicsAsset> DeathCollision(TEXT("/Game/BattleForTheA/Rider/Physics/PA_EllisonCrashCandidateV6.PA_EllisonCrashCandidateV6"));DeathAsset=DeathCollision.Object;
}
void ABattleGunman::BeginPlay(){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleDetailedGunman"))){
  const FString Root=TEXT("/Game/CitySampleCrowd/Character/Female/");
  const FString MeshRoot=Root+TEXT("NormalWeight/Meshes/f_tal_nrw_");
  auto* Mesh=LoadObject<USkeletalMesh>(nullptr,*(MeshRoot+TEXT("body")));
  auto* HairMesh=LoadObject<UStaticMesh>(nullptr,*(Root+TEXT("f_001/Hair/Hair/Hair_S_Coil_CardsMesh_Group0_LOD0")));
  const FString AnimRoot=TEXT("/Game/CitySampleCrowd/Character/Anims/Loco/FTN_Set/");
  auto* Idle=LoadObject<UAnimSequence>(nullptr,*(AnimRoot+TEXT("FTN_N_Idle_Base")));
  auto* Walk=LoadObject<UAnimSequence>(nullptr,*(AnimRoot+TEXT("FTN_N_Walk_F")));
  auto* Quick=LoadObject<UAnimSequence>(nullptr,*(AnimRoot+TEXT("FTN_N_Walk_F_Quickly")));
  TArray<USkeletalMesh*> Parts;
  for(const FString& Path:TArray<FString>{MeshRoot+TEXT("scoopneck"),MeshRoot+TEXT("jeans"),MeshRoot+TEXT("loafers"),Root+TEXT("f_001/Face/f_001_nrw_FaceMesh")})Parts.Add(LoadObject<USkeletalMesh>(nullptr,*Path));
  if(Mesh&&HairMesh&&Idle&&Walk&&Quick&&!Parts.Contains(nullptr)){
   Body->SetSkinnedAssetAndUpdate(Mesh);bNativeCrowdRig=bDetailedPlayerRig=true;
   GunIdle=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/Gunman/AimGrounded/GunmanPointing"));RelaxedIdle=Idle;GunWalk=Walk;GunRun=Quick;
   DeathAsset=Mesh->GetPhysicsAsset();
   if(GunIdle)for(const TCHAR* Name:{TEXT("root"),TEXT("pelvis"),TEXT("spine_01"),TEXT("hand_r")}){
    const auto& AimRef=GunIdle->GetSkeleton()->GetReferenceSkeleton();const auto& WalkRef=GunWalk->GetSkeleton()->GetReferenceSkeleton();const auto& MeshRef=Mesh->GetRefSkeleton();
    const int A=AimRef.FindBoneIndex(Name),W=WalkRef.FindBoneIndex(Name),R=MeshRef.FindBoneIndex(Name);
    if(A>=0&&R>=0){FTransform T;GunIdle->GetBoneTransform(T,FSkeletonPoseBoneIndex(A),FAnimExtractContext(.5),false);UE_LOG(LogTemp,Display,TEXT("GunmanPose: bone=%s aim_index=%d walk_index=%d bind=%s sample=%s"),Name,A,W,*MeshRef.GetRefBonePose()[R].ToString(),*T.ToString());}
   }

   for(int I=0;I<Parts.Num();I++){
    auto* Part=NewObject<USkeletalMeshComponent>(this,*FString::Printf(TEXT("GunmanOutfit%d"),I));AddInstanceComponent(Part);Part->SetupAttachment(Body);
    Part->SetDisablePostProcessBlueprint(true);Part->SetSkeletalMeshAsset(Parts[I]);Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCanEverAffectNavigation(false);Part->SetLeaderPoseComponent(Body,true,false);Part->RegisterComponent();
   }
   auto* Hair=NewObject<UStaticMeshComponent>(this,TEXT("GunmanHair"));AddInstanceComponent(Hair);Hair->SetMobility(EComponentMobility::Movable);Hair->SetStaticMesh(HairMesh);Hair->SetCollisionEnabled(ECollisionEnabled::NoCollision);Hair->SetCanEverAffectNavigation(false);Hair->SetupAttachment(Body);Hair->RegisterComponent();Hair->AttachToComponent(Body,FAttachmentTransformRules::KeepWorldTransform,TEXT("head"));
   UE_LOG(LogTemp,Display,TEXT("DetailedGunman: body=%s parts=%d physics=%s"),*Mesh->GetName(),Parts.Num(),*GetNameSafe(DeathAsset));
  }
 }
#endif
 Super::BeginPlay();
}
bool ABattleGunman::CanAttack() const{
 const auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 return !bDead&&!bSwimming&&M&&!M->bTutorialActive&&M->StartCountdown<=0&&!M->bRunEnded&&!UGameplayStatics::IsGamePaused(this);
}
bool ABattleGunman::TryAim(APawn* Target){
 if(!CanAttack()||!Target||Cooldown>0||bWarning||FVector::Dist2D(GetActorLocation(),Target->GetActorLocation())>2200)return false;
 auto* B=Cast<ABattleBike>(Target);if(auto* F=Cast<ABattleRider>(Target))B=F->ParkedBike;
 if(!B||B->RiderHealth<=0||B->DamageGrace>0||B->RespawnRemaining>0)return false;
 const FVector Spot=Target->GetActorLocation()+FVector(0,0,20);FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 if(GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,42),Spot,ECC_Visibility,Q)&&Hit.GetActor()!=Target)return false;
 AimTarget=Target;AimPoint=Spot;WindupRemaining=1.8f;bWarning=true;return true;
}
bool ABattleGunman::ResolveShot(){
 if(!CanAttack()||!bWarning||WindupRemaining>0)return false;
 bWarning=false;Cooldown=4;ShotsFired++;ShotAlertRemaining=1.1f;
 const FVector Muzzle=Weapon->GetStaticMesh()?Weapon->GetComponentTransform().TransformPosition(Weapon->GetStaticMesh()->GetBounds().Origin)+GetActorForwardVector()*14:GetActorLocation()+GetActorForwardVector()*48+FVector(0,0,42);
 const FVector End=AimPoint+(AimPoint-Muzzle).GetSafeNormal()*80;FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 // Check the barrel path too, so a muzzle cannot protrude through cover.
 if(!GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,42),Muzzle,ECC_Visibility,Q))GetWorld()->LineTraceSingleByChannel(Hit,Muzzle,End,ECC_Visibility,Q);
 LastShotEnd=Hit.bBlockingHit?Hit.ImpactPoint:End;
 if(auto* FX=GetWorld()->SpawnActorDeferred<ABattleShotFX>(ABattleShotFX::StaticClass(),FTransform(Muzzle),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){FX->Start=Muzzle;FX->End=LastShotEnd;FX->FinishSpawning(FTransform(Muzzle));}
 if(auto* Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/PiedmontRide/Audio/S_Gunshot.S_Gunshot")))UGameplayStatics::PlaySoundAtLocation(this,Sound,Muzzle);
 auto* Player=UGameplayStatics::GetPlayerPawn(this,0);
 if(Hit.GetActor()==Player){auto* B=Cast<ABattleBike>(Player);if(auto* F=Cast<ABattleRider>(Player))B=F->ParkedBike;if(B)B->ApplyRiderDamage(1000);}
 return true;
}
void ABattleGunman::Tick(float Dt){
 if(bDead){Super::Tick(Dt);MirrorDeathPose();return;}
 ShotAlertRemaining=FMath::Max(0.f,ShotAlertRemaining-Dt);
 bWeaponDrawn=bWarning||ShotAlertRemaining>0;Super::Tick(Dt);
 if(!CanAttack()){bWarning=false;WindupRemaining=0;return;}
 Cooldown=FMath::Max(0.f,Cooldown-Dt);
 if(bWarning){SetActorRotation(FRotator(0,(AimPoint-GetActorLocation()).Rotation().Yaw,0));WindupRemaining=FMath::Max(0.f,WindupRemaining-Dt);if(WindupRemaining<=0)ResolveShot();}
 else TryAim(UGameplayStatics::GetPlayerPawn(this,0));
}
float ABattleGunman::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(bDead||!FMath::IsFinite(Amount)||Amount<=0)return 0;
 const float Applied=FMath::Min(Health,Amount);Health-=Applied;bWarning=false;WindupRemaining=0;Cooldown=FMath::Max(Cooldown,.8f);
 APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,20),Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal():FVector::UpVector);
 if(Health<=0){bDead=true;GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);Weapon->SetVisibility(false);BeginDeathPhysics(Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal2D():GetActorForwardVector());SetLifeSpan(8);}
 return Applied;
}

void ABattleGunman::AnimateBody(float Dt){
 const bool Drawn=bWeaponDrawn;const TArray<FTransform> Previous=bDetailedPlayerRig?PreviousAnimationPose:Body->BoneSpaceTransforms;
 SetLocomotionClips(Drawn&&GunIdle?GunIdle:RelaxedIdle,GunWalk,GunRun);
 // Keep the authored gun stance; the generic hand IK otherwise overwrites it.
 if(GunIdle)bWeaponDrawn=false;
 Super::AnimateBody(Dt);bWeaponDrawn=Drawn;
 if(Previous.Num()==Body->BoneSpaceTransforms.Num()&&Dt>0){
  const float Weight=1.f-FMath::Exp(-14.f*Dt);
  for(int I=0;I<Previous.Num();I++){FTransform Blended;Blended.Blend(Previous[I],Body->BoneSpaceTransforms[I],Weight);Body->BoneSpaceTransforms[I]=Blended;}
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
 }
 if(bDetailedPlayerRig)PreviousAnimationPose=Body->BoneSpaceTransforms;
 if(Drawn&&bDetailedPlayerRig){
  const auto& Ref=CastChecked<USkeletalMesh>(Body->GetSkinnedAsset())->GetRefSkeleton();
  TArray<FTransform> Pose=Body->BoneSpaceTransforms;
  for(int I=0;I<Pose.Num();I++)if(Ref.GetParentIndex(I)>=0)Pose[I]=Pose[I]*Pose[Ref.GetParentIndex(I)];
  auto Child=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Ref.GetParentIndex(I);}return false;};
  auto Rotate=[&](int Root,FQuat Rotation){if(Root<0)return;const FVector Pivot=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();I++)if(Child(I,Root)){Pose[I].SetLocation(Pivot+Rotation.RotateVector(Pose[I].GetLocation()-Pivot));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}};
  const int Hand=Ref.FindBoneIndex(TEXT("hand_r")),Middle=Ref.FindBoneIndex(TEXT("middle_01_r")),Index=Ref.FindBoneIndex(TEXT("index_01_r")),Pinky=Ref.FindBoneIndex(TEXT("pinky_01_r"));
  const FVector Forward(0,1,0);
  if(Hand>=0&&Middle>=0&&Index>=0&&Pinky>=0){
   Rotate(Hand,FQuat::FindBetweenVectors(Pose[Middle].GetLocation()-Pose[Hand].GetLocation(),Forward));
   const FVector Across=FVector::VectorPlaneProject(Pose[Pinky].GetLocation()-Pose[Index].GetLocation(),Forward).GetSafeNormal();
   const FVector Down(0,0,-1);const float Twist=FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(Across,Down),Forward),FVector::DotProduct(Across,Down));Rotate(Hand,FQuat(Forward,Twist));
   for(const TCHAR* Finger:{TEXT("index"),TEXT("middle"),TEXT("ring"),TEXT("pinky")})for(int Joint=1;Joint<=3;Joint++){
    const int Bone=Ref.FindBoneIndex(*FString::Printf(TEXT("%s_%02d_r"),Finger,Joint));
    const float Curl=(Joint==1?35.f:65.f)*(FCString::Strcmp(Finger,TEXT("index"))==0?.5f:1.f);
    Rotate(Bone,FQuat(FVector::UpVector,FMath::DegreesToRadians(-Curl)));
   }
   const int Thumb=Ref.FindBoneIndex(TEXT("thumb_01_r")),Tip=Ref.FindBoneIndex(TEXT("thumb_03_r"));
   if(Thumb>=0&&Tip>=0)Rotate(Thumb,FQuat::FindBetweenVectors(Pose[Tip].GetLocation()-Pose[Thumb].GetLocation(),FVector(0,1,.12).GetSafeNormal()));
   for(int I=0;I<Pose.Num();I++)Body->BoneSpaceTransforms[I]=Ref.GetParentIndex(I)>=0?Pose[I].GetRelativeTransform(Pose[Ref.GetParentIndex(I)]):Pose[I];
   Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
  }
 }
 if(Drawn&&Weapon->GetStaticMesh()){
  const FVector Centre=Body->GetSocketLocation(BattleDetailedBone(TEXT("Hand_R"),bDetailedPlayerRig))+GetActorForwardVector()*(bDetailedPlayerRig?16:8)+FVector(0,0,bDetailedPlayerRig?0:6);
  Weapon->SetWorldLocation(Centre-Weapon->GetComponentTransform().TransformVector(Weapon->GetStaticMesh()->GetBounds().Origin));
 }
}
