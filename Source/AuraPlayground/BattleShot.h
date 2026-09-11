#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleShot.generated.h"
class APawn;class USceneComponent;class UStaticMeshComponent;class UPointLightComponent;
struct FBattleShotResult {FVector End=FVector::ZeroVector;float Damage=0;bool EnemyKilled=false;int32 Kills=0;};
FBattleShotResult FireBattlePistol(APawn* Shooter,USceneComponent* Gun,float SpreadDegrees);
FBattleShotResult FireBattleLongGun(APawn* Shooter,USceneComponent* Gun,int32 Slot,bool Aiming);
UCLASS()
class AURAPLAYGROUND_API ABattleShotFX : public AActor {
 GENERATED_BODY()
public:
 ABattleShotFX();
 virtual void BeginPlay() override;
 FVector Start,End;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Tracer;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Flash;
 UPROPERTY() TObjectPtr<UPointLightComponent> Light;
};
