#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleParkFurniture.generated.h"
class UInstancedStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleParkFurniture : public AActor {
 GENERATED_BODY()
public:
 ABattleParkFurniture();
 virtual void BeginPlay() override;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Wood;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Frame;
 TArray<FTransform> Benches;
 void AddBench(const FTransform& Transform);
 bool IsBenchAvailable(int32 Index) const;
 bool ReserveBench(int32 Index,AActor* Claimant);
 void ReleaseBench(int32 Index,AActor* Claimant);
private:
 TMap<int32,TWeakObjectPtr<AActor>> Reservations;
};
