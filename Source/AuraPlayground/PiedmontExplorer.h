#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PiedmontExplorer.generated.h"
class APiedmontBike;
class UPoseableMeshComponent;
class USpringArmComponent;
class UCameraComponent;
UCLASS()
class AURAPLAYGROUND_API APiedmontExplorer : public ACharacter {
 GENERATED_BODY()
public:
 APiedmontExplorer();
 virtual void BeginPlay() override;
 virtual void Tick(float DeltaSeconds) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<APiedmontBike> Bike;
 UPROPERTY(BlueprintReadOnly) bool bSwimming=false;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPoseableMeshComponent> Body;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> CameraArm;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
 UFUNCTION(BlueprintCallable) bool Remount();
 UFUNCTION(BlueprintCallable) void ValidationKey(FName Key,bool Pressed);
private:
 void Interact();
 void AnimateBody(float Dt);
 void UpdateSwimming(float Dt);
 TArray<FTransform> RestPose;
 TArray<int32> Parents;
 TArray<FName> Bones;
 float Gait=0;
};
