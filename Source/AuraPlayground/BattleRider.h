#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BattleRider.generated.h"
class ABattleBike;
UCLASS()
class AURAPLAYGROUND_API ABattleRider : public APiedmontExplorer {
 GENERATED_BODY()
public:
 ABattleRider();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPoseableMeshComponent> FirstPersonArms;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector GunRestPosition=FVector(65,18,-24);
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector GunAimPosition=FVector(65,0,-11.5f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector RightWristOffset=FVector(-9,4,-3);
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector LeftWristOffset=FVector(-10,-4,-5);
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float HandScale=1.f;
 UPROPERTY(BlueprintReadOnly) FVector RightGrip;
 UPROPERTY(BlueprintReadOnly) FVector LeftGrip;
 UFUNCTION(BlueprintCallable) bool ToggleDrawWeapon();
 UPROPERTY(BlueprintReadOnly) float DrawRemaining=0;
 virtual bool Fire() override;
 virtual void Reload() override;
 UFUNCTION(BlueprintCallable) bool SelectWeapon(int32 Slot);
 UPROPERTY(BlueprintReadOnly) int32 CurrentWeapon=0;
 void SaveWeapon();
 void RestoreLoadout();
 FString ReserveLabel() const;
 void UpdateWeaponModel();
 UPROPERTY() TObjectPtr<USceneComponent> LongGun;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> RifleMesh;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> LongGunParts;
 UFUNCTION(BlueprintCallable) bool Melee();
 UPROPERTY(BlueprintReadOnly) int32 MeleeSwings=0,MeleeHits=0;
 UPROPERTY(BlueprintReadOnly) float MeleeRemaining=0;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> MeleeRoot;
 UPROPERTY(BlueprintReadOnly) float HitFeedback=0;
 virtual void BeginPlay() override;
 virtual void OnStartCrouch(float HalfHeightAdjust,float ScaledHalfHeightAdjust) override;
 virtual void OnEndCrouch(float HalfHeightAdjust,float ScaledHalfHeightAdjust) override;
 virtual void Tick(float Dt) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<ABattleBike> ParkedBike;
 UPROPERTY(BlueprintReadOnly) float Health=100;
 UFUNCTION(BlueprintCallable) bool MountBike();
protected:
 virtual float AimedFieldOfView() const override{return CurrentWeapon==4?35.f:65.f;}
 virtual bool CanUseWeapon() const override;
 virtual void FinishReload() override;
private:
 void ReloadPistol(){Reload();}
 void DrawWeapon(){ToggleDrawWeapon();}
 void StartCrouch(){if(Health>0&&!bSwimming)Crouch();}void EndCrouch(){UnCrouch();}
 void ToggleCrouch(){if(GetCharacterMovement()->bWantsToCrouch)UnCrouch();else StartCrouch();}
 void SelectPistol(){SelectWeapon(0);}void SelectShotgun(){SelectWeapon(1);}void SelectSMG(){SelectWeapon(2);}void SelectFrisbee(){SelectWeapon(3);}void SelectRifle(){SelectWeapon(4);}
 void BuildLongGun();
 void StartMelee(){Melee();}
 void BuildMeleeVisual();
 void UpdateMelee(float Dt);
 void ResolveMelee();
 bool bMeleeResolved=false;
 void Interact(){MountBike();}
 void StartJump(){Jump();}void EndJump(){StopJumping();}
 float ShotCooldown=0,Kick=0;
 FRotator GunRestRotation;
 void PoseArms(float Dt);
 void InitializeDetailedArmsPreview();
 TArray<FTransform> ArmRest;TArray<int32> ArmParents;TArray<FName> ArmNames;
 float SwayTime=0;
 float AimBlend=0,ArmPoseBlend=0,EmptyArmMotion=0;
};
