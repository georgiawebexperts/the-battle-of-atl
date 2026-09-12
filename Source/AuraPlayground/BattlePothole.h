#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattlePothole.generated.h"
class ABattleBike;
class UStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API ABattlePothole : public AActor {
 GENERATED_BODY()
public:
 ABattlePothole();
 virtual void Tick(float DeltaSeconds) override;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Visual;
 UPROPERTY(EditAnywhere) float ContactRadius=65.f;
 UPROPERTY(EditAnywhere) bool bDeep=false;
 UPROPERTY(EditAnywhere) float CrashSpeed=1000.f;
 UPROPERTY(BlueprintReadOnly) int32 Contacts=0;
 bool EvaluateTraversal(ABattleBike* Bike,const FVector& From,const FVector& To);
private:
 TWeakObjectPtr<ABattleBike> TrackedBike;
 FVector PreviousWheel=FVector::ZeroVector;
 bool bHasPrevious=false,bLatched=false;
};
