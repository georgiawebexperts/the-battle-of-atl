#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleRoadCrossing.generated.h"
class UBoxComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleRoadCrossing : public AActor {
 GENERATED_BODY()
public:
 ABattleRoadCrossing();
 virtual void Tick(float DeltaSeconds) override;
 UPROPERTY(VisibleAnywhere) UBoxComponent* CrossingArea;
 // Director or authored signal controls this; no guessed real-world timing.
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bVehicleGreen=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bVehicleAmber=false;
 UPROPERTY(EditAnywhere) bool bAutoCycle=false;
 UPROPERTY(EditAnywhere,meta=(ClampMin="1")) float GreenSeconds=18.f;
 UPROPERTY(EditAnywhere,meta=(ClampMin="1")) float AmberSeconds=3.f;
 UPROPERTY(EditAnywhere,meta=(ClampMin="1")) float RedSeconds=9.f;
 bool CanEnter(const AActor* Vehicle,bool AllowAmber=false) const;
 bool TryReserve(AActor* Vehicle,bool AllowAmber=false);
 void ReleaseVehicle(const AActor* Vehicle);
private:
 TWeakObjectPtr<AActor> ReservedVehicle;
 bool bOwnerEntered=false;
 float CycleClock=0.f;
};
