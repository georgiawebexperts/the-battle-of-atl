#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleTrafficSignal.generated.h"
class ABattleRoadCrossing;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
UCLASS()
class AURAPLAYGROUND_API ABattleTrafficSignal : public AActor {
 GENERATED_BODY()
public:
 ABattleTrafficSignal();
 virtual void BeginPlay() override;
 virtual void Tick(float DeltaSeconds) override;
 UPROPERTY(EditAnywhere) TObjectPtr<ABattleRoadCrossing> Crossing;
 UPROPERTY(VisibleAnywhere) TArray<UStaticMeshComponent*> Lenses;
private:
 UPROPERTY(Transient) TArray<UMaterialInstanceDynamic*> LensMaterials;
 void UpdateLamps();
};
