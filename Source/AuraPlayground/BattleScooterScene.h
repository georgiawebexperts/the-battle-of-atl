#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleScooterScene.generated.h"
class APiedmontPedestrian;
UCLASS()
class AURAPLAYGROUND_API ABattleScooterScene : public AActor {
 GENERATED_BODY()
public:
 ABattleScooterScene();
 virtual void Tick(float Dt) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float AppearanceChance=.18f;
 UPROPERTY(BlueprintReadOnly) bool bSelected=false;
 UPROPERTY(BlueprintReadOnly) bool bSceneReady=false;
 UPROPERTY(BlueprintReadOnly) bool bVisitStarted=false;
 UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<APiedmontPedestrian>> Participants;
private:
 bool bChoiceMade=false,bSpawned=false,bPosesSet=false,bReleased=false;
 float SetupAge=0,VisitAge=0;
 bool IsOffscreen() const;
 bool SpawnScene();
 void AbortScene();
};
