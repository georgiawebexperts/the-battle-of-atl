#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattleRider.generated.h"
class ABattleBike;
UCLASS()
class AURAPLAYGROUND_API ABattleRider : public APiedmontExplorer {
 GENERATED_BODY()
public:
 ABattleRider();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPoseableMeshComponent> FirstPersonArms;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector GunRestPosition=FVector(55,12,-10);
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector RightWristOffset=FVector(-9,4,-3);
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector LeftWristOffset=FVector(-10,-4,-5);
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float HandScale=.65f;
 UPROPERTY(BlueprintReadOnly) FVector RightGrip;
 UPROPERTY(BlueprintReadOnly) FVector LeftGrip;
 virtual bool Fire() override;
 UFUNCTION(BlueprintCallable) bool Melee();
 UPROPERTY(BlueprintReadOnly) int32 MeleeSwings=0,MeleeHits=0;
 UPROPERTY(BlueprintReadOnly) float MeleeRemaining=0;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> MeleeRoot;
 UPROPERTY(BlueprintReadOnly) float HitFeedback=0;
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<ABattleBike> ParkedBike;
 UPROPERTY(BlueprintReadOnly) float Health=100;
 UFUNCTION(BlueprintCallable) bool MountBike();
protected:
 virtual bool CanUseWeapon() const override;
private:
 void ReloadPistol(){if(MeleeRemaining<=0)Reload();}
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
 TArray<FTransform> ArmRest;TArray<int32> ArmParents;TArray<FName> ArmNames;
 float SwayTime=0;
};
