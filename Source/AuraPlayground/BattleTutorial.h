#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleTutorial.generated.h"
class USceneComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleTutorial : public AActor {
 GENERATED_BODY()
public:
 ABattleTutorial();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 bool TryStart(FVector Previous,FVector Current);
 UPROPERTY() TArray<TObjectPtr<USceneComponent>> BoundaryRoots;
 FVector PreviousPosition=FVector::ZeroVector;
};
