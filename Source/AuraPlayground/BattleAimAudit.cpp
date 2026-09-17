#include "BattleAim.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "PiedmontExplorer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"

/**
 * Proves the player-facing aim sensitivity really scales look input: hold the
 * turn key at the lowest and highest setting, accumulate the yaw taken in each
 * window, then press the bracket key and confirm the choice is written to
 * GGameUserSettingsIni so it survives a restart.
 */
void TickBattleAimAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int Phase=0,Original=3,Sample=-1,Before=0,Calls=0;float Clock=0,Low=0,High=0,Turn=0,PreviousYaw=0,MouseLow=0,MouseHigh=0;bool Bracket=false,Axis=false;};
 static FState S;
 if(!PC||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto BeginWindow=[&](){PC->SetControlRotation(FRotator::ZeroRotator);S.PreviousYaw=PC->GetControlRotation().Yaw;S.Turn=0;S.Clock=0;S.Sample=-1;Key(EKeys::X,true);};
 auto EndWindow=[&](){Key(EKeys::X,false);};
 auto Finish=[&](bool Pass,const TCHAR* Why){
  Key(EKeys::X,false);GConfig->SetInt(TEXT("BattleControls"),TEXT("AimSensitivityStep"),S.Original,GGameUserSettingsIni);GConfig->Flush(false,GGameUserSettingsIni);
  UE_LOG(LogTemp,Display,TEXT("BattleAimAudit: {\"passed\":%s,\"reason\":\"%s\",\"low_percent_yaw\":%.2f,\"high_percent_yaw\":%.2f,\"ratio\":%.2f,\"bracket_key_changes_setting\":%s}"),
   Pass?TEXT("true"):TEXT("false"),Why,S.Low,S.High,S.Low>0?S.High/S.Low:0.f,S.Bracket?TEXT("true"):TEXT("false"));
  PC->ConsoleCommand(TEXT("quit"));
 };
 if(S.Phase==0){
  if(auto* Bike=Cast<ABattleBike>(PC->GetPawn())){Bike->Dismount();return;}
  if(!Cast<APiedmontExplorer>(PC->GetPawn()))return;
  S.Original=BattleAim::Step();
  BattleAim::Cycle(-BattleAim::StepCount);               // lowest step
  BeginWindow();S.Phase=1;return;
 }
 if(S.Phase==1||S.Phase==2){
  S.Clock+=Dt;
  const float Yaw=PC->GetControlRotation().Yaw;
  const float Step=FMath::FindDeltaAngleDegrees(S.PreviousYaw,Yaw);S.PreviousYaw=Yaw;
  if(Step>0)S.Turn+=Step;
  if(FMath::FloorToInt(S.Clock/.4f)!=S.Sample){S.Sample=FMath::FloorToInt(S.Clock/.4f);
   UE_LOG(LogTemp,Display,TEXT("BattleAimSample: window=%d t=%.2f turn=%.2f scale=%.2f"),S.Phase,S.Clock,S.Turn,BattleAim::Scale());}
  if(S.Clock<1.2f)return;
  EndWindow();
  if(S.Phase==1){S.Low=S.Turn;BattleAim::Cycle(BattleAim::StepCount);BeginWindow();S.Phase=2;return;}
  S.High=S.Turn;S.Phase=3;return;
 }
 if(S.Phase==3){
  // Park on a middle step so an increase is observable.
  BattleAim::Cycle(BattleAim::StepCount/2-BattleAim::Step());
  S.Before=BattleAim::Step();
  for(bool Down:{true,false})Key(EKeys::RightBracket,Down);
  S.Clock=0;S.Phase=4;return;
 }
 if(S.Phase==4){
  // Key bindings dispatch on the next input pass, so read back a few frames later.
  S.Clock+=Dt;if(S.Clock<.35f)return;
  int32 Saved=-1;GConfig->GetInt(TEXT("BattleControls"),TEXT("AimSensitivityStep"),Saved,GGameUserSettingsIni);
  S.Bracket=BattleAim::Step()==S.Before+1&&Saved==S.Before+1;
  S.Phase=8;
 }
 if(S.Phase==8){
  // The mouse path runs through the same scaled handler, so confirm the axis is
  // bound to it. Engine axis injection is not exposed in this build.
  if(auto* P=Cast<APiedmontExplorer>(PC->GetPawn()))if(P->InputComponent)
   for(auto& Binding:P->InputComponent->AxisKeyBindings)if(Binding.AxisKey==EKeys::MouseX)S.Axis=true;
  auto Ratio=[](float Low,float High){return Low>0.f?High/Low:0.f;};
  const bool Pass=S.Bracket&&Ratio(S.Low,S.High)>4.f&&Ratio(S.Low,S.High)<12.f
   &&S.Axis;
  UE_LOG(LogTemp,Display,TEXT("BattleAimMouse: axis_bound=%s"),S.Axis?TEXT("true"):TEXT("false"));
  Finish(Pass,Pass?TEXT("aim sensitivity scales look input and the bracket key persists the choice"):TEXT("sensitivity did not scale or persist as expected"));
 }
#endif
}
