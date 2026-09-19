#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleGhostRider.generated.h"
class USceneComponent;class UStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleGhostRider : public AActor {
 GENERATED_BODY()
public:
 ABattleGhostRider();
 void LoadRoute(const TArray<FVector>& Samples,float Stride);
 void Advance(float Elapsed);
 UPROPERTY() TObjectPtr<USceneComponent> Visual;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> FrontWheel;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> RearWheel;
 bool bLoaded=false;
 TArray<FVector> Samples;float Stride=.5f;float WheelAngle=0;FVector LastPos;
};