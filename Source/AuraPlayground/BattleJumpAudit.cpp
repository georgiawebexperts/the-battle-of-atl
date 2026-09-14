#include "BattleMacController.h"
#include "BattleBike.h"
#include "UnrealClient.h"
#include "Camera/CameraActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
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
 static float MinLandingHip=10000.f;static bool LandingSaddleClear=true;
 auto UpdateReviewCamera=[&](){
 if(!FParse::Param(FCommandLine::Get(),TEXT("BattleJumpSideReview")))return;
  static TWeakObjectPtr<ACameraActor> Camera;
  const bool Close=FParse::Param(FCommandLine::Get(),TEXT("BattlePedalReview"));
  FVector Eye=Close?Bike->Visual->GetComponentTransform().TransformPosition(FVector(0,-160,80)):Bike->GetActorTransform().TransformPosition(FVector(25,-450,170)),Focus=Close?Bike->Visual->GetComponentTransform().TransformPosition(FVector(-5,0,36)):Bike->GetActorLocation()+FVector(0,0,45);
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleSaddleCloseReview"))){Eye=Bike->Visual->GetComponentTransform().TransformPosition(FVector(65,-150,105));Focus=Bike->Visual->GetComponentTransform().TransformPosition(FVector(-18,0,85));}
  if(!Camera.IsValid())Camera=GetWorld()->SpawnActor<ACameraActor>(Eye,(Focus-Eye).Rotation());
  if(Camera.IsValid()){Camera->SetActorLocationAndRotation(Eye,(Focus-Eye).Rotation());SetViewTarget(Camera.Get());}
 };UpdateReviewCamera();
 auto Key=[&](FKey K,bool Down){InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("LandingSaddleAudit: minimum_hip_z_cm=%.3f clear=%d"),MinLandingHip,LandingSaddleClear);UE_LOG(LogTemp,Display,TEXT("BattleJumpAudit: {\"passed\":%s,\"stage\":%d,\"reason\":\"%s\",\"rewards\":%d,\"peak_cm\":%.2f,\"air_seconds\":%.3f}"),Pass?TEXT("true"):TEXT("false"),JumpStage,Why,Bike->Ride->AirRewards,Bike->Ride->AirPeak,Bike->Ride->AirSeconds);JumpStage=99;ConsoleCommand(TEXT("quit"));};
#define JCHECK(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Capture=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleJumpReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/(FString(Name)+TEXT(".png")),false,false);};
 auto Next=[&](){JumpStage++;JumpClock=0;};
 static float MaxSoleError=0,PedalPhaseStart=0;static int SoleSamples=0;
 if(((JumpStage==1&&JumpClock>.9f)||(JumpStage==2&&JumpClock>.3f)||JumpStage==3||JumpStage==4)&&Bike->Rider->GetBoneIndex(TEXT("ball_l"))>=0){
  for(int Sign:{-1,1}){
   auto* Pedal=Cast<UStaticMeshComponent>(Bike->GetDefaultSubobjectByName(*FString::Printf(TEXT("Pedal%d"),Sign)));
   JCHECK(Pedal,"Missing visible pedal platform");
   const FVector Top=Pedal->GetComponentTransform().TransformPosition(FVector(0,0,50));
   const FVector Sole=Bike->Rider->GetBoneLocation(Sign>0?TEXT("ball_l"):TEXT("ball_r"),EBoneSpaces::WorldSpace)+Bike->Rider->GetComponentTransform().TransformVector(FVector(0,0,-2.184858f-(Sign>0?.750843f:.751438f)));
   const float Error=FVector::Distance(Top,Sole);MaxSoleError=FMath::Max(MaxSoleError,Error);SoleSamples++;
   if(Error>3.f){UE_LOG(LogTemp,Display,TEXT("PedalContactAudit: side=%d error=%.3f phase=%.3f stage=%d"),Sign,Error,Bike->Ride->Cadence,JumpStage);Finish(false,TEXT("Shoe sole lost pedal contact"));return;}
  }
 }

 if(JumpStage==4&&Bike->Rider->GetBoneIndex(TEXT("pelvis"))>=0){
  const FVector Hip=Bike->Visual->GetComponentTransform().InverseTransformPosition(Bike->Rider->GetBoneLocation(TEXT("pelvis"),EBoneSpaces::WorldSpace));
  if(Hip.X<8.f){MinLandingHip=FMath::Min(MinLandingHip,float(Hip.Z));LandingSaddleClear&=Hip.Z>=96.f;}
  for(int Sign:{-1,1}){
   const FVector Grip=Bike->SteeringAssembly->GetComponentTransform().TransformPosition(FVector(31,-Sign*25,115)-Bike->SteeringAssembly->GetRelativeLocation());
   const FVector Wrist=Bike->Rider->GetBoneLocation(Sign>0?TEXT("hand_l"):TEXT("hand_r"),EBoneSpaces::WorldSpace);
   JCHECK(FVector::Distance(Grip,Wrist)<3.f,"Landing pose lost handlebar contact");
  }
 }

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
  JCHECK(!Bike->Ride->Hop(),"Stationary hop allowed");Bike->Ride->Gear=LowSpeed?1:2;UpdateReviewCamera();Capture(TEXT("takeoff"));Key(EKeys::W,true);Next();
 }else if(JumpStage==1&&JumpClock>(LowSpeed?.45f:1.5f)){
  if(FParse::Param(FCommandLine::Get(),TEXT("BattleHillJumpAudit"))){
   const auto& Floor=Bike->Ride->CurrentFloor.HitResult;const FVector N=Floor.ImpactNormal;
   const float Grade=-FVector::DotProduct(Bike->GetActorForwardVector(),N)/FMath::Max(.1f,N.Z);
   JCHECK(Bike->Ride->IsMovingOnGround()&&FMath::Abs(Grade)>.025f,"Takeoff was not on a meaningful real slope");
   UE_LOG(LogTemp,Display,TEXT("HillJumpTakeoff: grade=%.4f floor=%s position=%s"),Grade,*GetNameSafe(Floor.GetActor()),*Bike->GetActorLocation().ToString());
  }
  JCHECK(LowSpeed?(Bike->Ride->Speed>=150&&Bike->Ride->Speed<500):Bike->Ride->Speed>500,"Failed to reach requested takeoff speed");UE_LOG(LogTemp,Display,TEXT("JumpTakeoff: speed=%.2f low_speed=%d"),Bike->Ride->Speed,LowSpeed);JumpTimeBefore=Mode->TimeRemaining;if(FParse::Param(FCommandLine::Get(),TEXT("BattleCrestJumpAudit"))){Bike->Ride->SetMovementMode(MOVE_Falling);JCHECK(Bike->Ride->JumpGraceRemaining>0,"Missing crest jump grace");}Key(EKeys::J,true);Key(EKeys::J,false);Next();
 }else if(JumpStage==2&&JumpClock>.15f){
  JCHECK(Bike->Ride->IsFalling()&&Bike->Ride->AirPeak>50,"J did not produce real airborne movement");JCHECK(!Bike->Ride->Hop(),"Midair jump allowed");
  for(int Sign:{-1,1}){
   const bool Detailed=Bike->Rider->GetBoneIndex(TEXT("pelvis"))>=0;
   const FVector Expected=Bike->SteeringAssembly->GetComponentTransform().TransformPosition(FVector(Detailed?31:26,-Sign*25,115)-Bike->SteeringAssembly->GetRelativeLocation());
   const FVector Wrist=Bike->Rider->GetBoneLocation(Sign>0?(Detailed?TEXT("hand_l"):TEXT("Hand_L")):(Detailed?TEXT("hand_r"):TEXT("Hand_R")),EBoneSpaces::WorldSpace);
   const float Error=FVector::Dist(Expected,Wrist);UE_LOG(LogTemp,Display,TEXT("AirborneGripAudit: side=%d error_cm=%.3f"),Sign,Error);
   JCHECK(Error<3.f,"Airborne pose lost handlebar contact");
  }
  Capture(TEXT("airborne"));Next();
 }else if(JumpStage==3&&Bike->Ride->IsMovingOnGround()){
  JCHECK(Bike->Ride->AirRewards==1&&Mode->TimeRemaining>JumpTimeBefore+8,"Landing did not award exactly ten seconds");JCHECK(Bike->Ride->AirPeak>65&&Bike->Ride->Recovery==0,"Jump clearance or clean landing failed");Key(EKeys::W,false);Next();
 }else if(JumpStage==4){
  static bool Captured=false;if(!Captured&&JumpClock>.1f){Capture(TEXT("landing"));Captured=true;}
  if(JumpClock<=.4f)return;
  JCHECK(LandingSaddleClear,"Landing lowered pelvis into saddle clearance envelope");
  JCHECK(Bike->Ride->AirRewards==1,"Grounded reward repeated");Bike->StunRemaining=2;JCHECK(!Bike->Ride->Hop(),"Stunned hop allowed");Bike->StunRemaining=0;Mode->bRunEnded=true;JCHECK(!Bike->Ride->Hop(),"Ended run hop allowed");Mode->bRunEnded=false;
  UE_LOG(LogTemp,Display,TEXT("PedalContactAudit: max_error_cm=%.3f samples=%d phase_radians=%.3f"),MaxSoleError,SoleSamples,Bike->Ride->Cadence-PedalPhaseStart);
  Finish(true,TEXT("Actual J takeoff, clearance, landing +10 once, stationary/midair/stun/end guards pass"));
 }
 if(JumpClock>8&&JumpStage!=99)Finish(false,TEXT("Jump stage timeout"));
#undef JCHECK
#endif
}
