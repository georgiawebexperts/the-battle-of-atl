#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleKrogCrash.generated.h"

class UBoxComponent;
class APiedmontPedestrian;

/**
 * The crash on the Krog tunnel approach: a scooter wreck that blocks the crown
 * of the road just before the mouth, burning the whole run, with bodies,
 * brawls and bystanders who scatter and shout for help.
 */
UCLASS()
class AURAPLAYGROUND_API ABattleKrogCrash : public AActor {
 GENERATED_BODY()
public:
 ABattleKrogCrash();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 /** True when a bike-width corridor still exists beside the wreck. */
 bool GapIsRideable() const;
 UPROPERTY(BlueprintReadOnly) FVector WreckSpot=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) FVector Approach=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) float DistanceToTunnelCm=0;
 UPROPERTY(BlueprintReadOnly) float GapClearanceCm=0;
 UPROPERTY(BlueprintReadOnly) int32 Bystanders=0,Bodies=0,Fires=0,Brawlers=0;
 /** Wrecked scooters, and how many primitive parts they are built from. */
 UPROPERTY(BlueprintReadOnly) int32 Scooters=0,ScooterParts=0;
 UPROPERTY(BlueprintReadOnly) FString ShoutText;
 UPROPERTY(BlueprintReadOnly) float ShoutRemaining=0;
private:
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> CrashRoot;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> RoadBlock;
 UPROPERTY() TArray<TWeakObjectPtr<AActor>> Spawned;
 TArray<TWeakObjectPtr<APiedmontPedestrian>> Crowd;
 FVector RoadDir=FVector(1,0,0), RoadSide=FVector(0,1,0);
 float ShoutClock=0,TopUpClock=0;
 int32 ShoutIndex=0;
 void SpawnWreck();
 void TopUp();
};
