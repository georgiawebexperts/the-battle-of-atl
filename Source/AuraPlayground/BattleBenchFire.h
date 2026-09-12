#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleBenchFire.generated.h"
class UMaterialBillboardComponent;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class ABattleParkFurniture;
// Visual component of a bench encounter; spawning/ignition policy is separate.
UCLASS()
class AURAPLAYGROUND_API ABattleBenchFire : public AActor {
 GENERATED_BODY()
public:
 ABattleBenchFire();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 static ABattleBenchFire* IgniteBench(ABattleParkFurniture* Furniture,int32 Index);
 UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<UMaterialBillboardComponent>> Flames;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UMaterialBillboardComponent> Smoke;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> Glow;
 UPROPERTY(EditAnywhere) float Duration=35.f;
private:
 UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> FlameMaterials;
 UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SmokeMaterial;
 float Age=0;
 TWeakObjectPtr<ABattleParkFurniture> ReservedFurniture;
 int32 ReservedBench=INDEX_NONE;
};
