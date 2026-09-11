#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PiedmontBike.h"
#include "BattleBike.generated.h"
class ABattleRider;
class UPoseableMeshComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class AURAPLAYGROUND_API UBattleBikeMovement : public UCharacterMovementComponent {
 GENERATED_BODY()
public:
 UBattleBikeMovement();
 virtual void TickComponent(float Dt,ELevelTick TickType,FActorComponentTickFunction* Function) override;
 virtual void CalcVelocity(float Dt,float Friction,bool Fluid,float Braking) override;
 virtual void HandleImpact(const FHitResult& Hit,float TimeSlice,const FVector& MoveDelta) override;
 UPROPERTY(BlueprintReadOnly) float Speed=0;
 UPROPERTY(BlueprintReadOnly) int32 Gear=1;
 UPROPERTY(BlueprintReadOnly) float Recovery=0;
 UPROPERTY(BlueprintReadOnly) int32 Wipeouts=0;
 UPROPERTY(BlueprintReadOnly) FString RecoveryReason;
 UPROPERTY(BlueprintReadOnly) bool bGrass=false;
 UPROPERTY(BlueprintReadOnly) float SlideRemaining=0;
 float Pedal=0,Steer=0,Brake=0,Cadence=0;
 FVector LastSafeLocation;
 void Wipeout(const FString& Reason,bool Water=false);
 void Shift(int32 Delta){Gear=FMath::Clamp(Gear+Delta,1,5);}
private:
 bool bWaterReturn=false;
 float BounceRemaining=0,ContactCooldown=0,PreviousBrake=0;
 FVector BounceDirection,ReturnLocation;
};

UCLASS()
class AURAPLAYGROUND_API ABattleBike : public ACharacter {
 GENERATED_BODY()
public:
 ABattleBike(const FObjectInitializer& Init);
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UBattleBikeMovement> Ride;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCapsuleComponent> Capsule;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Visual;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UPoseableMeshComponent> Rider;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Arm;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Chase;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Handlebar;
 UPROPERTY(BlueprintReadOnly) bool bFirstPerson=false;
 UPROPERTY(BlueprintReadOnly) bool bParked=false;
 UPROPERTY(BlueprintReadOnly) float RiderHealth=100;
 UPROPERTY(BlueprintReadOnly) int32 PistolAmmo=12;
 UFUNCTION(BlueprintCallable) bool Dismount();
 bool Remount(ABattleRider* Person);
 UFUNCTION(BlueprintCallable) void ToggleCamera();
 UFUNCTION(BlueprintCallable) void ValidationKey(FName Key,bool Pressed);
 FVector FindPathReturn() const;
private:
 void Interact(){Dismount();}
 void PoseRider(float Dt);
 void GearUp(){Ride->Shift(1);}void GearDown(){Ride->Shift(-1);}
 TObjectPtr<UStaticMeshComponent> FrontWheel,RearWheel;
 TArray<FTransform> ReferencePose;TArray<int32> Parents;TArray<FName> BoneNames;
 float WheelAngle=0;
};
UCLASS()
class AURAPLAYGROUND_API ABattleLabMode : public APiedmontRideMode {
 GENERATED_BODY()
public:
 ABattleLabMode();
 virtual void StartPlay() override;
 virtual void Tick(float Dt) override;
};
UCLASS()
class AURAPLAYGROUND_API ABattleLabHUD : public AHUD {
 GENERATED_BODY()
public:
 virtual void DrawHUD() override;
};
