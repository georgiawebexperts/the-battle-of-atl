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
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void ToggleMenu();
private:
 TSharedPtr<SWidget> Menu;
 bool bStarted=false;
 void ShowMenu(FString Page=TEXT("Home"));
 void ResumeRide();
 void RemoveMenu();
};
