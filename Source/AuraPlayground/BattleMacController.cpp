#include "BattleMacController.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

void ABattleMacController::BeginPlay(){
 Super::BeginPlay();
 if(GetWorld()->WorldType==EWorldType::Game)ShowMenu();
}
void ABattleMacController::SetupInputComponent(){
 Super::SetupInputComponent();
 auto& Binding=InputComponent->BindKey(EKeys::Escape,IE_Pressed,this,&ABattleMacController::ToggleMenu);
 Binding.bExecuteWhenPaused=true;
}
void ABattleMacController::RemoveMenu(){
 if(Menu.IsValid()&&GetWorld()&&GetWorld()->GetGameViewport())GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Menu.ToSharedRef());
 Menu.Reset();
}
void ABattleMacController::EndPlay(const EEndPlayReason::Type Reason){RemoveMenu();Super::EndPlay(Reason);}
void ABattleMacController::ResumeRide(){
 RemoveMenu();bStarted=true;SetPause(false);bShowMouseCursor=false;ResetIgnoreMoveInput();ResetIgnoreLookInput();SetInputMode(FInputModeGameOnly());
 FlushPressedKeys();
 UE_LOG(LogTemp,Display,TEXT("BattleMac: ride resumed"));
}
void ABattleMacController::ToggleMenu(){if(Menu.IsValid()&&bStarted)ResumeRide();else ShowMenu();}
void ABattleMacController::ShowMenu(FString Page){
 RemoveMenu();SetPause(true);bShowMouseCursor=true;ResetIgnoreMoveInput();ResetIgnoreLookInput();SetIgnoreMoveInput(true);SetIgnoreLookInput(true);
 TSharedRef<SVerticalBox> Items=SNew(SVerticalBox);
 auto Label=[&](FString Text,int Size,FLinearColor Color){Items->AddSlot().AutoHeight().Padding(0,6)[SNew(STextBlock).Text(FText::FromString(Text)).Font(FCoreStyle::GetDefaultFontStyle("Bold",Size)).ColorAndOpacity(Color).AutoWrapText(true)];};
 auto Button=[&](FString Text,TFunction<void()> Action){Items->AddSlot().AutoHeight().Padding(0,5)[SNew(SButton).ContentPadding(FMargin(18,10)).OnClicked_Lambda([Action](){Action();return FReply::Handled();})[SNew(STextBlock).Text(FText::FromString(Text)).Font(FCoreStyle::GetDefaultFontStyle("Bold",18))]];};
 Label(TEXT("BATTLE FOR THE A"),42,FLinearColor(1,.12,.16));
 Label(bStarted?TEXT("PAUSED"):TEXT("PIEDMONT PARK / MAC PLAYTEST"),16,FLinearColor(1,.7,.35));
 if(Page==TEXT("Instructions")){
  Label(TEXT("Explore the park, dodge visitors, and test your bike and pistol. The Artifact-to-Cabbagetown campaign is still being built."),16,FLinearColor::White);
  Label(TEXT("BIKE\nW pedal | A/D or Left/Right steer\nUp/Down gears | Space brake/drift\nShift nitro | H horn | Tab camera\nE dismount | Left click pistol"),17,FLinearColor::White);
  Label(TEXT("ON FOOT\nWASD / arrows move | Mouse look\nShift sprint | Space jump\nLeft click fire | Right click aim | R reload\nE near bike to remount | Esc pause"),17,FLinearColor::White);
  Button(TEXT("BACK"),[this](){ShowMenu();});
 }else if(Page==TEXT("Options")){
  Label(TEXT("Graphics presets target 1080p with a 60 FPS cap. Actual frame rate depends on the scene."),16,FLinearColor::White);
  for(int Quality:{1,2})Button(Quality==1?TEXT("PERFORMANCE / 1080p"):TEXT("BALANCED / 1080p"),[this,Quality](){if(auto* Settings=UGameUserSettings::GetGameUserSettings()){Settings->SetOverallScalabilityLevel(Quality);Settings->SetScreenResolution(FIntPoint(1920,1080));Settings->SetFullscreenMode(EWindowMode::Windowed);Settings->SetFrameRateLimit(60);Settings->ApplySettings(false);Settings->SaveSettings();}ShowMenu(TEXT("Options"));});
  Button(TEXT("BACK"),[this](){ShowMenu();});
 }else{
  Button(bStarted?TEXT("RESUME RIDE"):TEXT("START PARK RIDE"),[this](){ResumeRide();});
  Button(TEXT("INSTRUCTIONS"),[this](){ShowMenu(TEXT("Instructions"));});
  Button(TEXT("OPTIONS"),[this](){ShowMenu(TEXT("Options"));});
  Button(TEXT("QUIT"),[this](){UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);});
  Label(TEXT("Development build 012 | Web Experts\nPark riding and FPS test. Full route, enemies, difficulty and campaign are not finished."),13,FLinearColor(.65,.68,.72));
 }
 Menu=SNew(SOverlay)+SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.008,.012,.02,.94))]+SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[SNew(SBox).WidthOverride(600)[Items]];
 if(auto* Viewport=GetWorld()->GetGameViewport())Viewport->AddViewportWidgetContent(Menu.ToSharedRef(),100);
 FInputModeGameAndUI Mode;Mode.SetWidgetToFocus(Menu);Mode.SetHideCursorDuringCapture(false);SetInputMode(Mode);
 UE_LOG(LogTemp,Display,TEXT("BattleMac: menu %s"),*Page);
}
