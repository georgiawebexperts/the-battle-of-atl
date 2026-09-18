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
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPROPERTY(BlueprintReadOnly) int32 KnockdownPhase=0;
 UPROPERTY(BlueprintReadOnly) int32 CompletedRecoveries=0;
 UPROPERTY(BlueprintReadOnly) FString RecoveryDirection;
 UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> PhysicsBody;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 CityAppearanceVariant=-1;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 CityOutfitVariant=-1;
 UPROPERTY(BlueprintReadOnly) EPiedmontPedestrianKind Kind=EPiedmontPedestrianKind::Walker;
 UPROPERTY(BlueprintReadOnly) FVector Destination;
 UPROPERTY(BlueprintReadOnly) bool bHasDestination=false;
 UPROPERTY(BlueprintReadOnly) float PauseRemaining=0;
 UPROPERTY(BlueprintReadOnly) float StumbleRemaining=0;
 UPROPERTY(BlueprintReadOnly) int32 HornReactions=0;
 UPROPERTY(BlueprintReadOnly) int32 BikeContacts=0;
 UPROPERTY(BlueprintReadOnly) int32 CompletedWalks=0;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<APiedmontPedestrian> GroupLeader;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) bool bParkDancer=false;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 DanceVariant=0;
 UPROPERTY(BlueprintReadOnly) float DanceClock=0;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) bool bParkMusician=false;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 MusicianKind=0;
 UPROPERTY(BlueprintReadOnly) float MusicClock=0;
 // Guitarist arm pose, in bone space. These put the fretting hand up and to the
 // performer's left and the strumming hand over the sound hole.
 // Solved by -BattleGuitarSweep against the reference-pose hold: the fretting
 // hand lands 132 cm up and 26 cm to the player's left, the strumming hand over
 // the sound hole at 104 cm. Both land within 1 cm of those marks.
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator GuitarArmL=FRotator(-60,-60,-20);
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator GuitarForeL=FRotator(-95,0,35);
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator GuitarArmR=FRotator(-50,-25,0);
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator GuitarForeR=FRotator(-25,0,35);
 // Same solve for the saxophonist: upper hand and lower hand both on the body
 // at 116 cm and 98 cm, in front of the chest.
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator SaxArmL=FRotator(-50,-25,-20);
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator SaxForeL=FRotator(-95,0,20);
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator SaxArmR=FRotator(-65,-45,0);
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FRotator SaxForeR=FRotator(-65,0,-20);
 UPROPERTY(EditAnywhere,BlueprintReadOnly) bool bPicnicChiller=false;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 PicnicPose=0;
 UPROPERTY(BlueprintReadOnly) float ChillClock=0;
 float GroupSide=1;
 void Configure(EPiedmontPedestrianKind NewKind);
 UFUNCTION(BlueprintCallable) bool BeginIncidentPose(UAnimSequence* Clip,float PoseSeconds,float HoldSeconds);
 UFUNCTION(BlueprintCallable) void ReleaseIncidentPose();
 UPROPERTY(BlueprintReadOnly) bool bIncidentPosing=false;
 bool BeginBenchReach();
 bool BeginBenchIgnition(class ABattleParkFurniture* Furniture,int32 Index);
 UPROPERTY(BlueprintReadOnly) int32 BenchesIgnited=0;
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> BenchLighterHandle;
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> BenchLighterStem;
 UPROPERTY(BlueprintReadOnly) bool bBenchReaching=false;
 void HearHorn(APawn* Source);
 void HearGunfire(FVector Source);
 UPROPERTY(BlueprintReadOnly) float PanicRemaining=0;
 FVector PanicOrigin;
 float PanicRepath=0;
 void BikeImpact(float Speed,FVector Direction);
 /** Shouted line when the rider runs into them; shown as a HUD subtitle. */
 UPROPERTY(BlueprintReadOnly) FString CurseLine;
 UPROPERTY(BlueprintReadOnly) float CurseRemaining=0;
 void Curse();
 // Opt-in until encounter placement and chase/settle behavior are complete.
 UFUNCTION(BlueprintCallable) bool BeginSleeping();
 UFUNCTION(BlueprintCallable) bool WakeFromSleep();
 UFUNCTION(BlueprintCallable) bool WakeAndChase(APawn* Target);
 UFUNCTION(BlueprintCallable) bool BeginSettling();
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bReturnToSleepAfterChase=false;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) bool bAmbientSleeper=false;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(ClampMin="0.0",ClampMax="1.0")) float AmbientWakeChance=.18f;
 UPROPERTY(BlueprintReadOnly) int32 SleepPhase=0;
 UFUNCTION(BlueprintCallable) bool SetDestinationForValidation(FVector Goal);
 // Re-evaluates the performer hold after pose constants change. Review only.
 void SamplePerformerPose(float Dt){AnimateBody(Dt);}
protected:
 virtual void AnimateBody(float Dt) override;
 float YieldCooldown=0;
 virtual bool CanUseWeapon() const override {return false;}
private:
 void CancelIncidentPose();
 bool TickIncidentPose(float Dt);
 bool CreateIncidentCollision();
 void UpdateIncidentCollision();
 void ClearIncidentCollision();
 UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> IncidentCollision;
 uint8 IncidentMovementMode=0,IncidentCustomMovementMode=0;
 float IncidentHoldRemaining=0;
 bool bIncidentRecovering=false;
 void CancelBenchReach();
 void TickBenchIgnition(float Dt);
 bool IsAtIgnitionBench(class ABattleParkFurniture* Furniture,int32 Index) const;
 TWeakObjectPtr<class ABattleParkFurniture> IgnitionFurniture;
 int32 IgnitionBench=INDEX_NONE;
 float BenchReachClock=0;
 bool bBenchRetreatPending=false;
 FVector BenchRetreatTarget;
 float BenchRetreatRemaining=0;
 FVector BenchRetreatDirection;
 FBattleSleeperTrigger SleeperTrigger;
 void TickSleeperTrigger(float Dt);
 void CancelSleepBehavior();
 void FinishSleeperChase();
 bool TickSleepBehavior(float Dt);
 TWeakObjectPtr<APawn> SleepTarget;
 FVector SleepOrigin;
 FRotator SleepOriginRotation;
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
 bool MoveTo(FVector Goal,float AcceptanceRadius=45.f);
 void ChooseDestination();
 void YieldTo(APawn* Source,bool Horn);
 float ThinkRemaining=0,YieldRemaining=0;
};
