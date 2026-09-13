#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleRoadCar.h"
#include "BattleRoadTrafficDirector.generated.h"
USTRUCT()
struct FBattleRoadTrafficLane {
 GENERATED_BODY()
 UPROPERTY(EditAnywhere) FName Name;
 UPROPERTY(EditAnywhere) TArray<FVector> Points;
 UPROPERTY(EditAnywhere) TArray<FBattleCarCrossing> Crossings;
 UPROPERTY(EditAnywhere) float CruiseSpeed=650.f;
 UPROPERTY(EditAnywhere) bool bLoopRoute=false;
};
UCLASS()
class AURAPLAYGROUND_API ABattleRoadTrafficDirector : public AActor {
 GENERATED_BODY()
public:
 ABattleRoadTrafficDirector();
 virtual void Tick(float DeltaSeconds) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(EditAnywhere) TArray<FBattleRoadTrafficLane> Lanes;
 UPROPERTY(EditAnywhere) int32 MaxCars=6;
 UPROPERTY(EditAnywhere) int32 MaxCarsPerLane=3;
 UPROPERTY(EditAnywhere) float SpawnInterval=8.f;
 UPROPERTY(BlueprintReadOnly) int32 LiveCars=0;
 UPROPERTY(BlueprintReadOnly) int32 TotalSpawned=0;
 UPROPERTY(BlueprintReadOnly) int32 TotalRemoved=0;
 UPROPERTY(BlueprintReadOnly) int32 PeakCars=0;
 bool IsUnobserved(FVector Location) const;
 ABattleRoadCar* TrySpawnLane(int32 LaneIndex);
private:
 struct FCar {TWeakObjectPtr<ABattleRoadCar> Actor;int32 Lane=0;};
 TArray<FCar> Cars;
 TArray<float> Cooldowns;
};
