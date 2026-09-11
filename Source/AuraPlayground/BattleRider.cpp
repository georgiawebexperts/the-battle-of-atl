#include "BattleRider.h"
#include "BattleShot.h"
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
 GetCharacterMovement()->MaxWalkSpeed=520;GetCharacterMovement()->JumpZVelocity=560;GetCharacterMovement()->AirControl=.8f;GetCharacterMovement()->GravityScale=1.4f;
 Weapon->SetupAttachment(Camera);bWeaponDrawn=true;
}
void ABattleRider::BeginPlay(){
 Super::BeginPlay();Weapon->SetRelativeLocation(FVector(34,16,-17));Weapon->SetVisibility(true);GunRestRotation=Weapon->GetRelativeRotation();
 if(IsValid(ParkedBike))GetCapsuleComponent()->IgnoreActorWhenMoving(ParkedBike,true);
}
void ABattleRider::SetupPlayerInputComponent(UInputComponent* I){
 ACharacter::SetupPlayerInputComponent(I);
 I->BindKey(EKeys::E,IE_Pressed,this,&ABattleRider::Interact);
 I->BindKey(EKeys::SpaceBar,IE_Pressed,this,&ABattleRider::StartJump);I->BindKey(EKeys::SpaceBar,IE_Released,this,&ABattleRider::EndJump);
 I->BindKey(EKeys::R,IE_Pressed,this,&ABattleRider::ReloadPistol);
}
bool ABattleRider::CanUseWeapon() const{const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));return Health>0&&!bSwimming&&GetController()&&(!Mode||(Mode->StartCountdown<=0&&!Mode->bRunEnded));}
void ABattleRider::Tick(float Dt){
 ShotCooldown=FMath::Max(0.f,ShotCooldown-Dt);HitFeedback=FMath::Max(0.f,HitFeedback-Dt);Kick=FMath::FInterpTo(Kick,0.f,Dt,14);
 Weapon->SetRelativeLocation(FVector(34-4*Kick,16,-17-Kick));Weapon->SetRelativeRotation(GunRestRotation+FRotator(5*Kick,0,0));
 bWeaponDrawn=CanUseWeapon();
 if(auto* PC=Cast<APlayerController>(GetController())){
  GetCharacterMovement()->MaxWalkSpeed=PC->IsInputKeyDown(EKeys::LeftShift)?850:520;
  bAiming=PC->IsInputKeyDown(EKeys::RightMouseButton)&&bWeaponDrawn;
  if(PC->IsInputKeyDown(EKeys::LeftMouseButton))Fire();
 }
 Super::Tick(Dt);HurtCooldown=FMath::Max(0.f,HurtCooldown-Dt);if(HurtCooldown<=0&&Health>0)Health=FMath::Min(100.f,Health+4*Dt);
}
bool ABattleRider::MountBike(){return Health>0&&!bSwimming&&IsValid(ParkedBike)&&ParkedBike->Remount(this);}
float ABattleRider::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(Amount<=0||Health<=0)return 0;const float Applied=FMath::Min(Health,Amount);Health-=Applied;HurtCooldown=5;
 // Lab recovery; persistent route checkpoints are implemented with the game loop.
 if(Health<=0){
  if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))Mode->TimeRemaining=FMath::Max(0.f,Mode->TimeRemaining-10);
  Health=100;SetActorLocation(IsValid(ParkedBike)?ParkedBike->GetActorLocation()+FVector(0,160,0):FVector(-7000,4000,100),false,nullptr,ETeleportType::TeleportPhysics);
 }
 return Applied;
}
bool ABattleBike::Dismount(){
 auto* PC=Cast<APlayerController>(GetController());if(!PC||bParked||Ride->Recovery>0)return false;
 FVector Exit;bool Found=false;FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleDismount),false,this);
 for(const FVector Direction:{GetActorRightVector(),-GetActorRightVector(),-GetActorForwardVector(),GetActorForwardVector()}){
  const FVector Candidate=GetActorLocation()+Direction*145;FHitResult Ground;
  if(!GetWorld()->LineTraceSingleByChannel(Ground,Candidate+FVector(0,0,100),Candidate-FVector(0,0,220),ECC_Visibility,Q)||Ground.ImpactNormal.Z<.65f)continue;
  Exit=Ground.ImpactPoint+FVector(0,0,90);
  FHitResult Wall;if(GetWorld()->SweepSingleByChannel(Wall,GetActorLocation(),Exit,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q))continue;
  if(!GetWorld()->OverlapBlockingTestByChannel(Exit,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(30,88),Q)){Found=true;break;}
 }
 if(!Found)return false;
 FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
 auto* Person=GetWorld()->SpawnActor<ABattleRider>(Exit,GetActorRotation(),P);if(!Person)return false;
 Ride->BoostRemaining=0;Ride->Speed=Ride->Pedal=Ride->Steer=Ride->Brake=0;Ride->StopMovementImmediately();Ride->DisableMovement();bParked=true;Visual->SetRelativeRotation(FRotator::ZeroRotator);Rider->SetVisibility(false);
 Person->ParkedBike=this;Person->Health=RiderHealth;Person->Ammo=PistolAmmo;Person->GetCapsuleComponent()->IgnoreActorWhenMoving(this,true);PC->Possess(Person);PC->SetControlRotation(GetActorRotation());return true;
}
bool ABattleBike::Remount(ABattleRider* Person){
 if(!IsValid(Person)||Person->ParkedBike!=this||!bParked||FVector::Dist(GetActorLocation(),Person->GetActorLocation())>240)return false;
 auto* PC=Cast<APlayerController>(Person->GetController());if(!PC)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(BattleRemount),false,this);Q.AddIgnoredActor(Person);
 if(GetWorld()->OverlapBlockingTestByChannel(GetActorLocation(),FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,95),Q))return false;
 RiderHealth=Person->Health;PistolAmmo=Person->Ammo;bParked=false;Ride->SetMovementMode(MOVE_Walking);Ride->Speed=0;Rider->SetVisibility(!bFirstPerson);PC->Possess(this);PC->SetControlRotation(GetActorRotation());Person->Destroy();return true;
}

bool ABattleRider::Fire(){
 if(!CanUseWeapon()||!bWeaponDrawn||ShotCooldown>0||ReloadRemaining>0||Ammo<=0)return false;
 Ammo--;ShotsFired++;ShotCooldown=.22f;Kick=1;
 const auto Shot=FireBattlePistol(this,Weapon,bAiming?.1f:.4f);LastShotEnd=Shot.End;if(Shot.Damage>0)HitFeedback=.2f;
 if(Shot.EnemyKilled&&IsValid(ParkedBike))ParkedBike->AwardEnemyKill();return true;
}
