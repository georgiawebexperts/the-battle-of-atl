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
 int32 GeographyPhase=0,GeographyInitialWipeouts=0;
 float GeographyClock=0;
 TArray<FVector> ConnectorPoints;
 float ConnectorElapsed=0,ConnectorTravel=0,ConnectorMaxError=0;
 int32 ConnectorLeg=0,ConnectorWipeouts=0,TrailGroundSamples=0,TrailPavedSamples=0;
 FVector ConnectorPrevious;
 void ShowMenu(FString Page=TEXT("Home"));
 void ResumeRide();
 void RemoveMenu();
};
