#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PiedmontWorldTools.generated.h"
class ALandscape;
UCLASS()
class AURAPLAYGROUND_API UPiedmontWorldTools : public UBlueprintFunctionLibrary {
 GENERATED_BODY()
public:
 UFUNCTION(BlueprintCallable,Category="Piedmont|Validation")
 static AActor* SpawnValidationObstacle(UObject* WorldContext,FVector Location,FVector Scale);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static bool RefreshWaterBody(AActor* WaterActor);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static bool TraceWorldSurface(FVector Start,FVector End,FVector& ImpactPoint,AActor*& HitActor,float SweepRadius=0.f);
 /** Import measured, little-endian R16 samples into a real Unreal Landscape. Editor only. */
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static ALandscape* ImportMeasuredLandscape(const FString& Filename,int32 Width,int32 Height,FVector Location,FVector Scale);
};
