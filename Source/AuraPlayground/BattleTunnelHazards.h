#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleTunnelHazards.generated.h"

class ABattlePothole;
class UStaticMeshComponent;
class APlayerController;
class ACameraActor;

/**
 * The inside of the Krog tunnel.
 *
 * Elliott rode 128 and asked why there is no scooter at Krog Street Tunnel, why
 * the tunnel is not longer and where the treacherous potholes are that throw
 * you off the bike. The crash scene sits outside the mouth; this actor owns the
 * bore itself: cratered pavement down its whole length, staggered across the
 * lane so a fast rider has to weave, and scooters strewn along both walls.
 *
 * Everything is traced onto the pavement that is actually there rather than
 * placed at authored heights, because the tunnel floor is baked geometry that
 * has moved twice. A pothole that lands on the rail deck above the bore is
 * worse than no pothole at all, so a station with no floor under it is skipped
 * and counted.
 */
UCLASS()
class AURAPLAYGROUND_API ABattleTunnelHazards : public AActor {
 GENERATED_BODY()
public:
 ABattleTunnelHazards();
 virtual void BeginPlay() override;
 /** World direction the rider travels through the bore, Z zeroed. */
 UPROPERTY(BlueprintReadOnly) FVector BoreAxis=FVector(1,0,0);
 /** Where the first deep hole is, and how far the rider accelerates to reach it. */
 UPROPERTY(BlueprintReadOnly) FVector FirstDeepHole=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) FVector FirstDeepHoleApproach=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) FVector FirstScooterSpot=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) int32 StationsProbed=0,StationsGrounded=0,DeepHoles=0,ShallowHoles=0,Scooters=0,Punks=0;
 UPROPERTY(BlueprintReadOnly) float BoreLengthCm=0;
 virtual void Tick(float Dt) override;
private:
 UPROPERTY() TArray<TObjectPtr<ABattlePothole>> Holes;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> CraterTrim;
 UPROPERTY() TObjectPtr<ACameraActor> ReviewCamera;
 bool bHasScooterSpot=false,bReviewPlaced=false,bReview=false;
 int32 ReviewStage=0;
 float ReviewClock=0;
 void CarveHoles();
 void StrewnScooters();
 void TunnelPunks();
};

/** Opt-in native audit: -BattleTunnelHazardAudit drives the bore over a deep hole. */
void TickBattleTunnelHazardAudit(APlayerController* PC,float Dt);
