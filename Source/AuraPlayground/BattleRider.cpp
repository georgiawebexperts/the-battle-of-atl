#include "BattleRider.h"
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
ABattleRider::ABattleRider(){
 Camera->SetupAttachment(GetCapsuleComponent());Camera->SetRelativeLocation(FVector(0,0,64));Camera->bUsePawnControlRotation=true;
 CameraArm->SetComponentTickEnabled(false);Body->SetOwnerNoSee(true);
 GetCharacterMovement()->MaxWalkSpeed=520;GetCharacterMovement()->JumpZVelocity=560;GetCharacterMovement()->AirControl=.8f;GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch=true;GetCharacterMovement()->CrouchedHalfHeight=48;GetCharacterMovement()->MaxWalkSpeedCrouched=240;GetCharacterMovement()->GravityScale=1.4f;
 FirstPersonArms=CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("FirstPersonArms"));FirstPersonArms->SetupAttachment(Camera);FirstPersonArms->SetRelativeLocation(FVector(22,0,-175));FirstPersonArms->SetRelativeRotation(FRotator(0,-90,0));FirstPersonArms->SetOnlyOwnerSee(true);FirstPersonArms->SetBoundsScale(10);FirstPersonArms->SetCastShadow(false);FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);FirstPersonArms->SetCanEverAffectNavigation(false);
 Weapon->SetupAttachment(Camera);bWeaponDrawn=false;BuildMeleeVisual();BuildLongGun();
}
void ABattleRider::BeginPlay(){
 Super::BeginPlay();
 if(auto* Steel=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")))for(auto Child:MeleeRoot->GetAttachChildren())if(auto* Mesh=Cast<UStaticMeshComponent>(Child))Mesh->SetMaterial(0,Steel);
 if(auto* Steel=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_LockSteel.M_LockSteel")))for(auto Part:LongGunParts)Part->SetMaterial(0,Steel);
 if(LongGunParts.Num()>5)LongGunParts[5]->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BattleForTheA/Materials/M_DiscGlow.M_DiscGlow")));
 Weapon->SetRelativeLocation(GunRestPosition);Weapon->SetVisibility(false);GunRestRotation=Weapon->GetRelativeRotation();
 if(auto* Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/BattleForTheA/Rider/SK_FPSArms.SK_FPSArms"))){FirstPersonArms->SetSkinnedAssetAndUpdate(Mesh);InitializeDetailedArmsPreview();const auto& Ref=Cast<USkeletalMesh>(FirstPersonArms->GetSkinnedAsset())->GetRefSkeleton();for(int32 I=0;I<Ref.GetNum();I++){ArmParents.Add(Ref.GetParentIndex(I));ArmNames.Add(Ref.GetBoneName(I));FTransform T=Ref.GetRefBonePose()[I];if(ArmParents[I]>=0)T=T*ArmRest[ArmParents[I]];ArmRest.Add(T);}}
 if(IsValid(ParkedBike))GetCapsuleComponent()->IgnoreActorWhenMoving(ParkedBike,true);
}
void ABattleRider::SetupPlayerInputComponent(UInputComponent* I){
 ACharacter::SetupPlayerInputComponent(I);
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
 ShotCooldown=FMath::Max(0.f,ShotCooldown-Dt);HitFeedback=FMath::Max(0.f,HitFeedback-Dt);Kick=FMath::FInterpTo(Kick,0.f,Dt,14);
 SwayTime+=Dt*FMath::Clamp(GetVelocity().Size2D()/80.f,0.f,11.f);const float Bob=FMath::Sin(SwayTime)*FMath::Min(GetVelocity().Size2D()/850.f,1.f)*.65f;
 const float ReloadPose=ReloadRemaining>0?FMath::Sin(PI*FMath::Clamp((BattleWeapons::ReloadSeconds(CurrentWeapon)-ReloadRemaining)/BattleWeapons::ReloadSeconds(CurrentWeapon),0.f,1.f)):0;

 DrawRemaining=FMath::Max(0.f,DrawRemaining-Dt);
 if(auto* PC=Cast<APlayerController>(GetController())){
  GetCharacterMovement()->MaxWalkSpeed=PC->IsInputKeyDown(EKeys::LeftShift)?850:520;
  bAiming=PC->IsInputKeyDown(EKeys::RightMouseButton)&&bWeaponDrawn&&DrawRemaining<=0&&CanUseWeapon()&&ReloadRemaining<=0&&MeleeRemaining<=0;
  if(PC->IsInputKeyDown(EKeys::LeftMouseButton))Fire();
 }
 // Exponential smoothing gives the view rig the same response at different frame rates.
 AimBlend=FMath::Lerp(AimBlend,bAiming?1.f:0.f,1.f-FMath::Exp(-12.f*Dt));
 const FVector ViewPosition=FMath::Lerp(GunRestPosition,GunAimPosition,AimBlend);
 Weapon->SetRelativeLocation(ViewPosition+FVector(-4*Kick-4*ReloadPose,-8*ReloadPose,-Kick+Bob*(1.f-.8f*AimBlend)-8*ReloadPose-65*FMath::Clamp(DrawRemaining/.3f,0.f,1.f)));
 Weapon->SetRelativeRotation(GunRestRotation+FRotator(5*Kick+22*ReloadPose,0,-28*ReloadPose));
 Super::Tick(Dt);
 const bool Stunned=ParkedBike&&ParkedBike->StunRemaining>0;
 Camera->SetRelativeLocation(FVector(0,0,FMath::FInterpTo(Camera->GetRelativeLocation().Z,Stunned?8.f:(bIsCrouched?38.f:64.f),Dt,8)));
 UpdateMelee(Dt);UpdateWeaponModel();PoseArms(Dt);if(IsValid(ParkedBike))Health=ParkedBike->RiderHealth;
}
bool ABattleRider::MountBike(){return (!ParkedBike||ParkedBike->StunRemaining<=0)&&Health>0&&!bSwimming&&IsValid(ParkedBike)&&ParkedBike->Remount(this);}
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
 auto* Person=GetWorld()->SpawnActor<ABattleRider>(Exit,GetActorRotation(),P);if(!Person)return false;
 Ride->BoostRemaining=0;Ride->Speed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->StopMovementImmediately();Ride->DisableMovement();bParked=true;Visual->SetRelativeRotation(FRotator::ZeroRotator);Rider->SetVisibility(false,true);
 ReloadTimer=0;LeanAngle=0;Ride->SmoothedSteer=0;Person->ParkedBike=this;Person->Health=RiderHealth;Person->RestoreLoadout();Person->GetCapsuleComponent()->IgnoreActorWhenMoving(this,true);PC->Possess(Person);PC->SetControlRotation(GetActorRotation());return true;
}
bool ABattleBike::Remount(ABattleRider* Person){
 if(bCrashActive||StunRemaining>0||!IsValid(Person)||Person->ParkedBike!=this)return false;
 if(IsValid(PlayerCrash)&&!PlayerCrash->PrepareRemount(Person))return false;
 if(StunRemaining>0||!IsValid(Person)||Person->ParkedBike!=this||!bParked||FVector::Dist(GetActorLocation(),Person->GetActorLocation())>240)return false;
 auto* PC=Cast<APlayerController>(Person->GetController());if(!PC)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleRemount),false,this);Q.AddIgnoredActor(Person);
 if(GetWorld()->OverlapBlockingTestByChannel(GetActorLocation(),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q))return false;
 Person->SaveWeapon();ABattleKnife::OnRemounted(this);bParked=false;ClearPhysicalCrash();Visual->SetRelativeRotation(FRotator::ZeroRotator);Rider->SetRelativeLocation(FVector::ZeroVector);Ride->SetMovementMode(MOVE_Walking);Ride->Speed=0;Rider->SetVisibility(!bFirstPerson,true);PC->Possess(this);PC->SetControlRotation(GetActorRotation());Person->Destroy();return true;
}

bool ABattleRider::Fire(){
 if(!CanUseWeapon()||!bWeaponDrawn||DrawRemaining>0||MeleeRemaining>0||ShotCooldown>0||ReloadRemaining>0||Ammo<=0)return false;
 if(CurrentWeapon==3){if(!ABattleDisc::Launch(this,LongGun))return false;Ammo--;ShotsFired++;ShotCooldown=.55f;Kick=.7f;SaveWeapon();return true;}
 if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))Mode->RecordGunfire();
 Ammo--;ShotsFired++;ShotCooldown=CurrentWeapon==1?.85f:CurrentWeapon==2?.085f:CurrentWeapon==4?.12f:.22f;Kick=CurrentWeapon==1?2:1;SaveWeapon();
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
