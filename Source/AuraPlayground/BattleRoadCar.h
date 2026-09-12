#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleRoadCar.generated.h"
class UBoxComponent;
class UStaticMeshComponent;
class ABattleRoadCrossing;
USTRUCT()
struct FBattleCarCrossing {
 GENERATED_BODY()
 UPROPERTY(EditAnywhere) TObjectPtr<ABattleRoadCrossing> Crossing=nullptr;
 // Car-centre distance along Route, before its nose enters the crossing.
 UPROPERTY(EditAnywhere) float StopDistance=0.f;
};

// Authored world-space lane, sampled densely enough to preserve road curvature.
UCLASS()
class AURAPLAYGROUND_API ABattleRoadCar : public AActor {
 GENERATED_BODY()
public:
 ABattleRoadCar();
 virtual void BeginPlay() override;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 PaintVariant=-1;
 UFUNCTION(BlueprintCallable) void ApplyPaint(int32 Variant);
 virtual void Tick(float DeltaSeconds) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(EditAnywhere) TArray<FVector> Route;
 UPROPERTY(EditAnywhere) float CruiseSpeed=650.f;
 UPROPERTY(EditAnywhere) TArray<FBattleCarCrossing> Crossings;
 UPROPERTY(BlueprintReadOnly) bool bWaitingForCrossing=false;
 UPROPERTY(VisibleAnywhere) UBoxComponent* Collision;
 UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Body;
 UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Glass;
 UPROPERTY(VisibleAnywhere) TArray<UStaticMeshComponent*> Wheels;
 UPROPERTY(BlueprintReadOnly) float Speed=0.f;
 UPROPERTY(BlueprintReadOnly) float DistanceTravelled=0.f;
 UPROPERTY(BlueprintReadOnly) bool bObstacleAhead=false;
 UPROPERTY(BlueprintReadOnly) bool bRouteFinished=false;
 UPROPERTY(BlueprintReadOnly) bool bGrounded=false;
 FString LastObstacle;
 bool StartRoute();
private:
 float RouteDistance=0.f;
 float WheelAngle=0.f;
 bool bStarted=false;
 bool bUseBodyHull=false;
 TArray<float> Lengths;
 TSet<int32> ClearedCrossings;
 TSet<int32> AmberStopping;
 FVector SampleRoute(float Distance) const;
 bool GroundPose(FVector Point,FVector Direction,FTransform& Pose,TArray<FVector>& Contacts) const;
 void UpdateWheels(const TArray<FVector>& Contacts,float Travel,float Steering);
};
