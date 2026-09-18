#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattlePickup.generated.h"
class APawn;class UStaticMeshComponent;class UPointLightComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleColaPickup : public AActor {
 GENERATED_BODY()
public:
 ABattleColaPickup();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) bool bConsumed=false;
 UPROPERTY(BlueprintReadOnly) bool bTrailPickup=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bTimeBonus=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bSpeedBonus=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float HealAmount=35;
 UFUNCTION(BlueprintCallable) bool TryCollect(APawn* Pawn);
private:
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Can;
 UPROPERTY() TObjectPtr<UPointLightComponent> Glow;
 FVector RestLocation;
 float Clock=0;
};
UCLASS()
class AURAPLAYGROUND_API ABattlePickupDirector : public AActor {
 GENERATED_BODY()
public:
 ABattlePickupDirector();
 virtual void BeginPlay() override;
 UPROPERTY(BlueprintReadOnly) int32 Spawned=0;
 UPROPERTY(BlueprintReadOnly) int32 WeaponCrates=0;
 UPROPERTY(BlueprintReadOnly) int32 TimePickups=0;
 UPROPERTY(BlueprintReadOnly) int32 AmmoPickups=0;
 UPROPERTY(BlueprintReadOnly) int32 HornPickups=0;
 UPROPERTY(BlueprintReadOnly) int32 SpeedPickups=0;
 UPROPERTY(BlueprintReadOnly) int32 ParkPickups=0;
 UPROPERTY(BlueprintReadOnly) int32 TrailPickups=0;
 UPROPERTY(BlueprintReadOnly) int32 PlacementFailures=0;
private:
 bool SpawnCola(FVector Surface,bool Trail,float Heal,int32 WeaponSlot=-1,bool bLandmark=false);
 TArray<FVector> Locations;
};

UCLASS()
class AURAPLAYGROUND_API ABattleWeaponCrate : public AActor {
 GENERATED_BODY()
public:
 ABattleWeaponCrate();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 WeaponSlot=1;
 UPROPERTY(BlueprintReadOnly) bool bConsumed=false;
 UFUNCTION(BlueprintCallable) bool TryCollect(APawn* Pawn);
private:
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Box;
 UPROPERTY() TObjectPtr<UPointLightComponent> Glow;
};

UCLASS()
class AURAPLAYGROUND_API ABattleHornPickup : public AActor {
 GENERATED_BODY()
public:
 ABattleHornPickup();
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) bool bConsumed=false;
 bool TryCollect(APawn* Pawn);
};
