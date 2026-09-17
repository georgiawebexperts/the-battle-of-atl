#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleSkyline.generated.h"

class UInstancedStaticMeshComponent;

/**
 * The downtown Atlanta skyline on the south-west horizon. The terrain only
 * reaches so far, so the cluster is a scale model placed for the correct
 * apparent size from the park rather than at true 1:3 distance.
 */
UCLASS()
class AURAPLAYGROUND_API ABattleSkyline : public AActor {
 GENERATED_BODY()
public:
 ABattleSkyline();
 virtual void BeginPlay() override;
 UPROPERTY(BlueprintReadOnly) FVector Centre=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) int32 Towers=0;
 UPROPERTY(BlueprintReadOnly) float TallestM=0;
private:
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Blocks;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Crown;
};
