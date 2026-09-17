#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleScooterTraffic.generated.h"
class APiedmontPathSpline;

UCLASS()
class AURAPLAYGROUND_API ABattleScooterTraffic : public AActor {
 GENERATED_BODY()
public:
 ABattleScooterTraffic();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) int32 SpawnedScooters=0;
 UPROPERTY(BlueprintReadOnly) int32 PlayerContacts=0;
private:
 struct FMovingScooter {TWeakObjectPtr<AActor> Actor;TWeakObjectPtr<APiedmontPathSpline> Path;float Distance=0,Speed=350;bool Reverse=false;};
 TArray<FMovingScooter> Scooters;
 UPROPERTY() TSubclassOf<AActor> ScooterClass;
 float ContactCooldown=0;
};
