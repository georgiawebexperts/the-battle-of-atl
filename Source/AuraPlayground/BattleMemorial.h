#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMemorial.generated.h"
UCLASS()
class AURAPLAYGROUND_API ABattleMemorial : public AActor {
 GENERATED_BODY()
public:
 ABattleMemorial();
 virtual void BeginPlay() override;
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UTextRenderComponent> Words;
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> MarkerPart;
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> Stone;
};
