#include "BattleMacController.h"
#include "BattleBike.h"
#include "Misc/CommandLine.h"
#include "BattleRider.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
void TickBattleBikeBalanceAudit(APlayerController* PC,float Dt);
void TickBattleBridgeTurnAudit(APlayerController* PC,float Dt);
void TickBattleRailScrapeAudit(APlayerController* PC,float Dt);
void TickBattleIncidentBikeAudit(APlayerController* PC,float Dt);
void TickBattleHandlingSlopeAudit(APlayerController* PC,float Dt);
void TickBattleArcadeDownhillAudit(APlayerController* PC,float Dt);
void TickBattleWorldHillAudit(APlayerController* PC,float Dt);
void TickBattleWallRecoveryAudit(APlayerController* PC,float Dt);
void ABattleMacController::TickSteeringAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleBikeBalanceAudit"))){TickBattleBikeBalanceAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleBridgeTurnAudit"))){TickBattleBridgeTurnAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleRailScrapeAudit"))){TickBattleRailScrapeAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleWallRecoveryAudit"))){TickBattleWallRecoveryAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleIncidentBikeAudit"))){TickBattleIncidentBikeAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleHandlingSlopeAudit"))){TickBattleHandlingSlopeAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleArcadeDownhillAudit"))){TickBattleArcadeDownhillAudit(this,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleWorldHillAudit"))){TickBattleWorldHillAudit(this,Dt);return;}
 if(GetWorld()->GetTimeSeconds()<5||SteeringStage==99)return;
 auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());if(!Bike)return;SteeringClock+=Dt;
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleSteeringAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"speed\":%.2f,\"steer\":%.3f,\"lean\":%.3f}"),Pass?TEXT("true"):TEXT("false"),SteeringStage,Reason,Bike->Ride->Speed,Bike->Ride->SmoothedSteer,Bike->LeanAngle);SteeringStage=99;ConsoleCommand(TEXT("quit"));};
#define CHECK_STEERING(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Next=[&](){SteeringStage++;SteeringClock=0;};
 if(SteeringStage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  float Low=0,High=0;for(int I=0;I<30;I++)Low=UBattleBikeMovement::SteeringResponse(Low,1,1.f/30);for(int I=0;I<120;I++)High=UBattleBikeMovement::SteeringResponse(High,1,1.f/120);
  CHECK_STEERING(FMath::Abs(Low-High)<.0001f&&Low>.99f,"Steering response differs by frame rate");
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingAudit"))){Key(EKeys::P,true);Key(EKeys::P,false);}
  SteeringStart=Bike->GetActorLocation();Key(EKeys::Up,true);Next();
 }else if(SteeringStage==1&&SteeringClock>.65f){if(FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingAudit")))CHECK_STEERING(Bike->Ride->bRealHandling,"P did not select realistic handling");CHECK_STEERING(Bike->Ride->Speed>250&&Bike->Ride->Gear==1&&FVector::Dist2D(SteeringStart,Bike->GetActorLocation())>70,"Up did not pedal or changed gear");Key(EKeys::Up,false);Key(EKeys::Down,true);Next();}
 else if(SteeringStage==2&&SteeringClock>.65f){CHECK_STEERING(Bike->Ride->Speed<10&&Bike->Ride->Gear==1,"Down did not brake or changed gear");Key(EKeys::Down,false);Key(EKeys::W,true);Next();}
 else if(SteeringStage==3&&SteeringClock>.65f){CHECK_STEERING(Bike->Ride->Speed>250,"W pedal failed");SteeringYaw=Bike->GetActorRotation().Yaw;Key(EKeys::Right,true);Next();}
 else if(SteeringStage==4&&SteeringClock>.08f){CHECK_STEERING(Bike->Ride->SmoothedSteer>0&&Bike->Ride->SmoothedSteer<.85f,"Right steering snapped to full lock");Key(EKeys::Right,false);Key(EKeys::A,true);Next();}
 else if(SteeringStage==5){if(SteeringClock<.08f)CHECK_STEERING(Bike->Ride->SmoothedSteer>-.9f,"Direction reversal snapped");if(SteeringClock>.65f){CHECK_STEERING(Bike->Ride->SmoothedSteer<-.9f&&Bike->LeanAngle<0&&FMath::Abs(FMath::FindDeltaAngleDegrees(SteeringYaw,Bike->GetActorRotation().Yaw))>2&&Bike->Ride->Recovery==0,"Left steering/lean/movement failed");Key(EKeys::A,false);Key(EKeys::W,false);Key(EKeys::S,true);Next();}}
 else if(SteeringStage==6&&SteeringClock>.65f){CHECK_STEERING(Bike->Ride->Speed<10,"S brake failed");Key(EKeys::S,false);Key(EKeys::R,true);Key(EKeys::R,false);Next();}
 else if(SteeringStage==7&&SteeringClock>.15f){CHECK_STEERING(Bike->Ride->Gear==2,"R gear up failed");Key(EKeys::Q,true);Key(EKeys::Q,false);Next();}
 else if(SteeringStage==8&&SteeringClock>.15f){CHECK_STEERING(Bike->Ride->Gear==1&&Bike->Dismount(),"Q gear down or dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_STEERING(Person&&Person->MountBike()&&Bike->Ride->SmoothedSteer==0&&Bike->LeanAngle==0,"Remount retained steering/lean");SteeringYaw=Bike->GetActorRotation().Yaw;Key(EKeys::Left,true);Next();}
 else if(SteeringStage==9&&SteeringClock>.65f){CHECK_STEERING(Bike->Ride->SmoothedSteer<-.8f,"Stationary steering input did not settle");CHECK_STEERING(FMath::Abs(FMath::FindDeltaAngleDegrees(SteeringYaw,Bike->GetActorRotation().Yaw))<.01f,"Stationary bike pivoted in place");Key(EKeys::Left,false);if(FParse::Param(FCommandLine::Get(),TEXT("BattleRealHandlingAudit"))){CHECK_STEERING(Bike->Ride->bRealHandling,"Remount lost handling mode");Key(EKeys::P,true);Key(EKeys::P,false);Next();return;}Finish(true,TEXT("Actual arrow/WASD pedal/brake, eased steering reversal and lean, Q/R gears, remount reset, stationary heading and 30/120Hz response pass"));}
 else if(SteeringStage==10&&SteeringClock>.15f){CHECK_STEERING(!Bike->Ride->bRealHandling,"P did not return to arcade handling");Finish(true,TEXT("Realistic handling native controls, lean, remount persistence and P toggle both ways passed"));}
 if(SteeringClock>8&&SteeringStage!=99)Finish(false,TEXT("Steering audit timeout"));
#undef CHECK_STEERING
#endif
}
