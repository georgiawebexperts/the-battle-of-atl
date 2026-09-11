#pragma once
#include "CoreMinimal.h"
#include "PiedmontPedestrian.h"
#include "BattleFrisbee.generated.h"
class UStaticMeshComponent;
class UTextRenderComponent;
class USoundAttenuation;

UCLASS()
class AURAPLAYGROUND_API ABattleFrisbeePlayer : public APiedmontPedestrian {
 GENERATED_BODY()
public:
 ABattleFrisbeePlayer();
 virtual void Tick(float Dt) override;
 FVector WalkTarget=FVector::ZeroVector;
 bool bWalkToTarget=false;
 float ThrowPose=-1;
 bool bCatchPose=false;
protected:
 virtual bool CanUseWeapon() const override {return false;}
 virtual FVector AdjustVisitorHand(int32 Side,FVector Target) const override;
private:
 float AvoidRemaining=0;
 FVector AvoidDirection;
};

UCLASS()
class AURAPLAYGROUND_API ABattleFrisbeeGroup : public AActor {
 GENERATED_BODY()
public:
 ABattleFrisbeeGroup();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<ABattleFrisbeePlayer>> Players;
 UPROPERTY(BlueprintReadOnly) int32 Throws=0,Catches=0,Misses=0,Retrievals=0,AmmoGiven=0;
 UPROPERTY(BlueprintReadOnly) int32 Supply=24;
 UPROPERTY(BlueprintReadOnly) FVector PathLanding;
 UPROPERTY(BlueprintReadOnly) bool bReady=false;
 UPROPERTY(BlueprintReadOnly) bool bDiscOnGround=false;
 UPROPERTY(BlueprintReadOnly) FVector DiscPosition;
 UFUNCTION(BlueprintCallable) bool TrySupply(APawn* Pawn);
 FVector FieldDirection=FVector::ForwardVector;
 FString AreaName=TEXT("Meadow");
 static bool Ground(UWorld* World,FVector XY,FVector& Point,bool GrassOnly=false);
private:
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Disc;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> SupplyBag;
 UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
 UPROPERTY() TObjectPtr<USoundAttenuation> Attenuation;
 TArray<FVector> Homes;
 FVector FlightStart,FlightEnd;
 float PhaseTime=0,FlightDuration=1.8f,SupplyCooldown=0;
 int32 Holder=0,Phase=0;
 bool bMissThrow=false;
 void BeginThrow();
 void Sound(const TCHAR* Asset,float Volume);
};

UCLASS()
class AURAPLAYGROUND_API ABattleParkLifeDirector : public AActor {
 GENERATED_BODY()
public:
 virtual void BeginPlay() override;
 UPROPERTY(BlueprintReadOnly) int32 FrisbeeGroups=0,PlacementFailures=0;
};
