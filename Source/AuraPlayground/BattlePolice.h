#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattlePolice.generated.h"
UCLASS()
class AURAPLAYGROUND_API ABattlePolice : public APiedmontExplorer {
 GENERATED_BODY()
public:
 static constexpr float TaserWarningSeconds=3.5f;
 static constexpr float TaserAimLockSeconds=1.6f;
 static constexpr float TaserHitRadiusCm=50.f;
 static constexpr float TaserCooldownSeconds=18.f;
 static constexpr float SquadRecoverySeconds=8.f;
 ABattlePolice();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 bool FireTaser();
 UPROPERTY(BlueprintReadOnly) float Health=100;
 UPROPERTY(BlueprintReadOnly) bool bWarning=false;
 UPROPERTY(BlueprintReadOnly) int32 TaserShots=0;
 UPROPERTY() TObjectPtr<class UAudioComponent> WarningVoice;
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> AimBeam;
 int32 WarningVoiceStarts=0;
 float VoiceCooldown=0;
 float WarningRemaining=0,Cooldown=3,PathDelay=0;
 UPROPERTY(BlueprintReadOnly) float TaserDrawBlend=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bAmbientMurderK=false;
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UTextRenderComponent> TauntLabel;
 UPROPERTY(BlueprintReadOnly) int32 TauntsHurled=0;
 UPROPERTY(BlueprintReadOnly) FVector LastTaserOrigin=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly) FVector WarningAimPoint=FVector::ZeroVector;
 FVector TaserMuzzle() const;
 void UpdateTaserBeam();
 UPROPERTY() TObjectPtr<UAnimSequence> TaserAim;
 UPROPERTY() TObjectPtr<UAnimSequence> TaserIdle;
 UPROPERTY() TObjectPtr<UAnimSequence> TaserWalk;
 UPROPERTY() TObjectPtr<UAnimSequence> TaserRun;
 float DischargeRemaining=0;
 float TauntCooldown=1,TauntVisible=0;
protected:
 virtual void AnimateBody(float Dt) override;
 virtual bool CanUseWeapon() const override{return false;}
private:
 bool CanReachTarget(APawn* Target) const;
 bool CanStartTaser(APawn* Target) const;
};
