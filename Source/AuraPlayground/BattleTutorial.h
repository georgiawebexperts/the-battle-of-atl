#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleTutorial.generated.h"
class USceneComponent;
class UInstancedStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleTutorial : public AActor {
 GENERATED_BODY()
public:
 ABattleTutorial();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 bool TryStart(FVector Previous,FVector Current);
 UPROPERTY() TArray<TObjectPtr<USceneComponent>> BoundaryRoots;
 UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> PracticeStreet;
 FVector PreviousPosition=FVector::ZeroVector;
};
