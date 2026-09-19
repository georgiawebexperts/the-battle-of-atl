#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Styling/SlateBrush.h"
#include "BattleMacController.generated.h"
class SWidget;
UCLASS()
class AURAPLAYGROUND_API ABattleMacController : public APlayerController {
 GENERATED_BODY()
public:
 ABattleMacController();
 virtual void BeginPlay() override;
 virtual void SetupInputComponent() override;
 virtual void PlayerTick(float DeltaTime) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void ToggleMenu();
 void TogglePracticeHelp();
 void CycleRiderStyle();
 // Player-facing aim sensitivity, adjustable in Options or with [ and ].
 void AdjustAim(int32 Delta);
 void AimSensitivityDown(){AdjustAim(-1);}
 void AimSensitivityUp(){AdjustAim(1);}
 UPROPERTY(BlueprintReadOnly) FString AimNotice;
 UPROPERTY(BlueprintReadOnly) float AimNoticeRemaining=0;
private:
 void TickSpiritAudit(float Dt);
 void TickMurderKAudit(float Dt);
 void TickKnifeAudit(float Dt);
 int KnifeAuditStage=0,KnifeAuditResets=0;float KnifeAuditClock=0,KnifeAuditTime=0;
 FVector KnifeAuditOrigin;
 int KnifeAuditShots=0,KnifeAuditAmmo=0;float KnifeAuditTrouble=0;
 bool bKnifeWindupCaptured=false,bKnifeStabCaptured=false,bKnifeDefenseCaptured=false;
 UPROPERTY() TObjectPtr<class ABattleKnife> KnifeAuditActor;
 void TickMemorialReview(float Dt);
 int MemorialReviewStage=0;float MemorialReviewClock=0;
 UPROPERTY() TObjectPtr<class ACameraActor> MemorialReviewCamera;
 bool bSpiritAuditDone=false;
int32 SpiritReviewFrames=0;
 float SpiritReviewClock=0.f;
 int32 SpiritReviewShot=0;
 bool bSpiritReviewTookThreeQuarter=false;
 bool bSpiritReviewTookApproach=false;
 bool bSpiritReviewMoved=false;
UPROPERTY() TObjectPtr<class ACameraActor> SpiritReviewCamera;
 UPROPERTY() TObjectPtr<class UTextureRenderTarget2D> SpiritReviewTarget;
 UPROPERTY() TObjectPtr<class USceneCaptureComponent2D> SpiritReviewCapture;
 bool bMurderKAuditDone=false;
 int32 MurderKReviewFrames=0;
 UPROPERTY() TObjectPtr<class ACameraActor> MurderKReviewCamera;
 TSharedPtr<SWidget> Menu;
 UPROPERTY() TObjectPtr<class ACameraActor> OpeningCamera;
 bool bOpeningActive=false,bOpeningSeen=false,bOpeningOldCameraMoveable=false;
 FVector OpeningInitialEye=FVector::ZeroVector;
 double OpeningStart=0;
 FVector OpeningRiderLocation=FVector::ZeroVector;
 float OpeningTimer=0;
 int32 OpeningCaptureStage=0;
 void BeginOpening();
 void FinishOpening();
 void SkipOpening();
 UPROPERTY() TObjectPtr<class UTexture2D> CelebrationArt;
 FSlateBrush CelebrationBrush;
 bool bCelebrating=false,bCelebrationSeen=false,bCredits=false;
 double CelebrationStart=0,CreditsStart=0;
 bool bStarted=false,bFocusPauseIssued=false;
 void StartDifficulty(FName Name);
 void RunDevelopmentAudit();
 void ApplyFocusState(bool bActive);
 void TickStorefrontAudit(float Dt);
 void TickTutorialAudit(float Dt);
 void TickFootAudit(float Dt);
 int32 FootStage=0,FootCaptures=0;float FootClock=0,FootWalkSpeed=0,FootRunSpeed=0,FootArmMin=MAX_flt,FootArmMax=-MAX_flt,FootJumpBase=0;
 FVector FootStart;TWeakObjectPtr<AActor> FootCeiling;
 int TutorialStage=0;float TutorialClock=0,TutorialDistance=0,TutorialMaxError=0;FVector TutorialPrevious;
 void TickConnectorAudit(float Dt);
 void TickGeographyAudit(float Dt);
 void TickHealthAudit(float Dt);
 void TickPickupAudit(float Dt);
 void TickMeleeAudit(float Dt);
 void TickStreakAudit(float Dt);
 void TickInventoryAudit(float Dt);
 void TickDiscAudit(float Dt);
 void TickFrisbeeAudit(float Dt);
 void TickHUDReview(float Dt);
 void TickTimeAudit(float Dt);
 void TickFurnitureAudit(float Dt);
 bool bFurnitureAudited=false;
 void TickDroneAudit(float Dt);
 int32 DroneStage=0;float DroneClock=0;int32 DroneSettleTicks=0,DroneShotAttempts=0;TWeakObjectPtr<AActor> AuditDrone,AuditDroneWall;
 void TickSkaterReview(float Dt);
 TWeakObjectPtr<AActor> SkateReviewActor,SkateReviewCamera;int32 SkateReviewFrames=0;
 void TickSkaterAudit(float Dt);
 void TickHornAudit(float Dt);
 void TickFinishAudit(float Dt);
 bool FinishAuditDone=false;
 int32 HornStage=0,HornPresses=0;float HornAuditClock=0;TWeakObjectPtr<AActor> HornReviewActor;
 int32 SkaterStage=0,SkaterInitialWipeouts=0;float SkaterClock=0,SkaterMaxError=0;bool SkaterPushSeen=false,SkaterCoastSeen=false;TWeakObjectPtr<AActor> SkaterTarget;
 /** The other skater: the realistic half of the contact test needs a victim the
  *  assault record has not already counted. */
 TWeakObjectPtr<AActor> SkaterSecond;
 /** Where the contact fixture put the bike, so the probe can report how far it
  *  actually travelled rather than only how fast it thinks it is going. */
 FVector SkaterContactStart=FVector::ZeroVector;
 void TickSkateAudit(float Dt);
 int32 SkateStage=0;float SkateClock=0,SkateTime=0;
 void TickJumpAudit(float Dt);
 int32 JumpStage=0;float JumpClock=0,JumpTimeBefore=0;
 void TickSteeringAudit(float Dt);
 int32 SteeringStage=0;float SteeringClock=0,SteeringYaw=0;FVector SteeringStart=FVector::ZeroVector;
 void TickTroubleAudit(float Dt);
 int32 TroubleStage=0;float TroubleClock=0,TroubleTime=0,TroubleAirStartZ=0;
 TWeakObjectPtr<AActor> TroubleOfficer;
 FVector TroublePoliceStart=FVector::ZeroVector;
 void TickAmmoAudit(float Dt);
 int32 AmmoStage=0,AmmoShots=0;float AmmoClock=0;
 int32 TimeAuditStage=0;float TimeAuditClock=0;
 TWeakObjectPtr<AActor> TimeAuditTarget;
 void TickLocomotionReview(float Dt);
 float LocoClock=0,LocoKneeMotion=0;int32 LocoStage=0;
 FQuat LocoInitialKnee;FVector LocoStart;
 TWeakObjectPtr<AActor> LocoPerson,LocoCamera;
 float HUDReviewClock=0;int32 HUDReviewStage=0;
 int32 FrisbeePhase=0,FrisbeeInitialWipeouts=0;float FrisbeeClock=0,FrisbeeGrassSeconds=0,FrisbeePeakSpeed=0;
 /** Gear and handling at the moment the speed peak was set: the top-speed half
  *  of the grass ride is a gear question, not only a speed one. */
 int32 FrisbeePeakGear=0;bool FrisbeePeakRealistic=false;
 FVector FrisbeeGrassStart;
 TWeakObjectPtr<AActor> FrisbeeAuditGroup;
 int32 DiscPhase=0;float DiscClock=0;
 TWeakObjectPtr<AActor> DiscFirst,DiscSecond,DiscProjectile,DiscWall;
 int32 InventoryPhase=0,InventoryShots=0;
 float InventoryClock=0;
 TWeakObjectPtr<AActor> InventoryTarget;
 int32 MeleePhase=0;
 int32 StreakPhase=0;
 float MeleeAuditClock=0;
 float StreakAuditClock=0;
 TWeakObjectPtr<AActor> MeleeTarget,MeleeWall;
 TWeakObjectPtr<AActor> StreakTarget;
 int32 PickupPhase=0;
 float PickupAuditClock=0;
 TWeakObjectPtr<AActor> PickupAuditTarget,PickupAuditWall;
 void TickZombieAudit(float Dt);
 void TickZombiePopulationAudit(float Dt);
 int32 ZombiePhase=0,ZombieShots=0;
 float ZombieAuditClock=0;
 FVector ZombieAuditInitial;
 TWeakObjectPtr<AActor> ZombieAuditTarget,ZombieAuditCorpse;
 int32 HealthPhase=0;
 float HealthAuditClock=0,HealthDeathTime=0;
 FVector HealthArtifactLocation;
 TWeakObjectPtr<APawn> HealthFormerRider;
 int32 GeographyPhase=0,GeographyInitialWipeouts=0;
 float GeographyClock=0;
 TArray<FVector> ConnectorPoints;
 float ConnectorElapsed=0,ConnectorTravel=0,ConnectorMaxError=0;
 int32 ConnectorLeg=0,ConnectorWipeouts=0,TrailGroundSamples=0,TrailPavedSamples=0;
 FVector ConnectorPrevious;
 float TunnelDarkTime=0;
 int32 TunnelLitSamples=0,TunnelUnlitSamples=0;
 void ShowMenu(FString Page=TEXT("Home"));
 void ResumeRide();
 void RemoveMenu();
};
