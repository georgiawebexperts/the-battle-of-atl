#include "BattleBike.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/World.h"
UBattleBikeMovement::UBattleBikeMovement(){
 MaxWalkSpeed=1800;MaxAcceleration=800;MaxStepHeight=60;SetWalkableFloorAngle(75);
 bOrientRotationToMovement=false;bUseControllerDesiredRotation=false;bMaintainHorizontalGroundVelocity=true;
 bEnablePhysicsInteraction=false;GravityScale=2;AirControl=1;BrakingDecelerationWalking=0;GroundFriction=0;
 MaxSimulationTimeStep=1.f/120;MaxSimulationIterations=16;
}
void UBattleBikeMovement::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function){
 BoostRemaining=FMath::Max(0.f,BoostRemaining-Dt);
 ContactCooldown=FMath::Max(0.f,ContactCooldown-Dt);BounceRemaining=FMath::Max(0.f,BounceRemaining-Dt);SlideRemaining=FMath::Max(0.f,SlideRemaining-Dt);
 if(Recovery>0){
  Recovery=FMath::Max(0.f,Recovery-Dt);Speed=0;
  if(Recovery==0){
   if(bWaterReturn&&CharacterOwner){CharacterOwner->SetActorLocation(ReturnLocation,false,nullptr,ETeleportType::TeleportPhysics);SetMovementMode(MOVE_Walking);bWaterReturn=false;}
   Speed=Pedal>0?240:0;
  }
 }
 if(CharacterOwner){
  const AActor* Floor=CurrentFloor.HitResult.GetActor();bGrass=Floor&&Floor->ActorHasTag(TEXT("RideGrass"));
  if(Recovery<=0){
   FRotator Heading=CharacterOwner->GetActorRotation();Heading.Pitch=Heading.Roll=0;
   Heading.Yaw+=Steer*FMath::Lerp(180.f,85.f,FMath::Clamp(Speed/1600.f,0.f,1.f))*Dt;CharacterOwner->SetActorRotation(Heading);
   if(Brake>.5f&&PreviousBrake<=.5f&&FMath::Abs(Steer)>.3f&&Speed>800)SlideRemaining=.65f;
   if(Floor&&(Floor->ActorHasTag(TEXT("RidePath"))||Floor->ActorHasTag(TEXT("RideDirt"))))LastSafeLocation=CharacterOwner->GetActorLocation();
   if(!Floor||!Floor->ActorHasTag(TEXT("RideBridge")))for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(CharacterOwner->GetActorLocation())){Wipeout(TEXT("Splash"),true);break;}
  }
 }
 PreviousBrake=Brake;Super::TickComponent(Dt,Type,Function);
}
void UBattleBikeMovement::CalcVelocity(float Dt,float Friction,bool Fluid,float Braking){
 if(!CharacterOwner)return;
 if(Recovery>0){Velocity.X=Velocity.Y=0;return;}
 if(BounceRemaining>0){Velocity.X=BounceDirection.X*140;Velocity.Y=BounceDirection.Y*140;return;}
 static const float Caps[]={420,700,1000,1300,1600};static const float Accel[]={640,500,420,360,320};
 const float Cap=(BoostRemaining>0?1800.f:Caps[Gear-1])*(bGrass?.75f:1.f);
 const float Drag=Speed>0?22.f+Speed*.012f:0;
 if(BoostRemaining>0&&Brake<=0)Speed=Cap;
 Speed=FMath::Clamp(Speed+(Pedal*Accel[Gear-1]-Drag-(Brake>0?(SlideRemaining>0?100.f:1100.f):0))*Dt,0.f,Cap);
 const FVector Desired=CharacterOwner->GetActorForwardVector()*Speed;
 const FVector Horizontal=FMath::VInterpTo(FVector(Velocity.X,Velocity.Y,0),Desired,Dt,SlideRemaining>0?2.3f:18.f);
 Velocity.X=Horizontal.X;Velocity.Y=Horizontal.Y;
 if(Pedal>0)Cadence+=Dt*FMath::Clamp(Speed/(Gear*100.f),.5f,2.f)*2*PI;
}
void UBattleBikeMovement::Wipeout(const FString& Reason,bool Water){
 if(Recovery>0)return;BoostRemaining=0;Recovery=2;Wipeouts++;RecoveryReason=Reason;bWaterReturn=Water;
 ReturnLocation=LastSafeLocation;if(Water)if(auto* Bike=Cast<ABattleBike>(CharacterOwner))ReturnLocation=Bike->FindPathReturn();
 Speed=0;StopMovementImmediately();
}
void UBattleBikeMovement::HandleImpact(const FHitResult& Hit,float TimeSlice,const FVector& MoveDelta){
 Super::HandleImpact(Hit,TimeSlice,MoveDelta);
 if(!CharacterOwner||Recovery>0||ContactCooldown>0||Hit.ImpactNormal.Z>.45f)return;
 const bool Traffic=Hit.GetActor()&&(Hit.GetActor()->ActorHasTag(TEXT("PiedmontTraffic"))||Hit.GetActor()->ActorHasTag(TEXT("PiedmontHostile"))||Hit.GetActor()->ActorHasTag(TEXT("RideVehicle")));
 if(Traffic){
  const float Directness=-FVector::DotProduct(CharacterOwner->GetActorForwardVector(),Hit.ImpactNormal.GetSafeNormal2D());
  const bool Direct=Speed>500&&Directness>.7f;
  if(auto* Person=Cast<APiedmontPedestrian>(Hit.GetActor()))Person->BikeImpact(Direct?Speed:Speed*.2f,Direct?CharacterOwner->GetActorForwardVector():-Hit.ImpactNormal.GetSafeNormal2D());
  ContactCooldown=.35f;if(Direct)Wipeout(TEXT("Traffic impact"));else Speed*=.8f;return;
 }
 // Terrain, walls and fences never enter the wipeout state.
 BounceDirection=Hit.ImpactNormal.GetSafeNormal2D();BounceRemaining=.16f;ContactCooldown=.25f;Speed=0;
}
