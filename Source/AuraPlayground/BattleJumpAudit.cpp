#include "BattleMacController.h"
#include "BattleBike.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
void ABattleMacController::TickJumpAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||JumpStage==99)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode)return;JumpClock+=Dt;
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BattleJumpAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"rewards\":%d,\"peak_cm\":%.2f,\"air_seconds\":%.3f}"),Pass?TEXT("true"):TEXT("false"),JumpStage,Why,Bike->Ride->AirRewards,Bike->Ride->AirPeak,Bike->Ride->AirSeconds);JumpStage=99;ConsoleCommand(TEXT("quit"));};
#define JCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){JumpStage++;JumpClock=0;};
 if(JumpStage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  JCHECK(!Bike->Ride->Hop(),"Stationary hop allowed");Bike->Ride->Gear=2;Key(EKeys::W,true);Next();
 }else if(JumpStage==1&&JumpClock>1.5f){
  JCHECK(Bike->Ride->Speed>500,"Failed to reach takeoff speed");JumpTimeBefore=Mode->TimeRemaining;Key(EKeys::J,true);Key(EKeys::J,false);Next();
 }else if(JumpStage==2&&JumpClock>.15f){
  JCHECK(Bike->Ride->IsFalling()&&Bike->Ride->AirPeak>50,"J did not produce real airborne movement");JCHECK(!Bike->Ride->Hop(),"Midair jump allowed");Next();
 }else if(JumpStage==3&&Bike->Ride->IsMovingOnGround()){
  JCHECK(Bike->Ride->AirRewards==1&&Mode->TimeRemaining>JumpTimeBefore+8,"Landing did not award exactly ten seconds");JCHECK(Bike->Ride->AirPeak>65&&Bike->Ride->Recovery==0,"Jump clearance or clean landing failed");Key(EKeys::W,false);Next();
 }else if(JumpStage==4&&JumpClock>.4f){
  JCHECK(Bike->Ride->AirRewards==1,"Grounded reward repeated");Bike->StunRemaining=2;JCHECK(!Bike->Ride->Hop(),"Stunned hop allowed");Bike->StunRemaining=0;Mode->bRunEnded=true;JCHECK(!Bike->Ride->Hop(),"Ended run hop allowed");Mode->bRunEnded=false;
  Finish(true,TEXT("Actual J takeoff, clearance, landing +10 once, stationary/midair/stun/end guards pass"));
 }
 if(JumpClock>8&&JumpStage!=99)Finish(false,TEXT("Jump stage timeout"));
#undef JCHECK
#endif
}
