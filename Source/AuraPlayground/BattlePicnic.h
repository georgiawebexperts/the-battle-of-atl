#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattlePicnic.generated.h"
class APiedmontPedestrian;
class UStaticMeshComponent;

UCLASS()
class AURAPLAYGROUND_API ABattlePicnicGroup : public AActor {
 GENERATED_BODY()
public:
 ABattlePicnicGroup();
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<APiedmontPedestrian>> Chillers;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Blanket;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> PicnicBag;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Drink;
 UPROPERTY(BlueprintReadOnly) bool bReady=false;
 int32 GroupVariant=0;
};

UCLASS()
class AURAPLAYGROUND_API ABattlePicnicDirector : public AActor {
 GENERATED_BODY()
public:
 ABattlePicnicDirector();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<ABattlePicnicGroup>> Groups;
 UPROPERTY(BlueprintReadOnly) int32 DesiredGroups=0;
 UPROPERTY(BlueprintReadOnly) int32 PlacementFailures=0;
private:
 float AuditClock=0,AuditMotion=0;
 bool bAuditSampled=false,bAuditDone=false,bReviewSetup=false;
 FVector AuditHand=FVector::ZeroVector;
};
