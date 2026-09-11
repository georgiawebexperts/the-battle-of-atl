#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleHome.generated.h"
class UInstancedStaticMeshComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleHome:public AActor {
 GENERATED_BODY()
public:
 ABattleHome();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 bool TryFinish();
 bool bTunnelEntered=false,bTunnelExited=false;
 UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Road;
};
