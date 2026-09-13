#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattleGunman.generated.h"
class USkeletalMeshComponent;
class UPhysicsAsset;
UCLASS()
class AURAPLAYGROUND_API ABattleGunman : public APiedmontExplorer {
 GENERATED_BODY()
public:
 ABattleGunman();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 bool TryAim(APawn* Target);
 bool ResolveShot();
 UPROPERTY(BlueprintReadOnly) bool bWarning=false;
 UPROPERTY(BlueprintReadOnly) float WindupRemaining=0;
 UPROPERTY(BlueprintReadOnly) float ShotAlertRemaining=0;
 float Health=80,Cooldown=2;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USkeletalMeshComponent> DeathPhysics;
 UPROPERTY() TObjectPtr<UPhysicsAsset> DeathAsset;
 UPROPERTY() TObjectPtr<UAnimSequence> GunIdle;
 UPROPERTY() TObjectPtr<UAnimSequence> RelaxedIdle;
 UPROPERTY() TObjectPtr<UAnimSequence> GunWalk;
 UPROPERTY() TObjectPtr<UAnimSequence> GunRun;
private:
 FVector AimPoint;
 TWeakObjectPtr<APawn> AimTarget;
 bool CanAttack() const;
 bool BeginDeathPhysics(FVector Direction);
 void MirrorDeathPose();
protected:
 virtual void AnimateBody(float Dt) override;
 virtual bool CanUseWeapon() const override{return !bDead;}
};
