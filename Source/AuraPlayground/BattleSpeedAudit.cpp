#include "BattleBike.h"
#include "BattlePickup.h"
#include "BattleRider.h"
#include "PiedmontPedestrian.h"
#include "PiedmontPathSpline.h"
#include "PiedmontBike.h"
#include "Components/SplineComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
/**
 * Proves the findable throttle boost, which is what Elliott asked for: "a
 * throttle boost and maybe have like 3 and some that i can find".
 *
 * The shape of that promise, and therefore of this audit:
 *   - a speed crate banks one charge, it does not fire the boost itself
 *   - the bank holds three, so a fourth crate is wasted
 *   - SHIFT spends a charge and gives a short burst above gear speed
 *   - the burst ends on its own, and with an empty bank SHIFT does nothing
 *   - a rider on foot cannot bank a mounted-only boost
 *
 * This audit used to demand `BoostRemaining==5` the instant a crate was
 * touched. That was the old contract and has been wrong since the crates were
 * changed to bank charges (BattlePickup.cpp), so it failed for that reason and
 * not because the boost was broken.
 */
void TickBattleSpeedAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;int Stage=0;float Clock=0,Peak=0;bool Done=false;};static FState S;
 if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}if(S.Done||PC->GetWorld()->GetTimeSeconds()<5)return;
 ABattleBike* Base=Cast<ABattleBike>(PC->GetPawn());
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(PC));if(!Mode)return;
 auto Finish=[&](bool Pass,const TCHAR* Why){S.Done=true;UE_LOG(LogTemp,Display,TEXT("SpeedPickupAudit: {\"passed\":%s,\"reason\":\"%s\",\"peak_speed_cm_s\":%.2f,\"charges\":%d,\"boost_remaining\":%.2f}"),Pass?TEXT("true"):TEXT("false"),Why,S.Peak,Base?Base->BoostCharges:0,Base&&Base->Ride?Base->Ride->BoostRemaining:0.f);PC->ConsoleCommand(TEXT("quit"));};
 auto Key=[&](FKey K,bool Down){return PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 // A crate drops next to a pawn that is standing still, then is tried.
 auto Crate=[&](APawn* Who)->ABattleColaPickup*{
  const FTransform T(Who->GetActorLocation()+FVector(60,0,-20));
  auto* P=PC->GetWorld()->SpawnActorDeferred<ABattleColaPickup>(ABattleColaPickup::StaticClass(),T,PC,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if(!P)return nullptr;
  P->bSpeedBonus=true;P->FinishSpawning(T);P->SetActorTickEnabled(false);return P;
 };
 auto Shot=[&](const TCHAR* Name){FString Dir;if(FParse::Value(FCommandLine::Get(),TEXT("BattleSpeedReviewDir="),Dir))FScreenshotRequest::RequestScreenshot(Dir/Name,false,false);};
 if(S.Stage==0){
  if(!Base){Finish(false,TEXT("Missing bike"));return;}
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)It->Destroy();
  Mode->StartCountdown=0;Mode->bRunEnded=false;Base->RiderHealth=100;Base->DamageGrace=100;
  // Find a dry, straight, clear stretch to boost along. The old hard-coded spot
  // (-10000,4000) is now under the park lake, so the bike splashed the moment
  // it landed and the audit reported "Lost the bike" - which is why this audit
  // has been red since the lake landed, and not because the boost was broken.
  const float LaneCm=4000.f;
  FVector Spot;FVector Heading=FVector::ForwardVector;bool bFound=false;
  for(TActorIterator<APiedmontPathSpline> Path(PC->GetWorld());Path&&!bFound;++Path){
   auto* Route=Path->Centerline.Get();if(!Route)continue;
   const float Length=Route->GetSplineLength();
   for(float D=800.f;D+LaneCm<Length;D+=1200.f){
    const FVector P=Route->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);
    const FVector Far=Route->GetLocationAtDistanceAlongSpline(D+LaneCm,ESplineCoordinateSpace::World);
    const FVector Mid=Route->GetLocationAtDistanceAlongSpline(D+LaneCm*.5f,ESplineCoordinateSpace::World);
    if(FVector::Dist2D(P,Far)<LaneCm*.85f)continue;                       // long enough
    if(FVector::Dist2D(Mid,(P+Far)*.5f)>250.f)continue;                   // straight enough
    FHitResult G;FCollisionQueryParams Q;Q.bIgnoreTouches=true;
    if(!PC->GetWorld()->LineTraceSingleByChannel(G,P+FVector(0,0,2000),P-FVector(0,0,2500),ECC_Visibility,Q))continue;
    if(G.ImpactNormal.Z<.9f)continue;                                     // flat, not a bank
    const FVector At=G.ImpactPoint+FVector(0,0,98);
    bool bWet=false;for(TActorIterator<APiedmontWaterHazard> Water(PC->GetWorld());Water;++Water)if(Water->ContainsBike(At)){bWet=true;break;}
    if(bWet)continue;
    Spot=At;Heading=(Far-P).GetSafeNormal2D();bFound=true;break;
   }
  }
  if(!bFound){Finish(false,TEXT("No dry straight stretch to boost along"));return;}
  Base->SetActorLocationAndRotation(Spot,Heading.Rotation(),false,nullptr,ETeleportType::TeleportPhysics);
  Base->Ride->StopMovementImmediately();Base->Ride->SetMovementMode(MOVE_Walking);Base->Ride->Speed=0;Base->Ride->Gear=1;Base->Ride->bRealHandling=false;Base->Ride->bForceNextFloorCheck=true;
  Base->BoostCharges=0;Base->Nitro=0;
  // A dead rider banks nothing, and the crate stays in the world to prove it.
  auto* Dead=Crate(Base);if(!Dead){Finish(false,TEXT("Crate fixture failed"));return;}
  Base->RiderHealth=0;const bool DeadBlocked=!Dead->TryCollect(Base);Base->RiderHealth=100;
  if(!DeadBlocked||Dead->bConsumed){Finish(false,TEXT("Dead rider banked a boost"));return;}
  if(!Dead->TryCollect(Base)||Base->BoostCharges!=1){Finish(false,TEXT("Riding rider did not bank exactly one charge"));return;}
  if(Dead->TryCollect(Base)||Base->BoostCharges!=1){Finish(false,TEXT("Spent crate collected a second time"));return;}
  // The bank is three deep: the fourth crate in the same spot is a no-op.
  for(int32 I=0;I<4;I++){auto* More=Crate(Base);if(!More){Finish(false,TEXT("Extra crate fixture failed"));return;}More->TryCollect(Base);}
  if(Base->BoostCharges!=3){Finish(false,TEXT("Boost bank did not cap at three"));return;}
  S.Stage=1;S.Clock=0;return;
 }
 if(!Base){Finish(false,TEXT("Lost the bike"));return;}
 S.Clock+=Dt;S.Peak=FMath::Max(S.Peak,Base->Ride->Speed);
 if(Base->bCrashActive){Finish(false,TEXT("Unexpected knockdown in boost lane"));return;}
 if(S.Stage==1&&S.Clock>1.f){
  // SHIFT is the real binding, so press the real key rather than calling Boost().
  // The press is recorded now and dispatched on the next input tick, so the
  // charge is checked in the stage below rather than in this frame.
  Key(EKeys::LeftShift,true);
  S.Stage=2;
 }
 if(S.Stage==2&&S.Clock>1.35f){
  Key(EKeys::LeftShift,false);
  if(Base->BoostCharges!=2){
   // Separate "the key never reached the binding" from "Boost() refused".
   const bool Direct=Base->Boost();
   UE_LOG(LogTemp,Display,TEXT("SpeedAudit: shift_pre recovery=%.2f grounded=%d parked=%d ctrl=%s run_ended=%d countdown=%.2f direct_call=%d"),Base->Ride->Recovery,Base->Ride->IsMovingOnGround()?1:0,Base->bParked?1:0,*GetNameSafe(Base->GetController()),Mode->bRunEnded?1:0,Mode->StartCountdown,Direct?1:0);
   if(Direct){Finish(false,TEXT("SHIFT key never reached Boost()"));return;}
   Finish(false,TEXT("Boost() refused with a full bank"));return;
  }
  if(Base->Ride->BoostRemaining<=0){Finish(false,TEXT("SHIFT did not start the burst"));return;}
  S.Stage=3;
 }
 if(S.Stage==3&&S.Clock>2.6f){
  // Gear 1 arcade tops out at 650 cm/s, so anything past 1500 is the burst.
  if(S.Peak<1500||Base->Ride->BoostRemaining<=0){Finish(false,TEXT("Burst did not reach temporary speed"));return;}
  Shot(TEXT("boost-active.png"));Key(EKeys::S,true);S.Stage=4;
 }
 if(S.Stage==4&&S.Clock>4.4f){
  if(Base->Ride->BoostRemaining>0){Finish(false,TEXT("Burst did not expire on its own"));return;}
  Shot(TEXT("boost-expired.png"));S.Stage=5;
 }
 if(S.Stage==5&&S.Clock>4.7f){
  Key(EKeys::S,false);
  if(Base->Ride->Speed>700){Finish(false,TEXT("Braking did not shed the earned speed"));return;}
  S.Stage=6;
 }
 if(S.Stage==6&&S.Clock>5.1f){
  Base->BoostCharges=0;Base->Nitro=0;
  if(Base->Boost()){Finish(false,TEXT("Boost fired with an empty bank"));return;}
  S.Stage=7;
 }
 if(S.Stage==7&&S.Clock>5.5f){
  Base->Ride->Speed=0;Base->Ride->StopMovementImmediately();
  if(!Base->Dismount()){Finish(false,TEXT("Dismount failed"));return;}
  auto* Person=Cast<ABattleRider>(PC->GetPawn());if(!Person){Finish(false,TEXT("Missing FPS rider"));return;}
  auto* OnFoot=Crate(Person);if(!OnFoot){Finish(false,TEXT("Foot crate fixture failed"));return;}
  if(OnFoot->TryCollect(Person)||OnFoot->bConsumed){Finish(false,TEXT("Foot rider banked a mounted-only boost"));return;}
  Finish(true,TEXT("Banked throttle boosts: one charge per crate, three-deep cap, SHIFT spends it, timed burst and expiry, empty-bank and foot guards all pass"));
 }
#endif
}
