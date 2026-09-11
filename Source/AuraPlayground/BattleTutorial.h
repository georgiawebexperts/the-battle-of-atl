#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleTutorial.generated.h"
UCLASS()
class AURAPLAYGROUND_API ABattleTutorial : public AActor {
 GENERATED_BODY()
public:
 ABattleTutorial();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 bool TryStart(FVector Previous,FVector Current);
 FVector PreviousPosition=FVector::ZeroVector;
};
