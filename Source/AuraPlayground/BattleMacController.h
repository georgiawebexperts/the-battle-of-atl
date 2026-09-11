#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BattleMacController.generated.h"
class SWidget;
UCLASS()
class AURAPLAYGROUND_API ABattleMacController : public APlayerController {
 GENERATED_BODY()
public:
 virtual void BeginPlay() override;
 virtual void SetupInputComponent() override;
 virtual void PlayerTick(float DeltaTime) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void ToggleMenu();
private:
 TSharedPtr<SWidget> Menu;
 bool bStarted=false;
 void StartDifficulty(FName Name);
 void RunDevelopmentAudit();
 void TickConnectorAudit(float Dt);
 void TickGeographyAudit(float Dt);
 void TickHealthAudit(float Dt);
 void TickPickupAudit(float Dt);
 void TickMeleeAudit(float Dt);
 void TickInventoryAudit(float Dt);
 void TickDiscAudit(float Dt);
 void TickFrisbeeAudit(float Dt);
 void TickHUDReview(float Dt);
 void TickTimeAudit(float Dt);
 void TickFurnitureAudit(float Dt);
 bool bFurnitureAudited=false;
 void TickDroneAudit(float Dt);
 int32 DroneStage=0;float DroneClock=0;TWeakObjectPtr<AActor> AuditDrone,AuditDroneWall;
 void TickJumpAudit(float Dt);
 int32 JumpStage=0;float JumpClock=0,JumpTimeBefore=0;
 void TickSteeringAudit(float Dt);
 int32 SteeringStage=0;float SteeringClock=0,SteeringYaw=0;FVector SteeringStart=FVector::ZeroVector;
 void TickTroubleAudit(float Dt);
 int32 TroubleStage=0;float TroubleClock=0,TroubleTime=0;
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
 FVector FrisbeeGrassStart;
 TWeakObjectPtr<AActor> FrisbeeAuditGroup;
 int32 DiscPhase=0;float DiscClock=0;
 TWeakObjectPtr<AActor> DiscFirst,DiscSecond,DiscProjectile,DiscWall;
 int32 InventoryPhase=0,InventoryShots=0;
 float InventoryClock=0;
 TWeakObjectPtr<AActor> InventoryTarget;
 int32 MeleePhase=0;
 float MeleeAuditClock=0;
 TWeakObjectPtr<AActor> MeleeTarget,MeleeWall;
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
