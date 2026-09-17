#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattleKnife.generated.h"
class ABattleBike;class USkeletalMeshComponent;class UPhysicsAsset;
UCLASS()
class AURAPLAYGROUND_API ABattleKnife : public APiedmontExplorer {
 GENERATED_BODY()
public:
 ABattleKnife();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 bool ResolveStrike();
 void Escape();
 static void OnRemounted(ABattleBike* Bike);
 static float SpawnChance(bool HasPhone,float Trouble);
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Blade;
 UPROPERTY() TObjectPtr<USkeletalMeshComponent> DeathPhysics;
 UPROPERTY() TObjectPtr<UPhysicsAsset> DeathAsset;
 UPROPERTY() TWeakObjectPtr<ABattleBike> VictimBike;
 float Health=100,Cooldown=1.5f,WindupRemaining=0,StrikePose=0,PathDelay=0,FarTime=0,DeathTime=0;
 int32 Stabs=0,PathRequests=0;
 bool bWindingUp=false,bEscaped=false;
 bool bSingleLunge=false;
protected:
 virtual bool CanUseWeapon() const override{return false;}
 virtual FVector AdjustVisitorHand(int32 Side,FVector Target) const override;
private:
 bool CanReach(APawn* Target) const;
 bool BeginDeathPhysics(FVector Direction);
 void MirrorDeathPose();
};
