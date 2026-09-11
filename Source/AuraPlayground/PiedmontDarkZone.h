#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PiedmontDarkZone.generated.h"
class UBoxComponent;
UCLASS()
class AURAPLAYGROUND_API APiedmontDarkZone : public AActor {
 GENERATED_BODY()
public:
 APiedmontDarkZone();
 UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Bounds;
 bool Contains(FVector Point) const;
};
