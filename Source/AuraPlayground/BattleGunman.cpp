#include "BattleGunman.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleShot.h"
#include "PiedmontBlood.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimSequence.h"
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
 const bool Drawn=bWeaponDrawn;const TArray<FTransform> Previous=Body->BoneSpaceTransforms;
 SetLocomotionClips(Drawn&&GunIdle?GunIdle:RelaxedIdle,GunWalk,GunRun);
 // Keep the authored gun stance; the generic hand IK otherwise overwrites it.
 if(GunIdle)bWeaponDrawn=false;
 Super::AnimateBody(Dt);bWeaponDrawn=Drawn;
 if(Previous.Num()==Body->BoneSpaceTransforms.Num()&&Dt>0){
  const float Weight=1.f-FMath::Exp(-14.f*Dt);
  for(int I=0;I<Previous.Num();I++){FTransform Blended;Blended.Blend(Previous[I],Body->BoneSpaceTransforms[I],Weight);Body->BoneSpaceTransforms[I]=Blended;}
  Body->MarkRefreshTransformDirty();Body->RefreshBoneTransforms();
 }
 if(Drawn&&Weapon->GetStaticMesh()){
  const FVector Centre=Body->GetSocketLocation(TEXT("Hand_R"))+GetActorForwardVector()*8+FVector(0,0,6);
  Weapon->SetWorldLocation(Centre-Weapon->GetComponentTransform().TransformVector(Weapon->GetStaticMesh()->GetBounds().Origin));
 }
}
