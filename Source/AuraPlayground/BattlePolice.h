#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattlePolice.generated.h"
UCLASS()
class AURAPLAYGROUND_API ABattlePolice : public APiedmontExplorer {
 GENERATED_BODY()
public:
 ABattlePolice();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 bool FireTaser();
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UTextRenderComponent> TaserWarning;
 UPROPERTY(BlueprintReadOnly) float Health=100;
 UPROPERTY(BlueprintReadOnly) bool bWarning=false;
 UPROPERTY(BlueprintReadOnly) int32 TaserShots=0;
 float WarningRemaining=0,Cooldown=3,PathDelay=0;
protected:
 virtual bool CanUseWeapon() const override{return false;}
private:
 bool CanReachTarget(APawn* Target) const;
};
