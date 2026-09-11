#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleDisc.generated.h"
class ABattleBike;class UStaticMeshComponent;class APawn;
UCLASS()
class AURAPLAYGROUND_API ABattleDisc : public AActor {
 GENERATED_BODY()
public:
 ABattleDisc();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) FVector Velocity=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) int32 Bounces=0,Hits=0,Kills=0;
 UPROPERTY() TObjectPtr<ABattleBike> Bike;
 UPROPERTY() TWeakObjectPtr<APawn> LaunchPawn;
 static ABattleDisc* Launch(APawn* Shooter,USceneComponent* Gun);
private:
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Disc;
 TArray<TWeakObjectPtr<AActor>> Struck;
 float Age=0;
};
