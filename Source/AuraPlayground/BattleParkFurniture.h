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
 virtual void Tick(float Dt) override;
 UPROPERTY(EditAnywhere) bool bAmbientBenchFire=true;
 UPROPERTY(EditAnywhere,meta=(ClampMin="0",ClampMax="1")) float BenchFireRunChance=1.f;
 UPROPERTY(EditAnywhere,meta=(ClampMin="0")) int32 TargetBenchFireEncounters=2;
 UPROPERTY(BlueprintReadOnly) int32 BenchEncounterAttempts=0;
 UPROPERTY(BlueprintReadOnly) int32 BenchFireEncountersCompleted=0;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Wood;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Frame;
 TArray<FTransform> Benches;
 void AddBench(const FTransform& Transform);
 bool IsBenchAvailable(int32 Index) const;
 bool ReserveBench(int32 Index,AActor* Claimant);
 void ReleaseBench(int32 Index,AActor* Claimant);
private:
 TMap<int32,TWeakObjectPtr<AActor>> Reservations;
 TWeakObjectPtr<class APiedmontPedestrian> EncounterVisitor;
 int32 EncounterBench=INDEX_NONE,EncounterRun=INDEX_NONE;
 float EncounterEligibleTime=0,EncounterWait=0;
 bool bEncounterRolled=false,bEncounterAllowed=false;
};
