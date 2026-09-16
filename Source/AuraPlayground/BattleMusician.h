#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundWaveProcedural.h"
#include "BattleMusician.generated.h"
class APiedmontPedestrian;
class UAudioComponent;
class UStaticMeshComponent;

UCLASS()
class AURAPLAYGROUND_API UBattleInstrumentLoop : public USoundWaveProcedural {
 GENERATED_BODY()
public:
 UBattleInstrumentLoop(const FObjectInitializer& ObjectInitializer);
 int32 InstrumentKind=0;
 uint64 Cursor=0;
 virtual int32 OnGeneratePCMAudio(TArray<uint8>& OutAudio,int32 NumSamples) override;
};

UCLASS()
class AURAPLAYGROUND_API ABattleMusician : public AActor {
 GENERATED_BODY()
public:
 ABattleMusician();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 InstrumentKind=0;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<APiedmontPedestrian> Performer;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UAudioComponent> Music;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GuitarBody;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GuitarUpper;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GuitarNeck;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GuitarHole;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GuitarHoleBack;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> SaxBody;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> SaxBell;
 UPROPERTY() TObjectPtr<UBattleInstrumentLoop> Loop;
};

UCLASS()
class AURAPLAYGROUND_API ABattleParkMusicDirector : public AActor {
 GENERATED_BODY()
public:
 ABattleParkMusicDirector();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(BlueprintReadOnly) TArray<TObjectPtr<ABattleMusician>> Musicians;
private:
 float AuditClock=0;
 float AuditHandTravel=0;
 bool bAuditSampled=false,bAuditDone=false,bReviewSetup=false;
 TArray<FVector> AuditHands;
};
