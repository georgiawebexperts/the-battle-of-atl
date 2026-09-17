#include "BattleRider.h"
#include "BattlePolice.h"
#include "BattleZombie.h"
#include "EngineUtils.h"
#include "Misc/App.h"
#include "BattleSpareBikes.h"
#include "BattleWeaponGrip.h"
#include "BattlePlayerCrash.h"
#include "BattleKnife.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "BattleShot.h"
#include "BattleDisc.h"
#include "BattleBike.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
namespace {
void PoseThirdPersonGrip(UPoseableMeshComponent* Body,FVector Forward,float Weight,int32 WeaponSlot,FVector SupportGrip){
 const bool LongGun=WeaponSlot>0;
 auto* Mesh=Cast<USkeletalMesh>(Body->GetSkinnedAsset());if(!Mesh)return;
 const auto& Ref=Mesh->GetRefSkeleton();TArray<FTransform> Pose=Body->BoneSpaceTransforms;
 for(int I=0;I<Pose.Num();I++)if(Ref.GetParentIndex(I)>=0)Pose[I]=Pose[I]*Pose[Ref.GetParentIndex(I)];
 auto Child=[&](int I,int Root){while(I>=0){if(I==Root)return true;I=Ref.GetParentIndex(I);}return false;};
 auto Rotate=[&](int Root,FQuat Rotation){if(Root<0)return;Rotation=FQuat::Slerp(FQuat::Identity,Rotation,Weight);const FVector Pivot=Pose[Root].GetLocation();for(int I=Root;I<Pose.Num();I++)if(Child(I,Root)){Pose[I].SetLocation(Pivot+Rotation.RotateVector(Pose[I].GetLocation()-Pivot));Pose[I].SetRotation(Rotation*Pose[I].GetRotation());}};
 // Blade the shotgun stance so the rear grip clears the chest while the support shoulder reaches forward.
 if(WeaponSlot==1){
  Rotate(Ref.FindBoneIndex(TEXT("spine_02")),FQuat(FVector::UpVector,FMath::DegreesToRadians(30.f)));
  Rotate(Ref.FindBoneIndex(TEXT("neck_01")),FQuat(FVector::UpVector,FMath::DegreesToRadians(-30.f)));
 }
 // Solve both arms from the current locomotion pose, preserving segment lengths.
 const FVector Right=FVector::CrossProduct(FVector::UpVector,Forward).GetSafeNormal();
 auto Solve=[&](const TCHAR* Side,FVector Target){
  const int U=Ref.FindBoneIndex(*FString::Printf(TEXT("upperarm_%s"),Side)),L=Ref.FindBoneIndex(*FString::Printf(TEXT("lowerarm_%s"),Side)),H=Ref.FindBoneIndex(*FString::Printf(TEXT("hand_%s"),Side));
  if(U<0||L<0||H<0)return;
  const FVector Origin=Pose[U].GetLocation();Target=FMath::Lerp(Pose[H].GetLocation(),Target,Weight);
  const float A=FVector::Dist(Origin,Pose[L].GetLocation()),B=FVector::Dist(Pose[L].GetLocation(),Pose[H].GetLocation());
  const FVector D=(Target-Origin).GetSafeNormal();const float Distance=FMath::Clamp(FVector::Dist(Origin,Target),FMath::Abs(A-B)+.1f,A+B-.1f);
  const float Along=(A*A-B*B+Distance*Distance)/(2*Distance),Height=FMath::Sqrt(FMath::Max(0.f,A*A-Along*Along));
  const FVector Bend=Right*(FCString::Strcmp(Side,TEXT("r"))==0?1.f:-1.f)-FVector::UpVector;
  const FVector Elbow=Origin+D*Along+FVector::VectorPlaneProject(Bend,D).GetSafeNormal()*Height;
  // Targets are already blended; solve with full rotations to avoid double blending.
  const float Saved=Weight;Weight=1;
  Rotate(U,FQuat::FindBetweenVectors(Pose[L].GetLocation()-Origin,Elbow-Origin));
  Rotate(L,FQuat::FindBetweenVectors(Pose[H].GetLocation()-Pose[L].GetLocation(),Origin+D*Distance-Pose[L].GetLocation()));Weight=Saved;
 };
 const int Shoulder=Ref.FindBoneIndex(TEXT("upperarm_r"));
 if(Shoulder>=0){
  const FVector GripTarget=Pose[Shoulder].GetLocation()+Forward*(WeaponSlot==1?20.f:LongGun?25.f:43.f)-FVector::UpVector*8.f;
  Solve(TEXT("r"),GripTarget);
  const FVector Delta=LongGun?SupportGrip-BattleWeaponGrip::Right(WeaponSlot):FVector(3,-9,-2);
  Solve(TEXT("l"),GripTarget+Forward*Delta.X+Right*Delta.Y+FVector::CrossProduct(Forward,Right).GetSafeNormal()*Delta.Z);
 }
 // The shotgun has a vertical front grip; orient its support palm as well.
 for(const TCHAR* Side:{TEXT("r"),TEXT("l")}){
  const bool Left=FCString::Strcmp(Side,TEXT("l"))==0;if(Left&&WeaponSlot!=1)continue;
  auto BoneIndex=[&](const TCHAR* Name){return Ref.FindBoneIndex(*FString::Printf(TEXT("%s_%s"),Name,Side));};
  const int Hand=BoneIndex(TEXT("hand")),Middle=BoneIndex(TEXT("middle_01")),Index=BoneIndex(TEXT("index_01")),Pinky=BoneIndex(TEXT("pinky_01"));
  if(Hand<0||Middle<0||Index<0||Pinky<0)continue;
  Rotate(Hand,FQuat::FindBetweenVectors(Pose[Middle].GetLocation()-Pose[Hand].GetLocation(),Forward));
  const FVector Across=FVector::VectorPlaneProject(Pose[Pinky].GetLocation()-Pose[Index].GetLocation(),Forward).GetSafeNormal();
  const FVector Down=-FVector::CrossProduct(Forward,Right).GetSafeNormal();Rotate(Hand,FQuat(Forward,FMath::Atan2(FVector::DotProduct(FVector::CrossProduct(Across,Down),Forward),FVector::DotProduct(Across,Down))));
  for(const TCHAR* Finger:{TEXT("index"),TEXT("middle"),TEXT("ring"),TEXT("pinky")})for(int Joint=1;Joint<=3;Joint++){
   const int Bone=Ref.FindBoneIndex(*FString::Printf(TEXT("%s_%02d_%s"),Finger,Joint,Side));
   const float Curl=(Joint==1?35.f:65.f)*(!Left&&FCString::Strcmp(Finger,TEXT("index"))==0?.5f:1.f);
   Rotate(Bone,FQuat(-Down,FMath::DegreesToRadians(Left?Curl:-Curl)));
  }
 }
 for(int I=0;I<Pose.Num();I++)Body->BoneSpaceTransforms[I]=Ref.GetParentIndex(I)>=0?Pose[I].GetRelativeTransform(Pose[Ref.GetParentIndex(I)]):Pose[I];
 Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
}
}
ABattleRider::ABattleRider(){
 Camera->SetupAttachment(CameraArm,USpringArmComponent::SocketName);Camera->SetRelativeLocation(FVector::ZeroVector);Camera->bUsePawnControlRotation=false;
 CameraArm->SetComponentTickEnabled(true);CameraArm->bDoCollisionTest=true;Body->SetOwnerNoSee(false);
 GetCharacterMovement()->MaxWalkSpeed=520;GetCharacterMovement()->JumpZVelocity=560;GetCharacterMovement()->AirControl=.8f;GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch=true;GetCharacterMovement()->CrouchedHalfHeight=48;GetCharacterMovement()->MaxWalkSpeedCrouched=240;GetCharacterMovement()->GravityScale=1.4f;
 FirstPersonArms=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("FirstPersonArms"));FirstPersonArms->SetupAttachment(Camera);FirstPersonArms->SetRelativeLocation(FVector(22,0,-175));FirstPersonArms->SetRelativeRotation(FRotator(0,-90,0));FirstPersonArms->SetOnlyOwnerSee(true);FirstPersonArms->SetBoundsScale(10);FirstPersonArms->SetCastShadow(false);FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);FirstPersonArms->SetCanEverAffectNavigation(false);
 Weapon->SetupAttachment(Camera);bWeaponDrawn=false;BuildMeleeVisual();BuildLongGun();
}
void ABattleRider::BeginPlay(){
 InitializeDetailedBodyPreview();
 Super::BeginPlay();
 InitializeDetailedPistolPreview();
 if(auto* Steel=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")))for(auto Child:MeleeRoot->GetAttachChildren())if(auto* Mesh=Cast<UStaticMeshComponent>(Child))Mesh->SetMaterial(0,Steel);
 if(auto* Steel=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")))for(auto Part:LongGunParts)Part->SetMaterial(0,Steel);
 if(LongGunParts.Num()>5)LongGunParts[5]->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow")));
 Weapon->SetRelativeLocation(GunRestPosition);Weapon->SetVisibility(false);GunRestRotation=Weapon->GetRelativeRotation();
 if(auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/BattleForTheA/Rider/SK_FPSArms.SK_FPSArms"))){FirstPersonArms->SetSkinnedAssetAndUpdate(Mesh);InitializeDetailedArmsPreview();const auto& Ref=Cast<USkeletalMesh>(FirstPersonArms->GetSkinnedAsset())->GetRefSkeleton();for(int32 I=0;I<Ref.GetNum();I++){ArmParents.Add(Ref.GetParentIndex(I));ArmNames.Add(Ref.GetBoneName(I));FTransform T=Ref.GetRefBonePose()[I];if(ArmParents[I]>=0)T=T*ArmRest[ArmParents[I]];ArmRest.Add(T);}}
 if(IsValid(ParkedBike))GetCapsuleComponent()->IgnoreActorWhenMoving(ParkedBike,true);
}
void ABattleRider::SetupPlayerInputComponent(UInputComponent* I){
 ACharacter::SetupPlayerInputComponent(I);
 I->BindAxisKey(EKeys::MouseX,this,&ABattleRider::LookYaw);I->BindAxisKey(EKeys::MouseY,this,&ABattleRider::LookPitch);
 I->BindKey(EKeys::LeftMouseButton,IE_Pressed,this,&ABattleRider::PullTrigger);I->BindKey(EKeys::LeftMouseButton,IE_Released,this,&ABattleRider::ReleaseTrigger);
 I->BindKey(EKeys::RightMouseButton,IE_Pressed,this,&ABattleRider::AimOn);I->BindKey(EKeys::RightMouseButton,IE_Released,this,&ABattleRider::AimOff);
 I->BindKey(EKeys::E,IE_Pressed,this,&ABattleRider::Interact);
 I->BindKey(EKeys::SpaceBar,IE_Pressed,this,&ABattleRider::StartJump);I->BindKey(EKeys::SpaceBar,IE_Released,this,&ABattleRider::EndJump);
 I->BindKey(EKeys::G,IE_Pressed,this,&ABattleRider::DrawWeapon);
 I->BindKey(EKeys::C,IE_Pressed,this,&ABattleRider::ToggleCrouch);I->BindKey(EKeys::LeftControl,IE_Pressed,this,&ABattleRider::StartCrouch);I->BindKey(EKeys::LeftControl,IE_Released,this,&ABattleRider::EndCrouch);
 I->BindKey(EKeys::R,IE_Pressed,this,&ABattleRider::ReloadPistol);
 I->BindKey(EKeys::F,IE_Pressed,this,&ABattleRider::StartMelee);
 I->BindKey(EKeys::One,IE_Pressed,this,&ABattleRider::SelectPistol);I->BindKey(EKeys::Two,IE_Pressed,this,&ABattleRider::SelectShotgun);I->BindKey(EKeys::Three,IE_Pressed,this,&ABattleRider::SelectSMG);I->BindKey(EKeys::Five,IE_Pressed,this,&ABattleRider::SelectRifle);I->BindKey(EKeys::Four,IE_Pressed,this,&ABattleRider::SelectFrisbee);
}
bool ABattleRider::CanUseWeapon() const{const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));return Health>0&&(!ParkedBike||(ParkedBike->RiderHealth>0&&ParkedBike->StunRemaining<=0))&&!UGameplayStatics::IsGamePaused(this)&&!bSwimming&&GetController()&&(!Mode||(Mode->StartCountdown<=0&&!Mode->bRunEnded));}
void ABattleRider::Tick(float Dt){
 UpdateShotgunMechanism(Dt);
 ShotCooldown=FMath::Max(0.f,ShotCooldown-Dt);HitFeedback=FMath::Max(0.f,HitFeedback-Dt);Kick=FMath::FInterpTo(Kick,0.f,Dt,14);
 SwayTime+=Dt*FMath::Clamp(GetVelocity().Size2D()/(bDetailedPlayerRig?32.f:80.f),0.f,11.f);const float Bob=FMath::Sin(SwayTime)*FMath::Min(GetVelocity().Size2D()/850.f,1.f)*.65f;
 const float ReloadPose=CurrentWeapon==1&&bShotgunReloading?ShotgunReloadBlend:ReloadRemaining>0?FMath::Sin(PI*FMath::Clamp((BattleWeapons::ReloadSeconds(CurrentWeapon)-ReloadRemaining)/BattleWeapons::ReloadSeconds(CurrentWeapon),0.f,1.f)):0;

 DrawRemaining=FMath::Max(0.f,DrawRemaining-Dt);
 if(auto* PC=Cast<APlayerController>(GetController())){
  GetCharacterMovement()->MaxWalkSpeed=(PC->IsInputKeyDown(EKeys::LeftShift)?700:400);
  bAiming=PC->IsInputKeyDown(EKeys::RightMouseButton)&&bWeaponDrawn&&DrawRemaining<=0&&CanUseWeapon()&&ReloadRemaining<=0&&MeleeRemaining<=0;
  if(PC->IsInputKeyDown(EKeys::LeftMouseButton))Fire();
 }
 bUseControllerRotationYaw=bWeaponDrawn;
 GetCharacterMovement()->bOrientRotationToMovement=!bWeaponDrawn;
 GetCharacterMovement()->RotationRate=FRotator(0,540,0);
 // Exponential smoothing gives the view rig the same response at different frame rates.
 AimBlend=FMath::Lerp(AimBlend,bAiming?1.f:0.f,1.f-FMath::Exp(-12.f*Dt));
 const FVector ViewPosition=FMath::Lerp(GunRestPosition,CurrentWeapon==1?FVector(65,-1.28f,-9.5f):GunAimPosition,AimBlend);
 Weapon->SetRelativeLocation(ViewPosition+FVector(-4*Kick-4*ReloadPose,-8*ReloadPose,-Kick+Bob*(1.f-.8f*AimBlend)+(bDetailedPlayerRig?(DetailedPistol&&CurrentWeapon==0?16.f:8.f):-8.f)*ReloadPose-65*FMath::Clamp(DrawRemaining/.3f,0.f,1.f)));
 Weapon->SetRelativeRotation(GunRestRotation+FRotator(5*Kick+22*ReloadPose,0,-28*ReloadPose));
 if(CurrentWeapon==1&&bShotgunReloading){
  // Present the receiver across the view so shell handling remains visible.
  Weapon->SetRelativeLocation(FMath::Lerp(ViewPosition,FVector(65,-5,-10),ReloadPose)+FVector(-4*Kick,0,-Kick));
  Weapon->SetRelativeRotation(GunRestRotation+FRotator(5*Kick,-45*ReloadPose,65*ReloadPose));
 }
 Super::Tick(Dt);
 const bool Stunned=ParkedBike&&ParkedBike->StunRemaining>0;
 Camera->SetRelativeLocation(FVector::ZeroVector);CameraArm->SetRelativeLocation(FVector(0,0,FMath::FInterpTo(CameraArm->GetRelativeLocation().Z,bSwimming?20.f:(Stunned?25.f:(bIsCrouched?38.f:64.f)),Dt,8)));
 UpdateMelee(Dt);UpdateWeaponModel();UpdateDetailedPistol();PoseArms(Dt);
 FirstPersonArms->SetHiddenInGame(true,true);
 // The camera follows Ellison; weapons stay with his body rather than the lens.
 const FRotator AimRotation=GetController()?GetControlRotation():GetActorRotation();
 const FVector AimForward=AimRotation.Vector();
 const FName Hand=bDetailedPlayerRig?FName(TEXT("hand_r")):FName(TEXT("Hand_R"));
 if(bDetailedPlayerRig&&bWeaponDrawn&&CurrentWeapon!=3&&!bSwimming)PoseThirdPersonGrip(Body,Body->GetComponentTransform().InverseTransformVectorNoScale(AimForward),1.f-FMath::Clamp(DrawRemaining/.3f,0.f,1.f),CurrentWeapon,CurrentWeapon==1?ShotgunReloadHand:BattleWeaponGrip::Left(CurrentWeapon));
 const FVector Grip=Body->GetSocketLocation(Hand);
 Weapon->SetWorldRotation((bWeaponDrawn?AimRotation:GetActorRotation())+GunRestRotation);
 FVector WeaponOrigin=Grip;
 if(CurrentWeapon==0&&DetailedPistol){
  auto* Mesh=Cast<USkeletalMesh>(DetailedPistol->GetSkinnedAsset());
  if(Mesh)WeaponOrigin=Grip+AimForward*12-DetailedPistol->GetComponentTransform().TransformVector(Mesh->GetBounds().Origin);
 }
 Weapon->SetWorldLocation(WeaponOrigin);
 if(CurrentWeapon>0&&CurrentWeapon!=3){
  // Long-gun art uses receiver-centred origins, not a grip socket. Keep the
  // weapon-specific wrist offset at Ellison's hand rather than at the camera.
  const FVector Wrist=BattleWeaponGrip::Right(CurrentWeapon);
  LongGun->SetWorldRotation(bWeaponDrawn?AimRotation:GetActorRotation());
  LongGun->SetWorldLocation(Grip-LongGun->GetComponentQuat().RotateVector(Wrist));
 }
 if(IsValid(ParkedBike))Health=ParkedBike->RiderHealth;
}
bool ABattleRider::MountBike(){
 if(!IsValid(ParkedBike)||ParkedBike->StunRemaining>0||Health<=0||bSwimming)return false;
 if(auto* Spare=BattleSpareBikes::Nearest(this))if(FVector::DistSquared(Spare->GetActorLocation(),GetActorLocation())<FVector::DistSquared(ParkedBike->GetActorLocation(),GetActorLocation()))return BattleSpareBikes::Mount(this,Spare);
 return ParkedBike->Remount(this);
}
float ABattleRider::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(!IsValid(ParkedBike))return 0;
 const float Applied=ParkedBike->ApplyRiderDamage(Amount);Health=ParkedBike->RiderHealth;
 return Applied;
}
bool ABattleBike::Dismount(){
 auto* PC=Cast<APlayerController>(GetController());if(!PC||bParked||RiderHealth<=0||Ride->Recovery>0)return false;
 FVector Exit;bool Found=false;FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleDismount),false,this);
 for(const FVector Direction:{GetActorRightVector(),-GetActorRightVector(),-GetActorForwardVector(),GetActorForwardVector()}){
  const FVector Candidate=GetActorLocation()+Direction*145;FHitResult Ground;
  if(!GetWorld()->LineTraceSingleByChannel(Ground,Candidate+FVector(0,0,100),Candidate-FVector(0,0,220),ECC_Visibility,Q)||Ground.ImpactNormal.Z<.65f)continue;
  // A vertical capsule needs extra clearance above a sloped plane.
  const float Clearance=88.f+30.f*(1.f/FMath::Max(.65f,Ground.ImpactNormal.Z)-1.f)+2.f;
  Exit=Ground.ImpactPoint+FVector(0,0,Clearance);
  FHitResult Wall;if(GetWorld()->SweepSingleByChannel(Wall,GetActorLocation(),Exit,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q))continue;
  if(!GetWorld()->OverlapBlockingTestByChannel(Exit,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q)){Found=true;break;}
 }
 if(!Found)return false;
 FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
 // Hop off with a small step and lean instead of popping into place.
 auto* Person=GetWorld()->SpawnActor<ABattleRider>(Exit+FVector(0,0,34),GetActorRotation()+FRotator(8,0,0),P);if(!Person)return false;
 Ride->BoostRemaining=0;Ride->Speed=Ride->ReverseSpeed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->bReverseRequested=false;Ride->StopMovementImmediately();Ride->DisableMovement();bParked=true;Visual->SetRelativeRotation(FRotator::ZeroRotator);Rider->SetVisibility(false,true);
 ReloadTimer=0;LeanAngle=0;Ride->SmoothedSteer=Ride->TurnRateDegrees=0;Person->ParkedBike=this;Person->Health=RiderHealth;Person->RestoreLoadout();Person->GetCapsuleComponent()->IgnoreActorWhenMoving(this,true);PC->Possess(Person);
 // On foot the player is auto-aimed at the nearest threat, or the bike, and
 // told that the mouse now drives the camera.
 FVector Aim=GetActorLocation();float Best=3400.f;
 for(TActorIterator<ABattlePolice> It(GetWorld());It;++It)if(!It->bDead){const float D=FVector::Dist2D(It->GetActorLocation(),Exit);if(D<Best){Best=D;Aim=It->GetActorLocation();}}
 for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)if(!It->bDead){const float D=FVector::Dist2D(It->GetActorLocation(),Exit);if(D<Best){Best=D;Aim=It->GetActorLocation();}}
 PC->SetControlRotation(FRotator(0,(Aim-Exit).Rotation().Yaw,0));
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))
  Mode->PushHint(TEXT("onfoot"),TEXT("ON FOOT — MOVE THE MOUSE TO LOOK AROUND  ·  E TO GET BACK ON THE BIKE"),7.f);
 PC->bShowMouseCursor=false;PC->ResetIgnoreLookInput();if(!FApp::IsUnattended())PC->SetInputMode(FInputModeGameOnly());return true;
}
bool ABattleBike::Remount(ABattleRider* Person){
 if(bCrashActive||StunRemaining>0||!IsValid(Person)||Person->ParkedBike!=this)return false;
 if(IsValid(PlayerCrash)&&!PlayerCrash->PrepareRemount(Person))return false;
 if(StunRemaining>0||!IsValid(Person)||Person->ParkedBike!=this||!bParked||FVector::Dist(GetActorLocation(),Person->GetActorLocation())>240)return false;
 auto* PC=Cast<APlayerController>(Person->GetController());if(!PC)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleRemount),false,this);Q.AddIgnoredActor(Person);
 // Match the bike movement filter, including the legacy invisible lake barrier.
 for(const auto& Ignored:GetCapsuleComponent()->GetMoveIgnoreActors())Q.AddIgnoredActor(Ignored);
 if(GetWorld()->OverlapBlockingTestByChannel(GetActorLocation(),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q))return false;
 Person->SaveWeapon();ABattleKnife::OnRemounted(this);bParked=false;ClearPhysicalCrash();Visual->SetRelativeRotation(FRotator::ZeroRotator);Rider->SetRelativeLocation(FVector::ZeroVector);Ride->StopMovementImmediately();Ride->Speed=Ride->ReverseSpeed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->bReverseRequested=false;Ride->SetMovementMode(MOVE_Walking);Rider->SetVisibility(!bFirstPerson,true);PC->Possess(this);PC->SetControlRotation(GetActorRotation());PC->bShowMouseCursor=false;PC->ResetIgnoreLookInput();if(!FApp::IsUnattended())PC->SetInputMode(FInputModeGameOnly());Person->Destroy();return true;
}

bool ABattleRider::Fire(){
 if(!CanUseWeapon()||!bWeaponDrawn||DrawRemaining>0||MeleeRemaining>0||ShotCooldown>0||ReloadRemaining>0||Ammo<=0)return false;
 if(CurrentWeapon==3){if(!ABattleDisc::Launch(this,LongGun))return false;Ammo--;ShotsFired++;ShotCooldown=.55f;Kick=.7f;SaveWeapon();return true;}
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))Mode->RecordGunfire();
 Ammo--;ShotsFired++;ShotCooldown=CurrentWeapon==1?.85f:CurrentWeapon==2?.085f:CurrentWeapon==4?.12f:.22f;Kick=CurrentWeapon==1?2:1;if(CurrentWeapon==1)ShotgunPumpRemaining=.75f;SaveWeapon();
 const auto Shot=CurrentWeapon==0?FireBattlePistol(this,Weapon,bAiming?.1f:.4f):FireBattleLongGun(this,LongGun,CurrentWeapon,bAiming);
 LastShotEnd=Shot.End;if(Shot.Damage>0)HitFeedback=.35f;
 if(IsValid(ParkedBike)&&!Shot.HitLabel.IsEmpty()){ParkedBike->ShotNotice=Shot.HitLabel;ParkedBike->ShotNoticeRemaining=.75f;}
 if(IsValid(ParkedBike))for(int32 I=0;I<Shot.Kills;I++)ParkedBike->AwardEnemyKill();return true;
}

bool ABattleRider::ToggleDrawWeapon(){
 if(!CanUseWeapon()||MeleeRemaining>0)return false;
 bWeaponDrawn=!bWeaponDrawn;bAiming=false;ReloadRemaining=0;DrawRemaining=bWeaponDrawn?.3f:0;
 Weapon->SetVisibility(bWeaponDrawn&&CurrentWeapon==0);UpdateWeaponModel();return true;
}

void ABattleRider::OnStartCrouch(float HalfHeightAdjust,float ScaledHalfHeightAdjust){
 Super::OnStartCrouch(HalfHeightAdjust,ScaledHalfHeightAdjust);
 Camera->AddLocalOffset(FVector(0,0,ScaledHalfHeightAdjust));
}
void ABattleRider::OnEndCrouch(float HalfHeightAdjust,float ScaledHalfHeightAdjust){
 Super::OnEndCrouch(HalfHeightAdjust,ScaledHalfHeightAdjust);
 Camera->AddLocalOffset(FVector(0,0,-ScaledHalfHeightAdjust));
}
