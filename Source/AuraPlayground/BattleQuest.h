#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleQuest.generated.h"
class AHUD;class UCanvas;
UCLASS()
class AURAPLAYGROUND_API ABattleQuest : public AActor {
 GENERATED_BODY()
public:
 // East is +X and north is -Y; screen coordinates increase downward.
 static FVector2D RadarOffset(FVector2D WorldDelta,float Scale){return WorldDelta*Scale;}
 ABattleQuest();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(BlueprintReadOnly) bool bReady=false;
 UPROPERTY(BlueprintReadOnly) bool bCollected=false;
 UPROPERTY(BlueprintReadOnly) FVector ArtifactLocation;
 UPROPERTY(BlueprintReadOnly) FVector ExitLocation;
 UPROPERTY(BlueprintReadOnly) FVector RouteTargetLocation;
 UPROPERTY(BlueprintReadOnly) int32 EastsideRoutePointCount=0;
 UPROPERTY(BlueprintReadOnly) int32 NextCheckpoint=0;
 UPROPERTY(BlueprintReadOnly) FString CheckpointNotice;
 UPROPERTY(BlueprintReadOnly) TArray<FVector> CheckpointLocations;
 UPROPERTY(BlueprintReadOnly) FVector StartLocation;
 UPROPERTY(BlueprintReadOnly) FString ArtifactWay;
 UPROPERTY(BlueprintReadOnly) int32 CandidateCount=0;
 UPROPERTY(BlueprintReadOnly) int32 RadarSegmentCount=0;
 UPROPERTY(BlueprintReadOnly) TArray<FVector> RoutePoints;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float RadarRange=18000;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<AActor> Artifact;
 void ResetAfterDeath();
 FString SearchDirection(FVector Viewer) const;
 static FVector2D CoarseDirection(FVector2D Delta);
 FTransform InitialStartTransform;
 int32 SearchResets=0;
 bool ArtifactVisibleOnRadar(FVector Viewer) const;
 float WatchSignalStrength(FVector Viewer) const;
 float WatchFlashHz(FVector Viewer) const;
 void DrawRadar(AHUD* HUD,UCanvas* Canvas) const;
 static bool ClipToCircle(FVector2D& A,FVector2D& B,float Radius);
private:
 bool PlaceArtifact();
 void UpdateCheckpoints(float Dt);
 void SpawnCheckpointMarkers();
 int32 RouteEndIndex=0;
 float CheckpointNoticeTime=0;
 TArray<int32> CheckpointIndices;
 TArray<TObjectPtr<AActor>> CheckpointMarkers;
 void RefreshRoute();
 float Clock=0,RouteDelay=0,EnemyDelay=0;
 TArray<TPair<FVector2D,FVector2D>> Segments;
 TArray<FVector> EnemyLocations;
 TArray<FVector> Mainline;
};
