#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BattleDifficulty.generated.h"

// Distances are game centimetres; speed is cm/s. Counts tune the full V3 roster.
USTRUCT(BlueprintType)
struct AURAPLAYGROUND_API FBattleDifficultyRow : public FTableRowBase {
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FString Warning;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float TimeLimitSeconds=900;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Walkers=14;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Joggers=6;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Cyclists=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Skaters=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 DogOwners=2;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 FrisbeeGroups=2;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 PicnicGroups=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Dancers=5;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Scooters=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ScooterSlowSpeed=250;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ScooterMediumSpeed=350;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ScooterFastSpeed=400;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ScooterWrongWayFraction=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 ScooterPackSize=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 IllegalBikes=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float IllegalBikeSpeed=2235.2;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ArmedBikeFraction=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Zombies=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ZombieRespawnSeconds=12;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ZombieDamage=20;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ZombieWarningSeconds=.9f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ZombieSpeed=140;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float SprinterFraction=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float SprinterSpeed=500;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 TunnelWaveSize=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Shooters=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 KrogerShooters=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ShooterWarningSeconds=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 KnifeBehavior=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float KnifeSpeed=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 WeaponCrates=20;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 HealthPickups=18;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float ColaHealAmount=35;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float RadarRange=18000;
};
