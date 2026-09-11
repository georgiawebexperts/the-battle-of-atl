#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PiedmontExplorer.generated.h"
class APiedmontBike;
class UAnimSequence;
class UPoseableMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API APiedmontExplorer : public ACharacter {
 GENERATED_BODY()
public:
 APiedmontExplorer();
 virtual void BeginPlay() override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 virtual void Tick(float DeltaSeconds) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<APiedmontBike> Bike;
 UPROPERTY(BlueprintReadOnly) bool bSwimming=false;
 UPROPERTY(BlueprintReadOnly) bool bAuthoredLocomotion=false;
 UPROPERTY(BlueprintReadOnly) bool bWeaponDrawn=false;
 UPROPERTY(BlueprintReadOnly) bool bAiming=false;
 UPROPERTY(BlueprintReadOnly) bool bDead=false;
 UPROPERTY(BlueprintReadOnly) bool bKnifeWounded=false;
 UPROPERTY(BlueprintReadOnly) int32 Ammo=12;
 UPROPERTY(BlueprintReadOnly) int32 ShotsFired=0;
 UPROPERTY(BlueprintReadOnly) float ReloadRemaining=0;
 UPROPERTY(BlueprintReadOnly) FVector LastShotEnd;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Weapon;
 UFUNCTION(BlueprintCallable) void ToggleWeapon();
 UFUNCTION(BlueprintCallable) virtual bool Fire();
 UFUNCTION(BlueprintCallable) void AimAtForValidation(AActor* Target);
 UFUNCTION(BlueprintCallable) virtual void Reload();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPoseableMeshComponent> Body;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> CameraArm;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
 UFUNCTION(BlueprintCallable) bool Remount();
 UFUNCTION(BlueprintCallable) void ValidationKey(FName Key,bool Pressed);
private:
 void Interact();
 void PullTrigger();void ReleaseTrigger();void AimOn();void AimOff();
protected:
 virtual float AimedFieldOfView() const{return 65.f;}
 void SetLocomotionClips(UAnimSequence* Idle,UAnimSequence* Walk,UAnimSequence* Run);
 virtual bool CanUseWeapon() const;
 virtual FVector AdjustVisitorHand(int32 Side,FVector Target) const {return Target;}
 virtual void FinishReload(){Ammo=12;}
private:
 bool bTriggerHeld=false;
 float FireCooldown=0;
protected:
 virtual void AnimateBody(float Dt);
private:
 void UpdateSwimming(float Dt);
 TArray<FTransform> RestPose;
 TArray<int32> Parents;
 TArray<FName> Bones;
 float Gait=0;
 UPROPERTY() TObjectPtr<UAnimSequence> IdleAnimation;
 UPROPERTY() TObjectPtr<UAnimSequence> WalkAnimation;
 UPROPERTY() TObjectPtr<UAnimSequence> RunAnimation;
 bool SampleLocomotion(float Dt,TArray<FTransform>& Pose);
 float LocomotionPhase=0,LocomotionSpeed=0,IdleClock=0,GroundPoseOffset=0;
};
