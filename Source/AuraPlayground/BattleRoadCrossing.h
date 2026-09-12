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
 bool CanEnter(const AActor* Vehicle) const;
 bool TryReserve(AActor* Vehicle);
 void ReleaseVehicle(const AActor* Vehicle);
private:
 TWeakObjectPtr<AActor> ReservedVehicle;
 bool bOwnerEntered=false;
};
