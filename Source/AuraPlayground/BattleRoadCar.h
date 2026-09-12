#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleRoadCar.generated.h"
class UBoxComponent;
class UStaticMeshComponent;

// Authored world-space lane, sampled densely enough to preserve road curvature.
UCLASS()
class AURAPLAYGROUND_API ABattleRoadCar : public AActor {
 GENERATED_BODY()
public:
 ABattleRoadCar();
 virtual void Tick(float DeltaSeconds) override;
 UPROPERTY(EditAnywhere) TArray<FVector> Route;
 UPROPERTY(EditAnywhere) float CruiseSpeed=650.f;
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
 TArray<float> Lengths;
 FVector SampleRoute(float Distance) const;
 bool GroundPose(FVector Point,FVector Direction,FTransform& Pose,TArray<FVector>& Contacts) const;
 void UpdateWheels(const TArray<FVector>& Contacts,float Travel,float Steering);
};
