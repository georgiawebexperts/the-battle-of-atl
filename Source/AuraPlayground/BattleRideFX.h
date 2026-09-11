#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleRideFX.generated.h"
class UInstancedStaticMeshComponent;
// Fixed-size pools keep long sessions from accumulating trail actors or particles.
UCLASS()
class AURAPLAYGROUND_API ABattleRideFX : public AActor {
 GENERATED_BODY()
public:
 ABattleRideFX();
 virtual void Tick(float Dt) override;
 void AddSkid(FVector Start,FVector End,FVector Normal);
 void Splash(FVector Position);
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UInstancedStaticMeshComponent> Tracks;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UInstancedStaticMeshComponent> Spray;
 UPROPERTY(BlueprintReadOnly) int32 TrackCount=0;
 UPROPERTY(BlueprintReadOnly) int32 SplashCount=0;
 UPROPERTY(BlueprintReadOnly) int32 ActiveDrops=0;
private:
 struct FTrack {FTransform Transform;float Life=0;};
 struct FDrop {FVector Position,Velocity;float Life=0,Scale=0;};
 TArray<FTrack> Trail;TArray<FDrop> Drops;int NextTrack=0;float SplashAge=0;
};
