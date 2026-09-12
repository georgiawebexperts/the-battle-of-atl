#include "PiedmontPedestrian.h"
#include "Animation/AnimSequence.h"
#include "PiedmontBike.h"
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
void APiedmontPedestrian::BeginPlay(){InitializeCityAppearance();Super::BeginPlay();Configure(Kind);ThinkRemaining=FMath::FRandRange(.1f,.7f);}
void APiedmontPedestrian::Configure(EPiedmontPedestrianKind NewKind){
 Kind=NewKind;GetCharacterMovement()->MaxWalkSpeed=Kind==EPiedmontPedestrianKind::Jogger?310:135;
}
bool APiedmontPedestrian::MoveTo(FVector Goal){
 if(bHasDestination&&FVector::Dist2D(Destination,Goal)<60)return true;
 auto* AI=Cast<AAIController>(GetController());if(!AI)return false;
 FAIMoveRequest Request;Request.SetGoalLocation(Goal);Request.SetAcceptanceRadius(45);Request.SetUsePathfinding(true);Request.SetAllowPartialPath(false);Request.SetProjectGoalLocation(true);
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
void APiedmontPedestrian::HearHorn(APawn* Source){YieldTo(Source,true);}
void APiedmontPedestrian::BikeImpact(float Speed,FVector Direction){
 if(bDead||StumbleRemaining>0)return;BikeContacts++;StumbleRemaining=Speed>330?2.3f:1.2f;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();bHasDestination=false;
 // A light contact makes visitors stop and raise their hands. Keep severe
 // impacts on the existing fall path until a true knockdown/recovery is integrated.
 bPlayingBumpReaction=bNativeCrowdRig&&BumpReaction&&Speed<=330;
 if(bPlayingBumpReaction){
  GetCharacterMovement()->StopMovementImmediately();
  PlayBodyAction(BumpReaction);StumbleRemaining=BumpReaction->GetPlayLength();
  UE_LOG(LogTemp,Display,TEXT("CityBump: actor=%s clip=%s duration=%.3f"),*GetName(),*BumpReaction->GetName(),StumbleRemaining);
 }else LaunchCharacter(Direction.GetSafeNormal2D()*FMath::Clamp(Speed*.25f,60.f,240.f)+FVector(0,0,60),true,true);
}
void APiedmontPedestrian::Tick(float Dt){
 Super::Tick(Dt);
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
 APiedmontBlood::Burst(GetWorld(),GetActorLocation()+FVector(0,0,30),FVector::UpVector);bDead=true;
 if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();GetCharacterMovement()->DisableMovement();SetLifeSpan(15);return Amount;
}
bool APiedmontPedestrian::SetDestinationForValidation(FVector Goal){
#if WITH_EDITOR
 if(GetWorld()->WorldType==EWorldType::PIE){GroupLeader=nullptr;PauseRemaining=0;return MoveTo(Goal);}
#endif
 return false;
}
