#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PiedmontWorldTools.generated.h"
class ALandscape;
class UPhysicsAsset;
UCLASS()
class AURAPLAYGROUND_API UPiedmontWorldTools : public UBlueprintFunctionLibrary {
 GENERATED_BODY()
public:
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Physics")
 static UPhysicsAsset* CreatePlayerCrashPhysics();
 UFUNCTION(BlueprintCallable,Category="Battle")
 static FString InspectZombieShoeBounds();
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Foliage")
 static float GetTreeLowGeometryRadius(UStaticMesh* Mesh,float LocalHeightAboveBottom);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Foliage")
 static AActor* CreateFoliageReviewInstances(const TArray<FTransform>& Stations,UStaticMesh* Mesh);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Foliage")
 static bool EnableParkTrunkCollision(AActor* Actor);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Foliage")
 static AActor* CreateParkFoliageAtPath(const TArray<FTransform>& Stations,UStaticMesh* Mesh,const FString& AssetFolder);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Foliage")
 static AActor* CreateParkFoliage(const TArray<FTransform>& Stations,UStaticMesh* Mesh);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static void FinishEditorAssetLoading();
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static void TickSceneReview();
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static void RebuildWaterZones();
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|World")
 static bool RefreshLandscapeCollision(ALandscape* Landscape);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Navigation")
 static bool BuildParkNavigation(FVector Center,FVector Extent);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Navigation")
 static bool IsParkNavigationBuilding();
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Navigation")
 static bool FinishParkNavigationBuild();
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Navigation")
 static bool ProjectParkNavigation(FVector Point,FVector& Projected);
 UFUNCTION(BlueprintCallable,CallInEditor,Category="Piedmont|Navigation")
 static float ParkRouteLength(FVector Start,FVector End);
 UFUNCTION(BlueprintCallable,Category="Piedmont|Validation")
 static AActor* SpawnValidationDarkZone(UObject* WorldContext,FVector Location);
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
