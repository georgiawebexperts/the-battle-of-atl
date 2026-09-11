#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "PiedmontThreat.generated.h"
UCLASS()
class AURAPLAYGROUND_API APiedmontThreat : public APiedmontExplorer {
 GENERATED_BODY()
public:
 APiedmontThreat();
 virtual void BeginPlay() override;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Blade;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPROPERTY(BlueprintReadOnly) bool bGunman=false;
 UPROPERTY(BlueprintReadOnly) bool bWindingUp=false;
 UPROPERTY(BlueprintReadOnly) int32 Attacks=0;
 UPROPERTY(BlueprintReadOnly) float Health=34;
protected:
 virtual bool CanUseWeapon() const override;
private:
 float AttackCooldown=1.5f,Windup=0,DeadTime=0;
 FVector AimPoint;
};
