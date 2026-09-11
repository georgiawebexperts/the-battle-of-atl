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
 FVector DiveStart,DiveTarget,EscapeDirection;
 TArray<TObjectPtr<UStaticMeshComponent>> Rotors;
};
