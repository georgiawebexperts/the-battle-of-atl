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
 virtual bool Fire() override;
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
 void ReloadPistol(){Reload();}
 void Interact(){MountBike();}
 void StartJump(){Jump();}void EndJump(){StopJumping();}
 float HurtCooldown=0,ShotCooldown=0,Kick=0;
 FRotator GunRestRotation;
};
