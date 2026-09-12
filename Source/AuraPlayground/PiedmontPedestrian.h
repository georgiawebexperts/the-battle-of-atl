#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "BattleSleeperTrigger.h"
#include "PiedmontPedestrian.generated.h"
class APiedmontBike;
class USkeletalMeshComponent;
UENUM(BlueprintType)
enum class EPiedmontPedestrianKind : uint8 { Walker, Jogger };

/** Moving, collidable park visitors driven by an AIController on the park navmesh. */
UCLASS()
class AURAPLAYGROUND_API APiedmontPedestrian : public APiedmontExplorer {
 GENERATED_BODY()
public:
 APiedmontPedestrian();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPROPERTY(BlueprintReadOnly) int32 KnockdownPhase=0;
 UPROPERTY(BlueprintReadOnly) int32 CompletedRecoveries=0;
 UPROPERTY(BlueprintReadOnly) FString RecoveryDirection;
 UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> PhysicsBody;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 CityAppearanceVariant=-1;
 UPROPERTY(BlueprintReadOnly) EPiedmontPedestrianKind Kind=EPiedmontPedestrianKind::Walker;
 UPROPERTY(BlueprintReadOnly) FVector Destination;
 UPROPERTY(BlueprintReadOnly) bool bHasDestination=false;
 UPROPERTY(BlueprintReadOnly) float PauseRemaining=0;
 UPROPERTY(BlueprintReadOnly) float StumbleRemaining=0;
 UPROPERTY(BlueprintReadOnly) int32 HornReactions=0;
 UPROPERTY(BlueprintReadOnly) int32 BikeContacts=0;
 UPROPERTY(BlueprintReadOnly) int32 CompletedWalks=0;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<APiedmontPedestrian> GroupLeader;
 float GroupSide=1;
 void Configure(EPiedmontPedestrianKind NewKind);
 void HearHorn(APawn* Source);
 void BikeImpact(float Speed,FVector Direction);
 // Opt-in until encounter placement and chase/settle behavior are complete.
 UFUNCTION(BlueprintCallable) bool BeginSleeping();
 UFUNCTION(BlueprintCallable) bool WakeFromSleep();
 UFUNCTION(BlueprintCallable) bool WakeAndChase(APawn* Target);
 UFUNCTION(BlueprintCallable) bool BeginSettling();
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bReturnToSleepAfterChase=false;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) bool bAmbientSleeper=false;
 UPROPERTY(BlueprintReadOnly) int32 SleepPhase=0;
 UFUNCTION(BlueprintCallable) bool SetDestinationForValidation(FVector Goal);
protected:
 virtual void AnimateBody(float Dt) override;
 float YieldCooldown=0;
 virtual bool CanUseWeapon() const override {return false;}
private:
 FBattleSleeperTrigger SleeperTrigger;
 void TickSleeperTrigger(float Dt);
 void CancelSleepBehavior();
 void FinishSleeperChase();
 bool TickSleepBehavior(float Dt);
 TWeakObjectPtr<APawn> SleepTarget;
 FVector SleepOrigin;
 FVector SleepLanding;
 FRotator SleepLandingRotation;
 bool IsSettlePathClear(const FVector& Landing) const;
 float ChaseClock=0,ChaseRepath=0,ReturnClock=0;
 bool BeginKnockdown(float Speed,FVector Direction);
 void TickKnockdown(float Dt);
 bool BeginRecovery();
 void AttachCityParts(class USkinnedMeshComponent* Leader);
 float KnockdownClock=0,RecoveryRetry=0;
 FQuat StandingPelvis;
 FVector StandingForward,StandingRight;
 UPROPERTY() TArray<TObjectPtr<UAnimSequence>> RecoveryClips;
 UPROPERTY() TObjectPtr<UAnimSequence> BumpReaction;
 bool bPlayingBumpReaction=false;
 void InitializeCityAppearance();
 bool MoveTo(FVector Goal);
 void ChooseDestination();
 void YieldTo(APawn* Source,bool Horn);
 float ThinkRemaining=0,YieldRemaining=0;
};
