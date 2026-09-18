#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PiedmontPedestrian.h"
#include "BattleHints.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Trees ATL: ride the grass on the BeltLine stretch and the crew working on it
 * comes after the rider for a few seconds.
 */
UCLASS()
class AURAPLAYGROUND_API ABattleGrassWatch : public AActor {
 GENERATED_BODY()
public:
 ABattleGrassWatch();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) int32 Chases=0,Pursuers=0;
private:
 struct FPursuer{TWeakObjectPtr<APiedmontPedestrian> Actor;float Remaining=0;};
 TArray<FPursuer> Crew;
 float Cooldown=4.f;
};

/**
 * Roadside board before the BeltLine: TREES ATL working on the grass.
 */
UCLASS()
class AURAPLAYGROUND_API ABattleGrassSign : public AActor {
 GENERATED_BODY()
public:
 ABattleGrassSign();
 virtual void BeginPlay() override;
 UPROPERTY(BlueprintReadOnly) FString Message;
 UPROPERTY(BlueprintReadOnly) FVector Place=FVector::ZeroVector;
private:
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Root;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Post;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Board;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Face;
};
