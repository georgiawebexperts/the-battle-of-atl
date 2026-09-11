#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PiedmontBike.h"
#include "BattleDifficulty.h"
#include "BattleInventory.h"
#include "BattleBike.generated.h"
class ABattleQuest;class ABattleEnemyDirector;class ABattlePickupDirector;
class ABattleRideFX;class UAudioComponent;
class USpotLightComponent;class UPointLightComponent;
class ABattleRider;
class UPoseableMeshComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class AURAPLAYGROUND_API UBattleBikeMovement : public UCharacterMovementComponent {
 GENERATED_BODY()
public:
 UBattleBikeMovement();
 virtual void TickComponent(float Dt,ELevelTick TickType,FActorComponentTickFunction* Function) override;
 virtual void CalcVelocity(float Dt,float Friction,bool Fluid,float Braking) override;
 virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode,uint8 PreviousCustomMode) override;
 bool Hop();
 int32 AirRewards=0;
 float AirSeconds=0,AirPeak=0;
 bool bRewardableAir=false;
 FVector AirOrigin;
 virtual void HandleImpact(const FHitResult& Hit,float TimeSlice,const FVector& MoveDelta) override;
 UPROPERTY(BlueprintReadOnly) float Speed=0;
 UPROPERTY(BlueprintReadOnly) float SmoothedSteer=0;
 static float SteeringResponse(float Current,float Target,float Dt){return FMath::Lerp(Current,FMath::Clamp(Target,-1.f,1.f),1.f-FMath::Exp(-8.f*FMath::Max(0.f,Dt)));}
 UPROPERTY(BlueprintReadOnly) int32 Gear=1;
 UPROPERTY(BlueprintReadOnly) float Recovery=0;
 UPROPERTY(BlueprintReadOnly) int32 Wipeouts=0;
 UPROPERTY(BlueprintReadOnly) FString RecoveryReason;
 UPROPERTY(BlueprintReadOnly) bool bGrass=false;
 UPROPERTY(BlueprintReadOnly) float SlideRemaining=0;
 UPROPERTY(BlueprintReadOnly) float BoostRemaining=0;
 float Pedal=0,Steer=0,Brake=0,Cadence=0;
 FVector LastSafeLocation;
 void Wipeout(const FString& Reason,bool Water=false);
 void Shift(int32 Delta){Gear=FMath::Clamp(Gear+Delta,1,5);}
private:
 bool bWaterReturn=false;
 float BounceRemaining=0,ContactCooldown=0,PreviousBrake=0,PreviousSteer=0;
 FVector BounceDirection,ReturnLocation;
};

UCLASS()
class AURAPLAYGROUND_API ABattleBike : public ACharacter {
 GENERATED_BODY()
public:
 ABattleBike(const FObjectInitializer& Init);
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 virtual void Tick(float Dt) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UBattleBikeMovement> Ride;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<ABattleRideFX> RideEffects;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UAudioComponent> AsphaltAudio;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UAudioComponent> GrassAudio;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UAudioComponent> MotorAudio;
 UPROPERTY(BlueprintReadOnly) float FeedbackStrength=0;
 UPROPERTY(BlueprintReadOnly) int32 ImpactEvents=0;
 UPROPERTY(BlueprintReadOnly) int32 TerrainBumps=0;
 UPROPERTY(BlueprintReadOnly) int32 SkidSounds=0;
 void RideImpact(float Strength,bool Water=false);
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCapsuleComponent> Capsule;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Visual;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UPoseableMeshComponent> Rider;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Arm;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Chase;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Handlebar;
 UPROPERTY(BlueprintReadOnly) bool bFirstPerson=false;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USpotLightComponent> Headlight;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> TailLight;
 UPROPERTY(BlueprintReadOnly) bool bLightsOn=false;
 UPROPERTY(BlueprintReadOnly) int32 HornCount=0;
 UFUNCTION(BlueprintCallable) void Horn();
 UPROPERTY(BlueprintReadOnly) float Nitro=0;
 UPROPERTY(BlueprintReadOnly) int32 NearMisses=0;
 UPROPERTY(BlueprintReadOnly) int32 EnemyKills=0;
 UPROPERTY(BlueprintReadOnly) int32 ShotsFired=0;
 UPROPERTY(BlueprintReadOnly) float HitFeedback=0;
 UPROPERTY(BlueprintReadOnly) float PistolSpread=0;
 UPROPERTY(BlueprintReadOnly) float LeanAngle=0;
 UPROPERTY(BlueprintReadOnly) FVector LastShotEnd;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Pistol;
 UFUNCTION(BlueprintCallable) bool FirePistol();
 UFUNCTION(BlueprintCallable) bool Boost();
 void AwardEnemyKill(){EnemyKills++;Nitro=FMath::Min(100.f,Nitro+25);}
 UPROPERTY(BlueprintReadOnly) bool bParked=false;
 UPROPERTY(BlueprintReadOnly) float RiderHealth=100;
 UPROPERTY(BlueprintReadOnly) float HurtCooldown=0;
 UPROPERTY(BlueprintReadOnly) float RespawnRemaining=0;
 UPROPERTY(BlueprintReadOnly) float DamageGrace=0;
 UPROPERTY(BlueprintReadOnly) FTransform CheckpointTransform;
 UPROPERTY(BlueprintReadOnly) FString CheckpointName=TEXT("14th Street Gate");
 UPROPERTY(BlueprintReadOnly) int32 Deaths=0;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 float ApplyRiderDamage(float Amount);
 bool ApplyTaser();
 void UpdateStun(float Dt);
 UPROPERTY(BlueprintReadOnly) float StunRemaining=0,TaserGrace=0;
 UPROPERTY(BlueprintReadOnly) int32 TaserHits=0;

 UFUNCTION(BlueprintCallable) float RestoreRiderHealth(float Amount);
 UPROPERTY(BlueprintReadOnly) int32 HealthPickups=0;
 UPROPERTY(BlueprintReadOnly) float PickupNoticeRemaining=0;
 UPROPERTY(BlueprintReadOnly) float LastHealAmount=0;
 void UpdateHealth(float Dt);
 bool RecoverAtCheckpoint();
 UPROPERTY(BlueprintReadOnly) int32 PistolAmmo=10;
 UPROPERTY(BlueprintReadOnly) TArray<FBattleWeaponState> Inventory;
 UPROPERTY(BlueprintReadOnly) int32 LastFootWeapon=0;
 UFUNCTION(BlueprintCallable) bool GiveWeapon(int32 Slot,int32 Rounds);
 UFUNCTION(BlueprintCallable) bool Dismount();
 bool Remount(ABattleRider* Person);
 UFUNCTION(BlueprintCallable) void ToggleCamera();
 UFUNCTION(BlueprintCallable) void ValidationKey(FName Key,bool Pressed);
 FVector FindPathReturn() const;
private:
 void UpdateLights(float Dt);
 void UpdateRideFeedback(float Dt);
 FVector PreviousFeedbackLocation,PreviousTrackPoint;
 float FeedbackClock=0,BumpCooldown=0,PreviousVertical=0,TrackDelay=0;
 bool WasSliding=false,HasTrackPoint=false;
 float HornCooldown=0,LightOffDelay=0,LightCheck=0;
 void StartBoost(){Boost();}
 void HopBike(){Ride->Hop();}
 void Interact(){Dismount();}
 void UpdateNearMisses();
 TMap<TWeakObjectPtr<AActor>,FVector2D> Passes;
 TMap<TWeakObjectPtr<AActor>,float> RewardTimes;
 float ShotCooldown=0,ReloadTimer=0,GunHold=0;
 void PoseRider(float Dt);
 void GearUp(){Ride->Shift(1);}void GearDown(){Ride->Shift(-1);}
 TObjectPtr<UStaticMeshComponent> FrontWheel,RearWheel;
 TArray<FTransform> ReferencePose;TArray<int32> Parents;TArray<FName> BoneNames;
 float WheelAngle=0;
};
UCLASS()
class AURAPLAYGROUND_API ABattleLabMode : public APiedmontRideMode {
 GENERATED_BODY()
public:
 ABattleLabMode();
 UFUNCTION(BlueprintCallable) bool AdjustRunTime(float Seconds,const FString& Reason);
 void RecordPlayerShotHit(AActor* Victim);
 void RecordGunfire();
 bool RecordAssault(AActor* Victim);
 void TickTrouble(float Dt);
 UPROPERTY(BlueprintReadOnly) float Trouble=0;
 UPROPERTY(BlueprintReadOnly) int32 PeopleHit=0,PoliceSpawned=0;
 UPROPERTY(BlueprintReadOnly) bool bPoliceAlert=false;
 TSet<TWeakObjectPtr<AActor>> AssaultVictims;
 float QuietTime=0,PoliceDelay=0;

 UPROPERTY(BlueprintReadOnly) float TimeNoticeRemaining=0,LastTimeDelta=0;
 UPROPERTY(BlueprintReadOnly) FString TimeNotice;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float FootTimeMultiplier=1.25f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FString CourseLabel=TEXT("ARCADE BIKE TEST");
 virtual void StartPlay() override;
 virtual void Tick(float Dt) override;
};
UCLASS()
class AURAPLAYGROUND_API ABattleLabHUD : public AHUD {
 GENERATED_BODY()
public:
 virtual void DrawHUD() override;
 UPROPERTY(Transient) TObjectPtr<UFont> ReadableFont;
};

UCLASS()
class AURAPLAYGROUND_API ABattleParkMode : public ABattleLabMode {
 GENERATED_BODY()
public:
 ABattleParkMode();
 virtual void InitGame(const FString& MapName,const FString& Options,FString& ErrorMessage) override;
 virtual void StartPlay() override;
 UPROPERTY(BlueprintReadOnly) FName DifficultyName=TEXT("Easy");
 UPROPERTY(BlueprintReadOnly) FBattleDifficultyRow Difficulty;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<ABattleQuest> Quest;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<ABattleEnemyDirector> Enemies;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<ABattlePickupDirector> Pickups;

};
