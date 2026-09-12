#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattleGunman.generated.h"
UCLASS()
class AURAPLAYGROUND_API ABattleGunman : public APiedmontExplorer {
 GENERATED_BODY()
public:
 ABattleGunman();
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 bool TryAim(APawn* Target);
 bool ResolveShot();
 UPROPERTY(BlueprintReadOnly) bool bWarning=false;
 UPROPERTY(BlueprintReadOnly) float WindupRemaining=0;
 float Health=80,Cooldown=2;
private:
 FVector AimPoint;
 TWeakObjectPtr<APawn> AimTarget;
 bool CanAttack() const;
protected:
 virtual bool CanUseWeapon() const override{return !bDead;}
};
