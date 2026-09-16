#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleDance.generated.h"
class APiedmontPedestrian;
class UAudioComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class AURAPLAYGROUND_API ABattleDanceCircle : public AActor {
 GENERATED_BODY()
public:
 ABattleDanceCircle();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<APiedmontPedestrian>> Dancers;
 UPROPERTY(BlueprintReadOnly) int32 DesiredDancers=0;
 UPROPERTY(BlueprintReadOnly) bool bReady=false;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Speaker;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> SpeakerFace;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UAudioComponent> Music;
private:
 float AuditClock=0;
 bool bAuditSampled=false,bAuditFinished=false,bReviewCaptured=false;
 int32 ReviewCaptureStage=0;
 TArray<FVector> AuditHands;
};
