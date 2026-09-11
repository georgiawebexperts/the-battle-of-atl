#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleSkatepark.generated.h"
class UInstancedStaticMeshComponent;class UStaticMeshComponent;class ABattleColaPickup;
UCLASS()
class AURAPLAYGROUND_API ABattleSkatepark:public AActor {
 GENERATED_BODY()
public:
 ABattleSkatepark();
 virtual void BeginPlay() override;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Concrete;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Berm;
 UPROPERTY() TArray<TObjectPtr<ABattleColaPickup>> Bonuses;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Metal;
};
