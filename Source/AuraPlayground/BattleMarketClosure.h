#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMarketClosure.generated.h"
UCLASS()
class AURAPLAYGROUND_API ABattleMarketClosure: public AActor {
 GENERATED_BODY()
public:
 ABattleMarketClosure();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
};
