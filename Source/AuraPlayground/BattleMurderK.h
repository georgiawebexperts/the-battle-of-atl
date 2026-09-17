#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMurderK.generated.h"
class UStaticMeshComponent;
class UTextRenderComponent;

// Fictional trail-side grocery landmark and plaza at the real 725 Ponce checkpoint.
UCLASS()
class AURAPLAYGROUND_API ABattleMurderK : public AActor {
 GENERATED_BODY()
public:
 ABattleMurderK();
 virtual void BeginPlay() override;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> StoreSign;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> GraffitiSign;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> StoreMass;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> OfficeTower;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GlassWing;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> TrailApron;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BrickParts;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> DarkParts;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> RedParts;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> GlassParts;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> ConcreteParts;
};
