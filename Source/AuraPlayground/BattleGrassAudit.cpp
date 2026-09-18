#include "BattleBike.h"
#include "BattleHints.h"
#include "BattleCheckpoints.h"
#include "PiedmontBike.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Components/StaticMeshComponent.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"

/**
 * Proves the off-piste rules, both halves of them:
 *
 *   - the PARK lawn is free. Elliott: "you should be able to ride in the park on
 *     the grass no problem. its the beltline that you should not be able to ride
 *     on the grass without penalty."
 *   - the BELTLINE lawn costs a little time, and brings the Trees ATL crew, but
 *     not a harsh amount: "sometimes you need to ride on the grass to avoid
 *     people."
 *
 * The two used to be one rule that punished grass everywhere.
 */
void TickBattleGrassAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int Phase=0;float Clock=0;float StartTime=0;float GrassSeen=0;float Window=0;int32 Chases=0;float ParkLost=0;float BeltLost=0;bool bHintSeen=false;};
 static FState S;
 if(!PC||!PC->GetWorld()||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(PC));
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());
 if(!Mode||!Bike||!Bike->Ride){S.Clock+=Dt;if(S.Clock>40){UE_LOG(LogTemp,Display,TEXT("BattleGrassAudit: {\"passed\":false,\"reason\":\"no bike or mode\"}"));PC->ConsoleCommand(TEXT("quit"));}return;}
 S.Clock+=Dt;
 if(Mode->HintRemaining>0&&!Mode->HintText.IsEmpty())S.bHintSeen=true;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){
   UE_LOG(LogTemp,Display,TEXT("BattleGrassAudit: {\"passed\":%s,\"reason\":\"%s\",\"park_lost\":%.1f,\"belt_lost\":%.1f,\"belt_grass_seconds\":%.1f,\"chases\":%d,\"hint_seen\":%s}"),
   Pass?TEXT("true"):TEXT("false"),Why,S.ParkLost,S.BeltLost,S.GrassSeen,S.Chases,S.bHintSeen?TEXT("true"):TEXT("false"));
  Key(EKeys::W,false);PC->ConsoleCommand(TEXT("quit"));
 };
 // Find lawn to stand on: inside the park (below the BeltLine) or on it.
 auto FindLawn=[&](bool bBelt,FVector& Out)->bool{
  for(TActorIterator<APiedmontPathSpline> Path(PC->GetWorld());Path;++Path){
   auto* Route=Path->Centerline.Get();if(!Route)continue;
   const float Length=Route->GetSplineLength();
   const float HalfWidth=Path->WidthCm*.5f;
   for(float D=500.f;D<Length;D+=900.f){
    const FVector P=Route->GetLocationAtDistanceAlongSpline(D,ESplineCoordinateSpace::World);
    if(BattleGrassRules::IsBeltLine(P)!=bBelt)continue;
     // Step off the path itself: the pavement under a spline is RidePath, and
     // the lawn is beside it. Sampling the centreline only ever found tarmac.
     const FVector Dir=Route->GetDirectionAtDistanceAlongSpline(D,ESplineCoordinateSpace::World).GetSafeNormal2D();
     if(Dir.IsNearlyZero())continue;
     const FVector Side=FVector::CrossProduct(FVector::UpVector,Dir).GetSafeNormal();
     for(float Off:{HalfWidth+320.f,HalfWidth+900.f,-(HalfWidth+320.f),-(HalfWidth+900.f)}){
      const FVector S=P+Side*Off;
      FHitResult G;FCollisionQueryParams Q;Q.bIgnoreTouches=true;Q.AddIgnoredActor(Bike);
      if(!PC->GetWorld()->LineTraceSingleByChannel(G,S+FVector(0,0,2000),S-FVector(0,0,2500),ECC_Visibility,Q))continue;
      if(!G.GetActor()||!G.GetActor()->ActorHasTag(TEXT("RideGrass")))continue;   // lawn, not pavement
      if(G.ImpactNormal.Z<.9f)continue;                                          // flat enough to ride
      const FVector At=G.ImpactPoint+FVector(0,0,98);
      bool bWet=false;for(TActorIterator<APiedmontWaterHazard> Water(PC->GetWorld());Water;++Water)if(Water->ContainsBike(At)){bWet=true;break;}
      if(bWet)continue;
      Out=At;return true;
     }
   }
  }
  return false;
 };
 auto Stand=[&](const FVector& At){
  Bike->SetActorLocationAndRotation(At,FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);
  Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;Bike->Ride->Speed=0;
 };
 if(S.Phase==0){
  FVector At;if(!FindLawn(false,At)){Finish(false,TEXT("no park lawn to ride"));return;}
  UE_LOG(LogTemp,Display,TEXT("GrassAuditPark: vantage=%s"),*At.ToString());
  Stand(At);S.Phase=1;S.Clock=0;return;
 }
 if(S.Phase==1&&S.Clock>.4f){
  if(!Bike->Ride->bGrass){Finish(false,TEXT("rider is not on park grass at the vantage"));return;}
  S.StartTime=Mode->TimeRemaining;S.GrassSeen=0;S.Phase=2;S.Clock=0;Key(EKeys::W,true);return;
 }
 if(S.Phase==2){
  S.GrassSeen+=Bike->Ride->bGrass?Dt:0.f;
  if(S.Clock<6.f)return;
  Key(EKeys::W,false);
  S.ParkLost=S.StartTime-Mode->TimeRemaining;
  S.Window=S.Clock;
  int32 Chases=0;for(TActorIterator<ABattleGrassWatch> It(PC->GetWorld());It;++It)Chases=It->Chases;
  // The clock ticks the whole time the rider is out there, so a fair ride loses
  // roughly the elapsed window and nothing more. A penalty shows up as excess
  // over that window, not over the seconds spent on grass.
  if(Chases>0){Finish(false,TEXT("Trees ATL came after the rider on park grass"));return;}
  if(S.ParkLost>S.Window+1.5f){Finish(false,TEXT("park grass cost extra time"));return;}
  FVector At;if(!FindLawn(true,At)){Finish(false,TEXT("no BeltLine lawn to ride"));return;}
  UE_LOG(LogTemp,Display,TEXT("GrassAuditBelt: vantage=%s park_lost=%.1f park_seconds=%.1f"),*At.ToString(),S.ParkLost,S.GrassSeen);
  Stand(At);S.Phase=3;S.Clock=0;return;
 }
 if(S.Phase==3&&S.Clock>.4f){
  if(!Bike->Ride->bGrass){Finish(false,TEXT("rider is not on BeltLine grass at the vantage"));return;}
  S.StartTime=Mode->TimeRemaining;S.GrassSeen=0;S.Phase=4;S.Clock=0;Key(EKeys::W,true);return;
 }
 if(S.Phase==4){
  S.GrassSeen+=Bike->Ride->bGrass?Dt:0.f;
  // A longer sample so one penalty tick cannot be lost in the difference
  // between our own clock and the run clock.
  if(S.Clock<9.f)return;
  Key(EKeys::W,false);
  S.BeltLost=S.StartTime-Mode->TimeRemaining;
  S.Window=S.Clock;
  int32 Chases=0;for(TActorIterator<ABattleGrassWatch> It(PC->GetWorld());It;++It)Chases=It->Chases;
  S.Chases=Chases;
  // One penalty tick is a second every two seconds of BeltLine lawn, so a few
  // seconds of grass should already show up as time lost beyond the bare clock.
  const bool bEnough=S.GrassSeen>=5.f;               // a fair sample of BeltLine lawn
  const bool bPenalty=S.BeltLost>=S.Window+1.5f;     // clock plus a visible extra loss
  const bool bGentle=S.BeltLost<=S.Window+5.f;       // ...but nowhere near harsh
  const bool bPursuit=Chases>0;
  const bool bHint=S.bHintSeen;
  // The board Elliott asked for exists, is built right and stands beside the
  // route on the way to Monroe. It used to be a child of the scaled post, which
  // crushed it to a sliver floating above the post.
  bool bSign=false;FString SignReason=TEXT("no Trees ATL sign in the world");
  for(TActorIterator<ABattleGrassSign> It(PC->GetWorld());It;++It){
   const UStaticMeshComponent* Board=nullptr;FVector BoardScale=FVector::ZeroVector;
   TInlineComponentArray<UStaticMeshComponent*> Parts(*It);
   for(auto* Part:Parts)if(Part->GetName()==TEXT("SignBoard")){Board=Part;BoardScale=Part->GetComponentScale();}
   const FVector Size=BoardScale*100.f;
   if(!Board){SignReason=TEXT("sign has no board");break;}
   if(!FMath::IsNearlyEqual(Size.X,6.f,1.f)||!FMath::IsNearlyEqual(Size.Y,150.f,3.f)||!FMath::IsNearlyEqual(Size.Z,90.f,3.f)){
    SignReason=FString::Printf(TEXT("board size is %.0fx%.0fx%.0f cm, expected about 6x150x90"),Size.X,Size.Y,Size.Z);break;
   }
   float Nearest=BIG_NUMBER;
   for(TActorIterator<APiedmontPathSpline> Path(PC->GetWorld());Path;++Path)if(auto* Route=Path->Centerline.Get())
    Nearest=FMath::Min(Nearest,float(FVector::Dist2D(Route->FindLocationClosestToWorldLocation(It->GetActorLocation(),ESplineCoordinateSpace::World),It->GetActorLocation())));
   if(Nearest>600.f){SignReason=FString::Printf(TEXT("sign stands %.0f cm off the route"),Nearest);break;}
   bSign=true;
  }
  if(!bEnough)Finish(false,TEXT("not enough BeltLine grass in the sample"));
  else if(!bPenalty)Finish(false,TEXT("no time cost for BeltLine grass"));
  else if(!bGentle)Finish(false,TEXT("BeltLine grass penalty is too harsh"));
  else if(!bPursuit)Finish(false,TEXT("no Trees ATL pursuit on the BeltLine"));
  else if(!bHint)Finish(false,TEXT("no hint shown"));
  else if(!bSign)Finish(false,*SignReason);
  else Finish(true,TEXT("park lawn rides free; BeltLine grass costs a gentle amount, draws Trees ATL, shows the hint and carries the roadside sign"));
 }
#endif
}
