#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PiedmontPathSpline.generated.h"
class USplineComponent;
/** Authoritative OSM centerline retained for route, recovery and crowd navigation. */
UCLASS()
class AURAPLAYGROUND_API APiedmontPathSpline : public AActor {
 GENERATED_BODY()
public:
 APiedmontPathSpline();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Path") TObjectPtr<USplineComponent> Centerline;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Path") FString OsmWayId;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Path") float WidthCm=280.f;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Path") bool bBridge=false;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Path") bool bArtifactEligible=true;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Path") bool bRideValidated=false;
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Path")
 void SetCenterline(const TArray<FVector>& WorldPoints);
};
