#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleDuck.generated.h"

class UStaticMeshComponent;

/**
 * Lake Clara Meer ducks. They fly in, settle on the open water, drift, and take
 * off again when a rider crowds them. A swimmer who blunders into one gets
 * dunked and loses time, which is the behaviour the design asks for.
 */
UCLASS()
class AURAPLAYGROUND_API ABattleDuck : public AActor {
 GENERATED_BODY()
public:
 ABattleDuck();
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 /** Send the bird to a landing spot on the water. */
 void FlyTo(const FVector& WaterPoint,float Delay);
 UPROPERTY(BlueprintReadOnly) float WaterZ=0;
 UPROPERTY(BlueprintReadOnly) int32 State=0;      // 0 approach, 1 swim, 2 climb out
 UPROPERTY(BlueprintReadOnly) int32 BumpsGiven=0;
private:
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Head;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Beak;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Tail;
 FVector Target=FVector::ZeroVector;
 float Delay=0,Clock=0,Drift=0,BumpCooldown=0;
 FRotator BaseRotation;
};

/** Spawns and keeps a small raft of ducks on the lake. */
UCLASS()
class AURAPLAYGROUND_API ABattleDuckFlock : public AActor {
 GENERATED_BODY()
public:
 ABattleDuckFlock();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<ABattleDuck>> Ducks;
 UPROPERTY(BlueprintReadOnly) int32 LandingPoints=0;
private:
 FVector WaterCentre=FVector::ZeroVector;
 float WaterZ=0,SpawnClock=0;
 TArray<FVector> Spots;
};
