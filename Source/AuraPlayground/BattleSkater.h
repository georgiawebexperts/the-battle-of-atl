#pragma once
#include "CoreMinimal.h"
#include "PiedmontPedestrian.h"
#include "BattleSkater.generated.h"
class USceneComponent;
UCLASS()
class AURAPLAYGROUND_API ABattleSkater:public APiedmontPedestrian {
 GENERATED_BODY()
public:
 ABattleSkater();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(EditAnywhere) int32 RouteIndex=1;
 UPROPERTY(EditAnywhere) float PhaseOffset=0;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SkateAssembly;
 float SkateClock=0,PushAmount=0,StanceError=0,Travelled=0,CarveLean=0;
 int32 Corners=0;
protected:
 virtual void AnimateBody(float Dt) override;
private:
 TArray<FTransform> SkateRest;
 TArray<int32> SkateParents;
 TArray<FName> SkateBones;
 FVector PreviousLocation=FVector::ZeroVector;
 int32 LastHorn=0;
 float YieldTime=0;
};
