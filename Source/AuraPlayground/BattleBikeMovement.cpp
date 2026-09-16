#include "BattleBike.h"
#include "Misc/CommandLine.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
UBattleBikeMovement::UBattleBikeMovement(){
 MaxWalkSpeed=2200;MaxAcceleration=800;MaxStepHeight=60;SetWalkableFloorAngle(75);
 bOrientRotationToMovement=false;bUseControllerDesiredRotation=false;bMaintainHorizontalGroundVelocity=true;
 bAlwaysCheckFloor=true;bEnablePhysicsInteraction=false;GravityScale=2;AirControl=1;BrakingDecelerationWalking=0;GroundFriction=0;
 MaxSimulationTimeStep=1.f/120;MaxSimulationIterations=16;
}
void UBattleBikeMovement::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function){
 // A rendering stall must not apply stale steering over a long unseen jump.
 // Bound bike simulation catch-up; the world countdown still uses elapsed time.
 Dt=FMath::Min(Dt,.05f);
 if(Recovery>0||MovementMode==MOVE_None){ReverseSpeed=0;bReverseRequested=false;}
 JumpGraceRemaining=IsMovingOnGround()?.12f:FMath::Max(0.f,JumpGraceRemaining-Dt);
 if(IsFalling()){AirSeconds+=Dt;if(CharacterOwner)AirPeak=FMath::Max(AirPeak,float(CharacterOwner->GetActorLocation().Z-AirOrigin.Z));}
 RampLaunchGrace=FMath::Max(0.f,RampLaunchGrace-Dt);
 if(IsMovingOnGround()&&CharacterOwner){
  const auto& H=CurrentFloor.HitResult;const bool Ramp=H.GetActor()&&H.GetActor()->ActorHasTag(TEXT("RideRamp"))&&!(H.GetComponent()&&H.GetComponent()->ComponentHasTag(TEXT("RideGrass")));
  const float Rise=Ramp&&H.ImpactNormal.Z>.25f?-FVector::DotProduct(CharacterOwner->GetActorForwardVector(),H.ImpactNormal)/H.ImpactNormal.Z:0;
  if(Ramp&&Rise>.1f&&Speed>=500){RampLaunchSpeed=FMath::Clamp(Speed*Rise,0.f,750.f);RampLaunchGrace=.35f;}
  else if(!Ramp||Rise<-.1f){RampLaunchSpeed=RampLaunchGrace=0;}
  else if(Ramp&&Rise<=.05f&&RampLaunchGrace>0&&RampLaunchSpeed>510){
   const auto* Bike=Cast<ABattleBike>(CharacterOwner);const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
   if(Bike&&Mode&&!Mode->bRunEnded&&Mode->StartCountdown<=0&&!UGameplayStatics::IsGamePaused(this)&&!Bike->bParked&&Bike->RiderHealth>0&&Bike->StunRemaining<=0&&Recovery<=0)SetMovementMode(MOVE_Falling);
  }
 }
 BoostRemaining=FMath::Max(0.f,BoostRemaining-Dt);
 SmoothedSteer=SteeringResponse(SmoothedSteer,Recovery>0?0.f:Steer,Dt);
 ContactCooldown=FMath::Max(0.f,ContactCooldown-Dt);BounceRemaining=FMath::Max(0.f,BounceRemaining-Dt);SlideRemaining=FMath::Max(0.f,SlideRemaining-Dt);
 if(auto* Bike=Cast<ABattleBike>(CharacterOwner);Bike&&Bike->bCrashActive){Recovery=2;Speed=0;return;}
 if(Recovery>0){
  Recovery=FMath::Max(0.f,Recovery-Dt);Speed=0;
  if(Recovery==0){
   Speed=!bPendingLakeEntry&&Pedal>0?240:0;
  }
 }
 if(CharacterOwner){
  const AActor* Floor=CurrentFloor.HitResult.GetActor();bGrass=Floor&&(Floor->ActorHasTag(TEXT("RideGrass"))||(CurrentFloor.HitResult.GetComponent()&&CurrentFloor.HitResult.GetComponent()->ComponentHasTag(TEXT("RideGrass"))));
  if(Recovery<=0){
   FRotator Heading=CharacterOwner->GetActorRotation();Heading.Pitch=Heading.Roll=0;
   const float SignedSpeed=Speed-ReverseSpeed;
   float DesiredTurnRate=SmoothedSteer*FMath::Lerp(105.f,55.f,FMath::Clamp(FMath::Abs(SignedSpeed)/1600.f,0.f,1.f))*FMath::Clamp(SignedSpeed/250.f,-1.f,1.f);
   if(bRealHandling){
    // Bicycle model: yaw = speed / wheelbase * tan(front-wheel angle).
    const float Requested=SignedSpeed/110.f*FMath::Tan(FMath::DegreesToRadians(SmoothedSteer*32.f));
    const float Grip=(bGrass?350.f:680.f)*(Brake>.5f?.65f:1.f);
    const float Limit=Grip/FMath::Max(FMath::Abs(SignedSpeed),50.f);
    DesiredTurnRate=IsMovingOnGround()?FMath::RadiansToDegrees(FMath::Clamp(Requested,-Limit,Limit)):0.f;
   }
   TurnRateDegrees=(FMath::Abs(SignedSpeed)<=1.f||(bRealHandling&&!IsMovingOnGround()))?0.f:FMath::Lerp(TurnRateDegrees,DesiredTurnRate,1.f-FMath::Exp(-6.f*Dt));
   Heading.Yaw+=TurnRateDegrees*Dt;CharacterOwner->SetActorRotation(Heading);
   const bool BrakeTurn=Brake>.5f&&PreviousBrake<=.5f&&FMath::Abs(Steer)>.3f&&Speed>800;
   // Drifting requires a deliberate brake input, never ordinary steering.
   if(!bRealHandling&&BrakeTurn)SlideRemaining=.65f;
   if(Floor&&(Floor->ActorHasTag(TEXT("RidePath"))||Floor->ActorHasTag(TEXT("RideDirt"))))LastSafeLocation=CharacterOwner->GetActorLocation();
  }
 }
 PreviousBrake=Brake;PreviousSteer=Steer;Super::TickComponent(Dt,Type,Function);
 // Resolve water after movement refreshes CurrentFloor. A bridge return must not
 // be judged using the underwater floor cached before teleporting to safety.
 if(CharacterOwner&&Recovery<=0&&IsMovingOnGround()){
  // Keep enough dry bank for the capsule to settle after remounting on a slope.
  bool Wet=false;const FVector Here=CharacterOwner->GetActorLocation();
  for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It&&!Wet;++It){
   FVector Probe=Here;Probe.Z=It->GetActorLocation().Z+1;
   Wet=It->ContainsBike(Probe);
   for(int I=0;I<8&&!Wet;I++){const float A=I*PI/4;Wet=It->ContainsBike(Probe+FVector(FMath::Cos(A)*120,FMath::Sin(A)*120,0));}
  }
  if(!Wet){bPendingLakeEntry=false;LastDryLocation=CharacterOwner->GetActorLocation();bHasDryLocation=true;}
 }
 if(CharacterOwner&&Recovery<=0){const AActor* Floor=CurrentFloor.HitResult.GetActor();if(!Floor||!Floor->ActorHasTag(TEXT("RideBridge")))for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(CharacterOwner->GetActorLocation())){Wipeout(TEXT("Splash"),true);break;}}
}
void UBattleBikeMovement::CalcVelocity(float Dt,float Friction,bool Fluid,float Braking){
 if(!CharacterOwner)return;
 if(Recovery>0){Velocity.X=Velocity.Y=0;return;}
 if(BounceRemaining>0){Velocity.X=BounceDirection.X*140;Velocity.Y=BounceDirection.Y*140;return;}
 // S/Down brakes to a stop, then walks the bike backward. Space never reverses.
 const bool Reverse=bReverseRequested&&Pedal<=0&&Speed<20&&IsMovingOnGround();
 ReverseSpeed=FMath::FInterpConstantTo(ReverseSpeed,Reverse?180.f:0.f,Dt,Reverse?400.f:650.f);
 if(ReverseSpeed>.1f){
  Speed=0;BoostRemaining=0;
  const FVector Backward=-CharacterOwner->GetActorForwardVector()*ReverseSpeed;
  Velocity.X=Backward.X;Velocity.Y=Backward.Y;return;
 }
 static const float Caps[]={420,700,1000,1300,1600};static const float Accel[]={640,500,420,360,320};
 static const float ArcadeCaps[]={650,950,1300,1650,1950};
 const float Cap=(BoostRemaining>0?(bRealHandling?1800.f:2200.f):(bRealHandling?Caps[Gear-1]:ArcadeCaps[Gear-1]))*(bGrass?.85f:1.f);
 const float Drag=Speed>0?22.f+Speed*.012f:0;
 if(BoostRemaining>0&&Brake<=0&&(!bRealHandling||IsMovingOnGround()))Speed=FMath::FInterpConstantTo(Speed,Cap,Dt,1500.f);
 if(bRealHandling){
  if(IsMovingOnGround()){
   const FVector Normal=CurrentFloor.HitResult.ImpactNormal.GetSafeNormal();
   const FVector Tangent=FVector::VectorPlaneProject(CharacterOwner->GetActorForwardVector(),Normal).GetSafeNormal();
   const float GradeForce=-980.f*Tangent.Z;
   const float Resistance=Speed>0?(bGrass?65.f:12.f)+.000025f*Speed*Speed:0.f;
   // An e-bike brake cuts motor assistance even if the pedal key is held.
   const float Motor=Brake>0?0.f:Pedal*Accel[Gear-1]*FMath::Clamp((Cap-Speed)/150.f,0.f,1.f);
   // Preserve downhill coasting, but let the brake stop and hold on every
   // rideable incline. Otherwise steep grass accelerates through full braking
   // and S can never reach the low-speed threshold needed to back out.
   const float BrakeForce=Brake*((bGrass?450.f:850.f)+FMath::Max(0.f,GradeForce));
   Speed=FMath::Clamp(Speed+(Motor+GradeForce-Resistance-BrakeForce)*Dt,0.f,2200.f);
  }
 }else {
  // Arcade corner assist eases off the motor and sheds speed progressively at
  // large steering angles, giving tight bridge turns a smaller turning circle.
  const FVector Normal=CurrentFloor.HitResult.ImpactNormal.GetSafeNormal();
  const FVector Tangent=IsMovingOnGround()?FVector::VectorPlaneProject(CharacterOwner->GetActorForwardVector(),Normal).GetSafeNormal():FVector::ZeroVector;
  // Arcade mode keeps easy steering, but hills still obey gravity. A steep
  // Piedmont descent can carry the bike beyond its motor-limited gear speed.
  // The world is geographically compressed to one-third scale. Give Arcade
  // descents a little more pull so Piedmont's real grades read at game speed;
  // Real Bike Physics above continues to use normal 980 cm/s² gravity.
  const float GradeForce=IsMovingOnGround()?-1150.f*Tangent.Z:0.f;
  const float Corner=IsMovingOnGround()?FMath::SmoothStep(.45f,1.f,FMath::Abs(SmoothedSteer)):0.f;
  const float CornerCap=FMath::Lerp(Cap,FMath::Min(Cap,220.f+80.f*Gear),Corner);
  const float Motor=Speed<CornerCap?Pedal*Accel[Gear-1]:0.f;
  const float BrakeForce=Brake>0?(SlideRemaining>0?100.f:1100.f)+FMath::Max(0.f,GradeForce):0.f;
  Speed=FMath::Clamp(Speed+(Motor+GradeForce-Drag-BrakeForce)*Dt,0.f,2500.f);
  // Preserve earned downhill and boost momentum on a straight line. Corner
  // assist sheds only the speed needed for the requested turn.
  if(Speed>CornerCap&&Corner>.01f)Speed=FMath::FInterpConstantTo(Speed,CornerCap,Dt,550.f*Corner);
 }
 const FVector Desired=CharacterOwner->GetActorForwardVector()*Speed;
 const FVector Horizontal=FMath::Lerp(FVector(Velocity.X,Velocity.Y,0),Desired,1.f-FMath::Exp(-(SlideRemaining>0?2.3f:18.f)*Dt));
 Velocity.X=Horizontal.X;Velocity.Y=Horizontal.Y;
 if(Pedal>0&&IsMovingOnGround())Cadence+=Dt*FMath::Clamp(Speed/(Gear*100.f),.5f,2.f)*2*PI;
}
void UBattleBikeMovement::Wipeout(const FString& Reason,bool Water){
 if(Recovery>0)return;BoostRemaining=0;RecoveryReason=Reason;
 if(Water){
  if(!bPendingLakeEntry)Wipeouts++;bPendingLakeEntry=true;Speed=0;StopMovementImmediately();
  if(auto* Bike=Cast<ABattleBike>(CharacterOwner))if(Bike->EnterLake(CharacterOwner->GetActorLocation(),LastDryLocation)){bPendingLakeEntry=false;Recovery=0;return;}
  // Retry transient spawn/shore obstructions without transporting the rider to a path.
  Recovery=.25f;DisableMovement();return;
 }
 const FVector CrashVelocity=Velocity.IsNearlyZero()?CharacterOwner->GetActorForwardVector()*Speed:Velocity;
 bPendingLakeEntry=false;Recovery=2;Wipeouts++;
 if(auto* Bike=Cast<ABattleBike>(CharacterOwner)){Bike->RideImpact(1.f,false);Bike->StartPhysicalCrash(CrashVelocity);}
 Speed=0;StopMovementImmediately();
}

void UBattleBikeMovement::HandleImpact(const FHitResult& Hit,float TimeSlice,const FVector& MoveDelta){
 Super::HandleImpact(Hit,TimeSlice,MoveDelta);
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("BattlePhoneRideAudit"))&&CharacterOwner&&CharacterOwner->GetActorLocation().Y>100500&&CharacterOwner->GetActorLocation().Y<101400&&Hit.ImpactNormal.Z<.6f){
  UE_LOG(LogTemp,Display,TEXT("PhoneActualImpact: actor=%s component=%s position=%s normal=%s delta=%s speed=%.2f penetrating=%d"),*GetNameSafe(Hit.GetActor()),*GetNameSafe(Hit.GetComponent()),*CharacterOwner->GetActorLocation().ToString(),*Hit.ImpactNormal.ToString(),*MoveDelta.ToString(),Speed,Hit.bStartPenetrating);
 }
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleTutorialAudit"))&&CharacterOwner&&Hit.ImpactNormal.Z<.6f){
  UE_LOG(LogTemp,Display,TEXT("TutorialImpact: actor=%s component=%s point=%s normal=%s bike=%s speed=%.1f"),*GetNameSafe(Hit.GetActor()),*GetNameSafe(Hit.GetComponent()),*Hit.ImpactPoint.ToString(),*Hit.ImpactNormal.ToString(),*CharacterOwner->GetActorLocation().ToString(),Speed);
 }
#endif

 if(!CharacterOwner||Recovery>0||ContactCooldown>0||Hit.ImpactNormal.Z>.45f)return;
 auto CrashImpact=[&](const TCHAR* Reason,float ClosingSpeed){
  if(auto* Bike=Cast<ABattleBike>(CharacterOwner)){
   Bike->ApplyRiderDamage(FMath::Clamp(ClosingSpeed/30.f,8.f,30.f));
   if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(Bike)))Mode->AdjustRunTime(-10,TEXT("CRASH"));
  }
  Wipeout(Reason);
 };
 const bool Traffic=Hit.GetActor()&&(Hit.GetActor()->ActorHasTag(TEXT("PiedmontTraffic"))||Hit.GetActor()->ActorHasTag(TEXT("PiedmontHostile"))||Hit.GetActor()->ActorHasTag(TEXT("RideVehicle")));
 if(Traffic){
  const float Directness=-FVector::DotProduct(CharacterOwner->GetActorForwardVector(),Hit.ImpactNormal.GetSafeNormal2D());
  const bool Direct=Speed>200&&Directness>.35f&&Speed*Directness>150;
  if(auto* Person=Cast<APiedmontPedestrian>(Hit.GetActor()))Person->BikeImpact(Direct?Speed:Speed*.2f,Direct?CharacterOwner->GetActorForwardVector():-Hit.ImpactNormal.GetSafeNormal2D());
  if(Direct&&Hit.GetActor()&&Hit.GetActor()->ActorHasTag(TEXT("PiedmontTraffic")))if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(CharacterOwner)))Mode->RecordAssault(Hit.GetActor());
  ContactCooldown=.35f;if(Direct&&(bRealHandling||Hit.GetActor()->ActorHasTag(TEXT("PiedmontHostile"))))CrashImpact(TEXT("Traffic impact"),Speed*Directness);else {Speed*=.8f;if(auto* Bike=Cast<ABattleBike>(CharacterOwner))Bike->RideImpact(.2f);}return;
 }
 if(Hit.GetActor()&&Hit.GetActor()->ActorHasTag(TEXT("RideTree")))++TreeContacts;
 // Let a glancing rail/wall contact slide instead of repeatedly killing propulsion.
 // A direct hit still stops the bike and gives the existing small separation nudge.
 const FVector Normal=Hit.ImpactNormal.GetSafeNormal2D();
 const FVector Travel=CharacterOwner->GetActorForwardVector()*(ReverseSpeed>Speed?-1.f:1.f);
 const float Into=FMath::Clamp(-FVector::DotProduct(Travel,Normal),0.f,1.f);
 if(bRealHandling&&Speed>200&&Into>.35f&&Speed*Into>150){ContactCooldown=.35f;CrashImpact(TEXT("Obstacle impact"),Speed*Into);return;}
 if(Into<.65f){
  const float Retained=FMath::Sqrt(FMath::Max(0.f,1.f-Into*Into));
  Speed*=Retained;ReverseSpeed*=Retained;ContactCooldown=.2f;
  if(auto* Bike=Cast<ABattleBike>(CharacterOwner))if(Speed>300)Bike->RideImpact(.12f);
  return;
 }
 // Walking-speed contact keeps the small separation nudge for maneuvering.
 if(auto* Bike=Cast<ABattleBike>(CharacterOwner))if(Speed>100)Bike->RideImpact(.45f);
 BounceDirection=Hit.ImpactNormal.GetSafeNormal2D();BounceRemaining=.16f;ContactCooldown=.25f;Speed=ReverseSpeed=0;
}

bool UBattleBikeMovement::Hop(){
 auto* Bike=Cast<ABattleBike>(CharacterOwner);auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(!Bike||!Mode||Mode->bRunEnded||Mode->StartCountdown>0||UGameplayStatics::IsGamePaused(this)||Bike->bParked||Bike->RiderHealth<=0||Bike->StunRemaining>0||Recovery>0||(!IsMovingOnGround()&&(bRealHandling||!IsFalling()||JumpGraceRemaining<=0))||Speed<150)return false;
 JumpGraceRemaining=0;Velocity.Z=FMath::Max(0.f,Velocity.Z)+(bRealHandling?650.f:750.f);SetMovementMode(MOVE_Falling);return true;
}
void UBattleBikeMovement::OnMovementModeChanged(EMovementMode Previous,uint8 Custom){
 Super::OnMovementModeChanged(Previous,Custom);
 auto* Bike=Cast<ABattleBike>(CharacterOwner);if(!Bike)return;
 if(IsFalling()&&Previous==MOVE_Walking){AirOrigin=Bike->GetActorLocation();AirSeconds=AirPeak=0;if(RampLaunchGrace>0&&Recovery<=0&&Bike->StunRemaining<=0&&Bike->RiderHealth>0&&!Bike->bParked)Velocity.Z=FMath::Max(Velocity.Z,RampLaunchSpeed);RampLaunchSpeed=RampLaunchGrace=0;bRewardableAir=Speed>=150&&!Bike->bParked&&Recovery<=0&&Bike->StunRemaining<=0&&Bike->RiderHealth>0;}
 else if(Previous==MOVE_Falling){
  bool Water=false;for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Bike->GetActorLocation())){Water=true;break;}
  const bool Earned=!Water&&bRewardableAir&&IsMovingOnGround()&&AirSeconds>=.25f&&AirPeak>=65&&Recovery<=0&&!Bike->bParked&&Bike->StunRemaining<=0&&Bike->RiderHealth>0;
  bRewardableAir=false;
  if(Earned)if(auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this)))if(Mode->AdjustRunTime(10,TEXT("AIRTIME")))AirRewards++;
 }
}
