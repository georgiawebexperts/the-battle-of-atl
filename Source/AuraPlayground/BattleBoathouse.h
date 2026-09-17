#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleBoathouse.generated.h"

class UInstancedStaticMeshComponent;
class UTextRenderComponent;

/**
 * Lake Clara Meer boathouse and dock. The lake outline already exists as an
 * OSM-derived water hazard, so the building is placed against the real shore
 * rather than at a hand-typed coordinate: the hull is on dry land, the dock
 * runs out over water, and both facts are checked at spawn time.
 */
UCLASS()
class AURAPLAYGROUND_API ABattleBoathouse : public AActor {
 GENERATED_BODY()
public:
 ABattleBoathouse();
 virtual void BeginPlay() override;
 /** Shore point the building was placed against, in world space. */
 UPROPERTY(BlueprintReadOnly) FVector ShorePoint=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) FVector DockTip=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) float GroundZ=0;
 UPROPERTY(BlueprintReadOnly) float WaterZ=0;
 UPROPERTY(BlueprintReadOnly) bool bPlaced=false;
 UPROPERTY(BlueprintReadOnly) bool bDockOverWater=false;
 UPROPERTY(BlueprintReadOnly) bool bHullOnLand=false;
private:
 bool FindShore(FVector& OutShore,FVector& OutToWater,float& OutWaterZ) const;
 void Build();
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Stone;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Siding;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Trim;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Openings;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Deck;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Railings;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Piles;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UInstancedStaticMeshComponent> Roof;
 UPROPERTY() TObjectPtr<UTextRenderComponent> Sign;
};
