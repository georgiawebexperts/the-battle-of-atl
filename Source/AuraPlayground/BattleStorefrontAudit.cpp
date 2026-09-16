#include "BattleMacController.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/CommandLine.h"
#include "UnrealClient.h"
#include "Containers/Ticker.h"

void ABattleMacController::TickStorefrontAudit(float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;bool Started=false,Done=false,FocusPause=false,VideoSettings=false;FIntPoint Resolution;EWindowMode::Type WindowMode=EWindowMode::Windowed;};static FState S;
 if(S.World!=GetWorld()){S=FState();S.World=GetWorld();}if(S.Done||GetWorld()->GetTimeSeconds()<5)return;
 if(!S.Started){
  auto* Settings=UGameUserSettings::GetGameUserSettings();if(!Settings)return;
  S.Resolution=Settings->GetScreenResolution();S.WindowMode=Settings->GetFullscreenMode();
  RemoveMenu();SetPause(false);bStarted=true;bOpeningActive=false;bFocusPauseIssued=false;ApplyFocusState(false);
  S.FocusPause=IsPaused()&&Menu.IsValid()&&bFocusPauseIssued;
  Settings->SetScreenResolution(FIntPoint(1600,900));Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
  S.VideoSettings=Settings->GetScreenResolution()==FIntPoint(1600,900)&&Settings->GetFullscreenMode()==EWindowMode::WindowedFullscreen;
  Settings->SetScreenResolution(S.Resolution);Settings->SetFullscreenMode(S.WindowMode);
  ShowMenu(TEXT("Options"));
  FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleStorefrontReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/TEXT("options.png"),true,false);
  const bool Pass=S.FocusPause&&S.VideoSettings&&IsPaused()&&Menu.IsValid();
  UE_LOG(LogTemp,Display,TEXT("BattleStorefrontAudit: {\"passed\":%s,\"focus_pause\":%s,\"video_settings\":%s,\"options_menu\":%s}"),Pass?TEXT("true"):TEXT("false"),S.FocusPause?TEXT("true"):TEXT("false"),S.VideoSettings?TEXT("true"):TEXT("false"),(IsPaused()&&Menu.IsValid())?TEXT("true"):TEXT("false"));
  S.Started=S.Done=true;
  FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Weak=TWeakObjectPtr<ABattleMacController>(this)](float){if(Weak.IsValid())Weak->ConsoleCommand(TEXT("quit"));return false;}),1.f);return;
 }
#endif
}
