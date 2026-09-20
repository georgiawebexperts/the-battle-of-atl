#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleScooterTraffic.generated.h"
class APiedmontPathSpline;

UCLASS()
class AURAPLAYGROUND_API ABattleScooterTraffic : public AActor {
 GENERATED_BODY()
public:
 ABattleScooterTraffic();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) int32 SpawnedScooters=0;
 UPROPERTY(BlueprintReadOnly) int32 PlayerContacts=0;
private:
 struct FMovingScooter {TWeakObjectPtr<AActor> Actor;TWeakObjectPtr<APiedmontPathSpline> Path;float Distance=0,Speed=350;bool Reverse=false;};
 TArray<FMovingScooter> Scooters;
 UPROPERTY() TSubclassOf<AActor> ScooterClass;
 float ContactCooldown=0;
 // Visual QA, Development only: -BattleScooterTrafficReviewDir=<folder> frames
 // the nearest moving scooter from the side and from ahead, which is the only
 // way to see a prop defect that the rider's own view hides.
 void TickScooterReview(float Dt);
 float ReviewClock=0;
 int32 ReviewStage=0;
 UPROPERTY() TObjectPtr<class ACameraActor> ReviewCamera;
};
