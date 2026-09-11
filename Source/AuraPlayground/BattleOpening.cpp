#include "BattleMacController.h"
#include "BattleBike.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "Containers/Ticker.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"

void ABattleMacController::SkipOpening(){if(bOpeningActive)FinishOpening();}
void ABattleMacController::BeginOpening(){
 if(bOpeningSeen||bOpeningActive)return;
#if !UE_BUILD_SHIPPING
 TArray<FString> Args;FString(FCommandLine::Get()).ParseIntoArrayWS(Args);
 for(const FString& A:Args)if(A.StartsWith(TEXT("-Battle"))&&(A.EndsWith(TEXT("Audit"))||A.EndsWith(TEXT("Review")))&&!A.StartsWith(TEXT("-BattleOpening"))){bOpeningSeen=true;return;}
#endif
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* Bike=Cast<ABattleBike>(GetPawn());
 if(!Mode||!Mode->bTutorialActive||!Bike||!GetWorld()->GetGameViewport())return;
 bOpeningSeen=true;bOpeningActive=true;OpeningStart=FPlatformTime::Seconds();OpeningTimer=Mode->TimeRemaining;OpeningRiderLocation=Bike->GetActorLocation();
 Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=Bike->Ride->Pedal=Bike->Ride->Steer=0;
 OpeningCamera=GetWorld()->SpawnActor<ACameraActor>();if(!OpeningCamera){bOpeningActive=false;return;}
 OpeningCamera->GetCameraComponent()->SetFieldOfView(62);
 const FVector Eye=OpeningRiderLocation+FVector(-450,-650,250),Target=OpeningRiderLocation+FVector(0,0,65);
 OpeningCamera->SetActorLocationAndRotation(Eye,(Target-Eye).Rotation());SetViewTarget(OpeningCamera);OpeningInitialEye=Eye;Bike->RefreshRiderPose();
 bOpeningOldCameraMoveable=GetWorld()->bIsCameraMoveableWhenPaused;GetWorld()->bIsCameraMoveableWhenPaused=true;
 if(PlayerCameraManager)PlayerCameraManager->UpdateCamera(0);
 SetPause(true);FlushPressedKeys();SetIgnoreMoveInput(true);SetIgnoreLookInput(true);bShowMouseCursor=true;SetInputMode(FInputModeGameAndUI());if(GetHUD())GetHUD()->bShowHUD=false;
 auto Caption=[this](){const double T=FPlatformTime::Seconds()-OpeningStart;return FText::FromString(T<4?TEXT("Ellison left his phone in Piedmont Park after a game of frisbee."):T<8?TEXT("His watch has a signal. Somewhere in the park, the phone is still out there."):TEXT("Morgan is waiting at 98 Estoria. Find the phone. Make the party."));};
 const FLinearColor Ink(.025,.035,.045,.96),Gold(1,.72,.42);
 Menu=SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1280).HeightOverride(720)[SNew(SOverlay)
  +SOverlay::Slot().VAlign(VAlign_Top)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Ink).Padding(FMargin(46,24))[SNew(SVerticalBox)
   +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("THE BATTLE OF ATL"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",42)).ColorAndOpacity(FLinearColor::White)]
   +SVerticalBox::Slot().AutoHeight().Padding(0,8)[SNew(STextBlock).Text(FText::FromString(TEXT("MIDTOWN  /  13TH STREET"))).Font(FCoreStyle::GetDefaultFontStyle("Regular",18)).ColorAndOpacity(Gold)]]]
  +SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center).Padding(0,0,38,0)[SNew(SBox).WidthOverride(280)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Ink).Padding(22)[SNew(SVerticalBox)
   +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("ELLISON'S WATCH"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",16)).ColorAndOpacity(Gold)]
   +SVerticalBox::Slot().AutoHeight().Padding(0,14)[SNew(STextBlock).Text(FText::FromString(TEXT("Find My Lost Phone"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",23)).AutoWrapText(true).ColorAndOpacity(FLinearColor::White)]
   +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text_Lambda([this](){return FText::FromString(FPlatformTime::Seconds()-OpeningStart<4?TEXT("Searching for signal..."):TEXT("Signal found\nPiedmont Park"));}).Font(FCoreStyle::GetDefaultFontStyle("Regular",18)).ColorAndOpacity(FLinearColor(.55,1,.8))]]]]
  +SOverlay::Slot().VAlign(VAlign_Bottom)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Ink).Padding(FMargin(46,22))[SNew(SVerticalBox)
   +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text_Lambda(Caption).Font(FCoreStyle::GetDefaultFontStyle("Bold",24)).ColorAndOpacity(FLinearColor::White).AutoWrapText(true)]
   +SVerticalBox::Slot().AutoHeight().Padding(0,14,0,0)[SNew(SHorizontalBox)
    +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::FromString(TEXT("Practice first. The clock starts at the 14th Street gate."))).Font(FCoreStyle::GetDefaultFontStyle("Regular",18)).ColorAndOpacity(Gold)]
    +SHorizontalBox::Slot().AutoWidth()[SNew(SButton).ContentPadding(FMargin(18,9)).OnClicked_Lambda([this](){FinishOpening();return FReply::Handled();})[SNew(STextBlock).Text(FText::FromString(TEXT("SKIP INTRO  /  ENTER"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",16))]]]]]
 ]];
 GetWorld()->GetGameViewport()->AddViewportWidgetContent(Menu.ToSharedRef(),100);
 UE_LOG(LogTemp,Display,TEXT("BattleOpening: started; world paused; timer=%.0f"),OpeningTimer);
 FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Weak=TWeakObjectPtr<ABattleMacController>(this)](float){
  if(!Weak.IsValid()||!Weak->bOpeningActive)return false;
  auto* C=Weak.Get();const double T=FPlatformTime::Seconds()-C->OpeningStart;const float A=FMath::InterpEaseInOut(0.f,1.f,FMath::Clamp(float(T/12),0.f,1.f),2.f);
  const FVector Eye=C->OpeningRiderLocation+FMath::Lerp(FVector(-450,-650,250),FVector(350,-550,180),A),Target=C->OpeningRiderLocation+FVector(0,0,65);
  if(C->OpeningCamera)C->OpeningCamera->SetActorLocationAndRotation(Eye,(Target-Eye).Rotation());
  if(C->PlayerCameraManager)C->PlayerCameraManager->UpdateCamera(0);
#if !UE_BUILD_SHIPPING
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleOpeningReview"))&&C->OpeningCaptureStage<3&&T>2+C->OpeningCaptureStage*4){FString Dir;FParse::Value(FCommandLine::Get(),TEXT("BattleHUDReviewDir="),Dir);FScreenshotRequest::RequestScreenshot(Dir/FString::Printf(TEXT("opening%d.png"),C->OpeningCaptureStage),true,false);C->OpeningCaptureStage++;}
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleOpeningSkip"))&&T>2&&C->OpeningCaptureStage==0){C->OpeningCaptureStage=1;C->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),FParse::Param(FCommandLine::Get(),TEXT("BattleOpeningEscape"))?EKeys::Escape:EKeys::Enter,IE_Pressed,1.f,false,0));}
#endif
  if(T>=12){C->FinishOpening();return false;}return true;
 }));
}
void ABattleMacController::FinishOpening(){
 if(!bOpeningActive)return;
 const float Duration=FPlatformTime::Seconds()-OpeningStart;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 const bool Protected=M&&M->bTutorialActive&&M->StartCountdown==0&&M->RunElapsed==0&&M->TimeRemaining==OpeningTimer&&GetPawn()&&GetPawn()->GetActorLocation().Equals(OpeningRiderLocation,1);
 FVector ActualEye;FRotator ActualView;GetPlayerViewPoint(ActualEye,ActualView);const float CameraTravel=FVector::Distance(ActualEye,OpeningInitialEye);
 GetWorld()->bIsCameraMoveableWhenPaused=bOpeningOldCameraMoveable;bOpeningActive=false;if(GetHUD())GetHUD()->bShowHUD=true;ResumeRide();if(OpeningCamera){OpeningCamera->Destroy();OpeningCamera=nullptr;}
 UE_LOG(LogTemp,Display,TEXT("BattleOpening: finished duration=%.2f protected=%d cameraTravel=%.1f"),Duration,Protected,CameraTravel);
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleOpeningAudit"))||FParse::Param(FCommandLine::Get(),TEXT("BattleOpeningReview"))){
  const bool Skip=FParse::Param(FCommandLine::Get(),TEXT("BattleOpeningSkip"));BeginOpening();const bool Pass=!bOpeningActive&&bOpeningSeen&&CameraTravel>(Skip?10:500)&&Protected&&!IsPaused()&&!Menu.IsValid()&&GetViewTarget()==GetPawn()&&(Skip?Duration<4:Duration>=12);
  UE_LOG(LogTemp,Display,TEXT("BattleOpeningAudit: {\"passed\":%s,\"duration\":%.2f,\"skipped\":%s,\"protected\":%s}"),Pass?TEXT("true"):TEXT("false"),Duration,Skip?TEXT("true"):TEXT("false"),Protected?TEXT("true"):TEXT("false"));
  ConsoleCommand(TEXT("quit"));
 }
#endif
}
