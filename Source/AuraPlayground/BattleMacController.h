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
