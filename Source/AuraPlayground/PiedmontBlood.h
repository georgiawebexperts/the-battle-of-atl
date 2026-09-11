#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PiedmontBlood.generated.h"
class UStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API APiedmontBlood : public AActor {
 GENERATED_BODY()
public:
 APiedmontBlood();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 static void Burst(UWorld* World,FVector Location,FVector Direction);
 FVector SprayDirection=FVector::UpVector;
private:
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Drops;
 TArray<FVector> Velocities;
};
