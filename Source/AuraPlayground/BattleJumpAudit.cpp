#include "BattleMacController.h"
#include "BattleBike.h"
#include "UnrealClient.h"
#include "Components/CapsuleComponent.h"
#include "Misc/CommandLine.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
void ABattleMacController::TickJumpAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5||JumpStage==99)return;
 auto* Bike=Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode)return;JumpClock+=Dt;const bool LowSpeed=FParse::Param(FCommandLine::Get(),TEXT("BattleLowSpeedJumpAudit"));
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("BattleJumpAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"rewards\":%d,\"peak_cm\":%.2f,\"air_seconds\":%.3f}"),Pass?TEXT("true"):TEXT("false"),JumpStage,Why,Bike->Ride->AirRewards,Bike->Ride->AirPeak,Bike->Ride->AirSeconds);JumpStage=99;ConsoleCommand(TEXT("quit"));};
#define JCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Capture=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleJumpReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);};
 auto Next=[&](){JumpStage++;JumpClock=0;};
 if(JumpStage==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleHillJumpAudit"))){
   FVector Start(-829.488810,11610.492232,-201.436432),End(-3814.784366,11712.364849,-6.822107);
   if(FParse::Param(FCommandLine::Get(),TEXT("BattleDownhillJumpAudit")))Swap(Start,End);
   FHitResult Ground;FCollisionQueryParams Query(SCENE_QUERY_STAT(HillJumpFixture),true,Bike);
   JCHECK(GetWorld()->LineTraceSingleByChannel(Ground,Start+FVector(0,0,1000),Start-FVector(0,0,1000),ECC_Visibility,Query),"Missing actual hill surface");
   JCHECK(FMath::Abs(Ground.ImpactPoint.Z-Start.Z)<60,"Hill source differs from installed ground");
   Bike->Ride->StopMovementImmediately();Bike->SetActorLocationAndRotation(Ground.ImpactPoint+FVector(0,0,Bike->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+3),(End-Start).Rotation());
   Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bRealHandling=FParse::Param(FCommandLine::Get(),TEXT("BattleRealHillJumpAudit"));
   UE_LOG(LogTemp,Display,TEXT("HillJumpFixture: surface=%s position=%s realistic=%d downhill=%d"),*GetNameSafe(Ground.GetActor()),*Ground.ImpactPoint.ToString(),Bike->Ride->bRealHandling,FParse::Param(FCommandLine::Get(),TEXT("BattleDownhillJumpAudit")));
  }
  JCHECK(!Bike->Ride->Hop(),"Stationary hop allowed");Bike->Ride->Gear=LowSpeed?1:2;Capture(TEXT("takeoff"));Key(EKeys::W,true);Next();
 }else if(JumpStage==1&&JumpClock>(LowSpeed?.45f:1.5f)){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleHillJumpAudit"))){
   const auto& Floor=Bike->Ride->CurrentFloor.HitResult;const FVector N=Floor.ImpactNormal;
   const float Grade=-FVector::DotProduct(Bike->GetActorForwardVector(),N)/FMath::Max(.1f,N.Z);
   JCHECK(Bike->Ride->IsMovingOnGround()&&FMath::Abs(Grade)>.025f,"Takeoff was not on a meaningful real slope");
   UE_LOG(LogTemp,Display,TEXT("HillJumpTakeoff: grade=%.4f floor=%s position=%s"),Grade,*GetNameSafe(Floor.GetActor()),*Bike->GetActorLocation().ToString());
  }
  JCHECK(LowSpeed?(Bike->Ride->Speed>=150&&Bike->Ride->Speed<500):Bike->Ride->Speed>500,"Failed to reach requested takeoff speed");UE_LOG(LogTemp,Display,TEXT("JumpTakeoff: speed=%.2f low_speed=%d"),Bike->Ride->Speed,LowSpeed);JumpTimeBefore=Mode->TimeRemaining;if(FParse::Param(FCommandLine::Get(),TEXT("BattleCrestJumpAudit"))){Bike->Ride->SetMovementMode(MOVE_Falling);JCHECK(Bike->Ride->JumpGraceRemaining>0,"Missing crest jump grace");}Key(EKeys::J,true);Key(EKeys::J,false);Next();
 }else if(JumpStage==2&&JumpClock>.15f){
  JCHECK(Bike->Ride->IsFalling()&&Bike->Ride->AirPeak>50,"J did not produce real airborne movement");JCHECK(!Bike->Ride->Hop(),"Midair jump allowed");Capture(TEXT("airborne"));Next();
 }else if(JumpStage==3&&Bike->Ride->IsMovingOnGround()){
  JCHECK(Bike->Ride->AirRewards==1&&Mode->TimeRemaining>JumpTimeBefore+8,"Landing did not award exactly ten seconds");JCHECK(Bike->Ride->AirPeak>65&&Bike->Ride->Recovery==0,"Jump clearance or clean landing failed");Capture(TEXT("landing"));Key(EKeys::W,false);Next();
 }else if(JumpStage==4&&JumpClock>.4f){
  JCHECK(Bike->Ride->AirRewards==1,"Grounded reward repeated");Bike->StunRemaining=2;JCHECK(!Bike->Ride->Hop(),"Stunned hop allowed");Bike->StunRemaining=0;Mode->bRunEnded=true;JCHECK(!Bike->Ride->Hop(),"Ended run hop allowed");Mode->bRunEnded=false;
  Finish(true,TEXT("Actual J takeoff, clearance, landing +10 once, stationary/midair/stun/end guards pass"));
 }
 if(JumpClock>8&&JumpStage!=99)Finish(false,TEXT("Jump stage timeout"));
#undef JCHECK
#endif
}
