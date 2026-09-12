#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleBenchFire.generated.h"
class UMaterialBillboardComponent;
class UMaterialInstanceDynamic;
class UPointLightComponent;
// Visual component of a bench encounter; spawning/ignition policy is separate.
UCLASS()
class AURAPLAYGROUND_API ABattleBenchFire : public AActor {
 GENERATED_BODY()
public:
 ABattleBenchFire();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<UMaterialBillboardComponent>> Flames;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UMaterialBillboardComponent> Smoke;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> Glow;
 UPROPERTY(EditAnywhere) float Duration=35.f;
private:
 UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> FlameMaterials;
 UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SmokeMaterial;
 float Age=0;
};
