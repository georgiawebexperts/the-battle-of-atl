#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "PiedmontBike.generated.h"
class UCapsuleComponent;
class UPoseableMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;

UCLASS()
class AURAPLAYGROUND_API APiedmontWaterHazard : public AActor {
 GENERATED_BODY()
public:
 APiedmontWaterHazard();
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FVector> Polygon;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float DetectionHeight=190;
 bool ContainsBike(const FVector& WorldPoint) const;
};

UCLASS(ClassGroup=Movement, meta=(BlueprintSpawnableComponent))
class AURAPLAYGROUND_API UPiedmontBikeMovement : public UPawnMovementComponent {
 GENERATED_BODY()
public:
 UPiedmontBikeMovement();
 virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 UPROPERTY(BlueprintReadOnly) float Speed=0;
 UPROPERTY(BlueprintReadOnly) float BrakePressure=0;
 TArray<TWeakObjectPtr<APiedmontWaterHazard>> WaterHazards;
 UPROPERTY(BlueprintReadOnly) float Lean=0;
 UPROPERTY(BlueprintReadOnly) float Pitch=0;
 UPROPERTY(BlueprintReadOnly) bool bGrass=false;
 UPROPERTY(BlueprintReadOnly) bool bGrounded=false;
 UPROPERTY(BlueprintReadOnly) int32 Gear=1;
 UPROPERTY(BlueprintReadOnly) float Recovery=0;
 UPROPERTY(BlueprintReadOnly) int32 Crashes=0;
 UPROPERTY(BlueprintReadOnly) FString LastCrash;
 UPROPERTY(BlueprintReadOnly) float TopSpeed=0;
 float Pedal=0,Steer=0,Brake=0,Cadence=0;
 FVector SafeLocation=FVector::ZeroVector; FRotator SafeRotation=FRotator::ZeroRotator;
 void Shift(int Delta); void Crash(const FString& Reason); void Respawn();
 float GearLimit() const;
private:
 void Step(float Dt);
 float VerticalSpeed=0,SafeTime=0,SteeringRack=0,GripOverload=0;
};

UCLASS()
class AURAPLAYGROUND_API APiedmontBike : public APawn {
 GENERATED_BODY()
public:
 APiedmontBike();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 virtual UPawnMovementComponent* GetMovementComponent() const override;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPiedmontBikeMovement> Ride;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPoseableMeshComponent> Rider;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UCapsuleComponent> Capsule;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Visual;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Arm;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Chase;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Handlebar;
 UPROPERTY(BlueprintReadOnly) bool bFirstPerson=false;
 UFUNCTION(BlueprintCallable) void ToggleCamera();
 UFUNCTION(BlueprintCallable) void ResetRide();
 UFUNCTION(BlueprintCallable) void ValidationKey(FName Key,bool Pressed);
private:
 void PedalInput(float V);void Steering(float V);void BrakeOn();void BrakeOff();void GearUp();void GearDown();
 void PoseRider(float Dt);
 TObjectPtr<UStaticMeshComponent> FrontWheel,RearWheel,FrontFork;
 TArray<FTransform> ReferencePose;TArray<int32> Parents;TArray<FName> BoneNames;
 float WheelAngle=0;
};

UCLASS()
class AURAPLAYGROUND_API APiedmontRideHUD : public AHUD {
 GENERATED_BODY()
public: virtual void DrawHUD() override;
};
UCLASS()
class AURAPLAYGROUND_API APiedmontRideMode : public AGameModeBase {
 GENERATED_BODY()
public: APiedmontRideMode();
};
