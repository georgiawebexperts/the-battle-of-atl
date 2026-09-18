#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleDrone.generated.h"
class UStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleDrone : public AActor {
 GENERATED_BODY()
public:
 ABattleDrone();
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 bool bWarning=true,bSpent=false;
 float Clock=0,Health=40;
 int32 RiderHits=0;
 /** Why the dive ended. "blocked by <actor>" and "struck the bike" look the same
  *  from outside - both leave bSpent set and RiderHits at whatever it was - and
  *  telling them apart is the whole question when a drone quietly does nothing. */
 FString EndReason;
 FVector DiveStart,DiveTarget,EscapeDirection;
 TArray<TObjectPtr<UStaticMeshComponent>> Rotors;
};
