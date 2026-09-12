#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "PiedmontBike.h"
#include "BattleBike.h"
#include "PiedmontBlood.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

APiedmontPedestrian::APiedmontPedestrian(){
 AIControllerClass=AAIController::StaticClass();AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;
 bUseControllerRotationYaw=false;Tags.Add(TEXT("PiedmontTraffic"));
 auto* Move=GetCharacterMovement();Move->bOrientRotationToMovement=true;Move->RotationRate=FRotator(0,360,0);
 Move->bUseRVOAvoidance=true;Move->AvoidanceConsiderationRadius=350;Move->AvoidanceWeight=.5f;
}
void APiedmontPedestrian::BeginPlay(){
 if(bAmbientSleeper){CityAppearanceVariant=0;bReturnToSleepAfterChase=true;}
 InitializeCityAppearance();Super::BeginPlay();Configure(Kind);ThinkRemaining=FMath::FRandRange(.1f,.7f);
 if(bAmbientSleeper)BeginSleeping();
}
void APiedmontPedestrian::Configure(EPiedmontPedestrianKind NewKind){
 Kind=NewKind;GetCharacterMovement()->MaxWalkSpeed=Kind==EPiedmontPedestrianKind::Jogger?310:135;
}
bool APiedmontPedestrian::MoveTo(FVector Goal,float AcceptanceRadius){
 if(bHasDestination&&FVector::Dist2D(Destination,Goal)<60)return true;
 auto* AI=Cast<AAIController>(GetController());if(!AI)return false;
 FAIMoveRequest Request;Request.SetGoalLocation(Goal);Request.SetAcceptanceRadius(AcceptanceRadius);Request.SetUsePathfinding(true);Request.SetAllowPartialPath(false);Request.SetProjectGoalLocation(true);
 const auto Result=AI->MoveTo(Request);bHasDestination=Result.Code!=EPathFollowingRequestResult::Failed;
 if(bHasDestination)Destination=Goal;return bHasDestination;
}
void APiedmontPedestrian::ChooseDestination(){
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());if(!Nav)return;
 FNavLocation Goal;
 if(IsValid(GroupLeader)&&!GroupLeader->bDead){
  if(GroupLeader->PauseRemaining>0&&FVector::Dist2D(GetActorLocation(),GroupLeader->GetActorLocation())<350){PauseRemaining=GroupLeader->PauseRemaining;return;}
  const FVector Side=GroupLeader->GetActorRightVector()*GroupSide*95;
  if(GroupLeader->bHasDestination&&Nav->ProjectPointToNavigation(GroupLeader->Destination+Side,Goal,FVector(150,150,180)))MoveTo(Goal.Location);
  return;
 }
 GroupLeader=nullptr;
 for(int32 Try=0;Try<5;Try++)if(Nav->GetRandomReachablePointInRadius(GetActorLocation(),3000,Goal)&&FVector::Dist2D(GetActorLocation(),Goal.Location)>600){if(MoveTo(Goal.Location))return;}
}
void APiedmontPedestrian::YieldTo(APawn* Source,bool Horn){
 if(!Source||bDead||Kind==EPiedmontPedestrianKind::Jogger||YieldCooldown>0||StumbleRemaining>0)return;
 if(FVector::Dist2D(Source->GetActorLocation(),GetActorLocation())>(Horn?1400:600))return;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(ParkVisitorWarning),false,this);Q.AddIgnoredActor(Source);FHitResult Hit;
 if(GetWorld()->LineTraceSingleByChannel(Hit,Source->GetActorLocation()+FVector(0,0,35),GetActorLocation()+FVector(0,0,30),ECC_Visibility,Q))return;
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());if(!Nav)return;
 FVector Away=(GetActorLocation()-Source->GetActorLocation()).GetSafeNormal2D();
 FVector Side=Source->GetActorRightVector();if(FVector::DotProduct(Away,Side)<0)Side=-Side;
 FNavLocation Goal;
 if(Nav->ProjectPointToNavigation(GetActorLocation()+Side*180+Away*80,Goal,FVector(100,100,180))&&MoveTo(Goal.Location)){
  PauseRemaining=0;YieldRemaining=2;YieldCooldown=5;if(Horn)HornReactions++;
 }
}
bool APiedmontPedestrian::BeginBenchReach(){
 if(bDead||bSwimming||SleepPhase||KnockdownPhase||StumbleRemaining>0||bBenchReaching||CityAppearanceVariant!=0||!bNativeCrowdRig)return false;
 auto* Clip=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/Mixamo/LowReachCandidate/MixamoLowReachReference_Anim.MixamoLowReachReference_Anim"));if(!Clip)return false;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();bHasDestination=false;SetBodySequence(Clip,false);bBenchReaching=true;return true;
}
void APiedmontPedestrian::CancelBenchReach(){if(bBenchReaching){bBenchReaching=false;StopBodySequence();}}
void APiedmontPedestrian::HearHorn(APawn* Source){CancelBenchReach();YieldTo(Source,true);}
void APiedmontPedestrian::BikeImpact(float Speed,FVector Direction){
 if(bDead||StumbleRemaining>0)return;CancelBenchReach();CancelSleepBehavior();BikeContacts++;StumbleRemaining=Speed>330?2.3f:1.2f;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();bHasDestination=false;
 if(Speed>330&&BeginKnockdown(Speed,Direction))return;
 // Light contact plays an authored surprise; unavailable rigs retain the fallback.
 bPlayingBumpReaction=bNativeCrowdRig&&BumpReaction&&Speed<=330;
 if(bPlayingBumpReaction){
  GetCharacterMovement()->StopMovementImmediately();
  PlayBodyAction(BumpReaction);StumbleRemaining=BumpReaction->GetPlayLength();
  UE_LOG(LogTemp,Display,TEXT("CityBump: actor=%s clip=%s duration=%.3f"),*GetName(),*BumpReaction->GetName(),StumbleRemaining);
 }else LaunchCharacter(Direction.GetSafeNormal2D()*FMath::Clamp(Speed*.25f,60.f,240.f)+FVector(0,0,60),true,true);
}
void APiedmontPedestrian::Tick(float Dt){
 Super::Tick(Dt);
 if(KnockdownPhase){TickKnockdown(Dt);return;}
 if(bBenchReaching){
  auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));
  if(bDead||bSwimming||!Mode||Mode->bRunEnded||!IsBodySequencePlaying())CancelBenchReach();
  else return;
 }
 if(bAmbientSleeper)TickSleeperTrigger(Dt);
 if(TickSleepBehavior(Dt))return;
 if(bDead){Body->SetRelativeRotation(FRotator(0,-90,85));return;}
 if(StumbleRemaining>0){StumbleRemaining=FMath::Max(0.f,StumbleRemaining-Dt);if(!bPlayingBumpReaction)Body->SetRelativeRotation(FRotator(0,-90,FMath::Sin(StumbleRemaining*5)*22));
  if(StumbleRemaining<=0)bPlayingBumpReaction=false;return;}
 auto* AI=Cast<AAIController>(GetController());if(!AI)return;
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));
 if(!Mode||Mode->bRunEnded||bSwimming){AI->StopMovement();GetCharacterMovement()->StopMovementImmediately();return;}
 YieldCooldown=FMath::Max(0.f,YieldCooldown-Dt);ThinkRemaining-=Dt;

 if(PauseRemaining>0){PauseRemaining=FMath::Max(0.f,PauseRemaining-Dt);AI->StopMovement();bHasDestination=false;return;}
 if(YieldRemaining>0){YieldRemaining-=Dt;return;}
 if(ThinkRemaining>0)return;ThinkRemaining=.65f;
 if(auto* BikePawn=UGameplayStatics::GetPlayerPawn(this,0))if((BikePawn->ActorHasTag(TEXT("RideBike"))||Cast<APiedmontBike>(BikePawn))&&BikePawn->GetVelocity().Size2D()>150&&FVector::DotProduct((GetActorLocation()-BikePawn->GetActorLocation()).GetSafeNormal2D(),BikePawn->GetActorForwardVector())>.5f){YieldTo(BikePawn,false);if(YieldRemaining>0)return;}
 if(bHasDestination&&AI->GetMoveStatus()==EPathFollowingStatus::Idle){
  const bool Arrived=FVector::Dist2D(GetActorLocation(),Destination)<150;bHasDestination=false;
  if(Arrived){CompletedWalks++;if(!GroupLeader&&Kind==EPiedmontPedestrianKind::Walker&&FMath::FRand()<.25f){PauseRemaining=FMath::FRandRange(2.f,5.f);return;}}
 }
 if(!bHasDestination||IsValid(GroupLeader))ChooseDestination();
}
float APiedmontPedestrian::TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer){
 if(bDead||Amount<=0)return 0;
 CancelBenchReach();CancelSleepBehavior();
 APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,30),FVector::UpVector);bDead=true;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 if(KnockdownPhase==2)KnockdownPhase=0;
 if(KnockdownPhase==0)BeginKnockdown(100,Causer?(GetActorLocation()-Causer->GetActorLocation()).GetSafeNormal2D():GetActorForwardVector());
 GetCharacterMovement()->DisableMovement();SetLifeSpan(15);return Amount;
}
bool APiedmontPedestrian::SetDestinationForValidation(FVector Goal){
#if WITH_EDITOR
 if(GetWorld()->WorldType==EWorldType::PIE){GroupLeader=nullptr;PauseRemaining=0;return MoveTo(Goal);}
#endif
 return false;
}

bool APiedmontPedestrian::BeginSleeping(){
 if(!bNativeCrowdRig||bDead||bSwimming||KnockdownPhase||StumbleRemaining>0||SleepPhase)return false;
 // The initial candidate was authored for the male City skeleton only.
 if(!Body->GetSkinnedAsset()||!Body->GetSkinnedAsset()->GetName().StartsWith(TEXT("m_tal_nrw")))return false;
 auto* Animation=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/Mixamo/SleepCandidate/SleepingBaked"));
 if(!Animation)return false;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();bHasDestination=false;
 SleepOrigin=GetActorLocation();SleepOriginRotation=GetActorRotation();SleepTarget.Reset();
 SetBodySequence(Animation,true);SleepPhase=1;return true;
}
bool APiedmontPedestrian::WakeFromSleep(){
 if(SleepPhase!=1||bDead||bSwimming)return false;
 // Keep the sleeper down if there is no room for the standing capsule.
 FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperStandingSpace),false,this);
 if(GetWorld()->OverlapBlockingTestByChannel(GetActorLocation(),FQuat::Identity,ECC_Pawn,
  FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(),GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-2.f),Q))return false;
 auto* Animation=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/Mixamo/SleepCandidate/SleepToStand_R"));
 if(!Animation)return false;
 SetBodySequence(Animation,false);SleepPhase=2;return true;
}

bool APiedmontPedestrian::WakeAndChase(APawn* Target){
 if(!IsValid(Target)||Target==this||Target->GetWorld()!=GetWorld()||FVector::Dist2D(Target->GetActorLocation(),GetActorLocation())>600)return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperNotice),false,this);Q.AddIgnoredActor(Target);FHitResult Hit;
 if(GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation(),Target->GetActorLocation(),ECC_Visibility,Q))return false;
 if(!WakeFromSleep())return false;
 SleepTarget=Target;return true;
}
void APiedmontPedestrian::CancelSleepBehavior(){
 if(!SleepPhase)return;
 SleepPhase=0;SleepTarget.Reset();StopBodySequence();ChaseClock=0;ChaseRepath=0;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();Configure(Kind);bHasDestination=false;
}
bool APiedmontPedestrian::TickSleepBehavior(float Dt){
 if(!SleepPhase)return false;
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));
 if(bDead||bSwimming||!Mode||Mode->bRunEnded){CancelSleepBehavior();return false;}
 auto* AI=Cast<AAIController>(GetController());
 if(SleepPhase==2&&!IsBodySequencePlaying()){
  StopBodySequence();
  if(SleepTarget.IsValid()){SleepPhase=3;ChaseClock=0;ChaseRepath=0;GetCharacterMovement()->MaxWalkSpeed=260;}
  else{CancelSleepBehavior();return false;}
 }
 if(SleepPhase==4&&!IsBodySequencePlaying()){
  // Keep the authored landed pose if a new obstacle occupies the landing area.
  // Retry the handoff while the character remains down instead of teleporting through it.
  if(!IsSettlePathClear(SleepLanding))return true;
  auto* Sleep=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/Mixamo/SleepCandidate/SleepingBaked"));
  if(!Sleep)return true;
  SetActorLocationAndRotation(SleepLanding,SleepLandingRotation,false,nullptr,ETeleportType::TeleportPhysics);
  GetCharacterMovement()->bForceNextFloorCheck=true;
  SetBodySequence(Sleep,true);SleepOrigin=SleepLanding;SleepOriginRotation=SleepLandingRotation;SleepPhase=1;AnimateBody(0);return true;
 }
 if(SleepPhase==3){
  ChaseClock+=Dt;ChaseRepath-=Dt;
  auto* Target=SleepTarget.Get();
  if(!AI||!IsValid(Target)||Target->IsActorBeingDestroyed()||ChaseClock>=6.f||
   FVector::Dist2D(GetActorLocation(),SleepOrigin)>900||FVector::Dist2D(Target->GetActorLocation(),SleepOrigin)>1200){
   FinishSleeperChase();return true;
  }
  if(ChaseRepath<=0){ChaseRepath=.35f;if(!MoveTo(Target->GetActorLocation()))FinishSleeperChase();}
  return true;
 }
 if(SleepPhase==5){
  ReturnClock+=Dt;ChaseRepath-=Dt;
  if(!AI||ReturnClock>10.f){CancelSleepBehavior();PauseRemaining=5;return true;}
  const float HomeDistance=FVector::Dist2D(GetActorLocation(),SleepOrigin);
  if(HomeDistance<12){
   AI->StopMovement();GetCharacterMovement()->StopMovementImmediately();
   // Returning navigation changes facing. Restore the checked resting direction
   // gradually so the authored lateral fall uses its original clear corridor.
   SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(),SleepOriginRotation,Dt,180.f));
   if(FMath::Abs(FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw,SleepOriginRotation.Yaw))<1.f&&ChaseRepath<=0){ChaseRepath=.5f;BeginSettling();}
  }else if(HomeDistance<200){
   // Grass rest points can lie just beyond the projected navigation goal.
   // Walk the final short segment through CharacterMovement collision/floor checks.
   AI->StopMovement();bHasDestination=false;
   AddMovementInput((SleepOrigin-GetActorLocation()).GetSafeNormal2D(),1.f,true);
  }else if(ChaseRepath<=0){
   ChaseRepath=.5f;
   if(!MoveTo(SleepOrigin,10.f)){CancelSleepBehavior();PauseRemaining=5;}
  }
  return true;
 }
 if(AI)AI->StopMovement();GetCharacterMovement()->StopMovementImmediately();return true;
}

bool APiedmontPedestrian::IsSettlePathClear(const FVector& Landing) const{
 FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperFallSpace),false,this);FHitResult Hit;
 const FVector Start=GetActorLocation()+FVector(0,0,15),End=Landing+FVector(0,0,15);
 // Check a wider corridor for the spread arms and torso, not just the standing capsule.
 if(GetWorld()->OverlapBlockingTestByChannel(Start,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(100),Q)||
  GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(100),Q))return false;
 const float Floor=GetActorLocation().Z-GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-GetCharacterMovement()->CurrentFloor.GetDistanceToFloor();
 for(int32 I=0;I<=6;++I){
  const FVector P=FMath::Lerp(GetActorLocation(),Landing,float(I)/6.f);
  if(!GetWorld()->LineTraceSingleByChannel(Hit,FVector(P.X,P.Y,Floor+50),FVector(P.X,P.Y,Floor-50),ECC_Visibility,Q)||
   Hit.ImpactNormal.Z<.98f||FMath::Abs(Hit.ImpactPoint.Z-Floor)>5.f)return false;
 }
 return true;
}
bool APiedmontPedestrian::BeginSettling(){
 if(!bNativeCrowdRig||bDead||bSwimming||KnockdownPhase||StumbleRemaining>0||(SleepPhase!=0&&SleepPhase!=3&&SleepPhase!=5))return false;
 if(!Body->GetSkinnedAsset()||!Body->GetSkinnedAsset()->GetName().StartsWith(TEXT("m_tal_nrw")))return false;
 // From the StumbleToSleep bake: mesh-local anchor of the aligned sleeping loop.
 const FVector Offset=Body->GetComponentQuat().RotateVector(FVector(-261.387456,39.753575,0));
 const FVector Landing=GetActorLocation()+Offset;
 if(!IsSettlePathClear(Landing))return false;
 auto* Fall=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/Mixamo/StumbleCandidate/StumbleToSleep"));
 if(!Fall)return false;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();Configure(Kind);bHasDestination=false;SleepTarget.Reset();
 SleepLanding=Landing;SleepLandingRotation=GetActorRotation()+FRotator(0,36.510763,0);
 SetBodySequence(Fall,false);SleepPhase=4;return true;
}

void APiedmontPedestrian::FinishSleeperChase(){
 if(!bReturnToSleepAfterChase){CancelSleepBehavior();PauseRemaining=2;return;}
 SleepTarget.Reset();
 if(BeginSettling())return;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
 GetCharacterMovement()->StopMovementImmediately();Configure(Kind);bHasDestination=false;
 SleepPhase=5;ReturnClock=0;ChaseRepath=0;
}

void APiedmontPedestrian::TickSleeperTrigger(float Dt){
 auto* Target=UGameplayStatics::GetPlayerPawn(this,0);
 auto* Mode=Cast<APiedmontRideMode>(UGameplayStatics::GetGameMode(this));
 const auto* Lab=Cast<ABattleLabMode>(Mode);
 bool Eligible=SleepPhase==1&&!bDead&&!bSwimming&&Mode&&!Mode->bRunEnded&&Mode->StartCountdown<=0&&(!Lab||!Lab->bTutorialActive)&&IsValid(Target);
 if(const auto* Person=Cast<APiedmontExplorer>(Target))Eligible=Eligible&&!Person->bDead;
 if(const auto* BikePawn=Cast<ABattleBike>(Target))Eligible=Eligible&&BikePawn->RiderHealth>0;
 const float Distance=IsValid(Target)?FVector::Dist2D(GetActorLocation(),Target->GetActorLocation()):BIG_NUMBER;
 if(IsValid(Target)&&FMath::Abs(GetActorLocation().Z-Target->GetActorLocation().Z)>140)Eligible=false;
 if(SleeperTrigger.Observe(Distance,Dt,Eligible)){
  const bool Woke=FMath::FRand()<FMath::Clamp(AmbientWakeChance,0.f,1.f)&&WakeAndChase(Target);
  SleeperTrigger.Attempted(Woke);
 }
}
