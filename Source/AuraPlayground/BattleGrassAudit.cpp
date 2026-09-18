#include "BattleBike.h"
#include "BattleHints.h"
#include "BattleCheckpoints.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Components/StaticMeshComponent.h"
#include "PiedmontPathSpline.h"
#include "Components/SplineComponent.h"

/**
 * Proves the off-piste rules: riding grass costs extra time on top of the
 * ticking clock, the Trees ATL crew comes after the rider on the BeltLine, and
 * the hint system actually surfaces a message.
 */
void TickBattleGrassAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int Phase=0;float Clock=0;float StartTime=0;float GrassSeen=0;int32 Chases=0;bool bHintSeen=false;};
 static FState S;
 if(!PC||!PC->GetWorld()||PC->GetWorld()->GetTimeSeconds()<5)return;
 auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(PC));
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());
 if(!Mode||!Bike||!Bike->Ride){S.Clock+=Dt;if(S.Clock>40){UE_LOG(LogTemp,Display,TEXT("BattleGrassAudit: {\"passed\":false,\"reason\":\"no bike or mode\"}"));PC->ConsoleCommand(TEXT("quit"));}return;}
 S.Clock+=Dt;
 if(Mode->HintRemaining>0&&!Mode->HintText.IsEmpty())S.bHintSeen=true;
 auto Key=[&](FKey K,bool Down){PC->InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Finish=[&](bool Pass,const TCHAR* Why){
   UE_LOG(LogTemp,Display,TEXT("BattleGrassAudit: {\"passed\":%s,\"reason\":\"%s\",\"time_delta\":%.1f,\"grass_seconds\":%.1f,\"chases\":%d,\"hint_seen\":%s}"),
   Pass?TEXT("true"):TEXT("false"),Why,S.StartTime-Mode->TimeRemaining,S.GrassSeen,S.Chases,S.bHintSeen?TEXT("true"):TEXT("false"));
  Key(EKeys::W,false);PC->ConsoleCommand(TEXT("quit"));
 };
 if(S.Phase==0){
  // Park the bike on open grass up on the BeltLine stretch.
  for(const FVector& Candidate:{FVector(6000.f,22000.f,0),FVector(9000.f,26000.f,0),FVector(12000.f,30000.f,0),FVector(3000.f,24000.f,0)}){
   FHitResult Ground;FCollisionQueryParams Q;Q.AddIgnoredActor(Bike);
   if(!PC->GetWorld()->LineTraceSingleByChannel(Ground,Candidate+FVector(0,0,3000),Candidate-FVector(0,0,3000),ECC_Visibility,Q))continue;
   Bike->SetActorLocationAndRotation(Ground.ImpactPoint+FVector(0,0,98),FRotator::ZeroRotator,false,nullptr,ETeleportType::TeleportPhysics);
   Bike->Ride->SetMovementMode(MOVE_Walking);Bike->Ride->bForceNextFloorCheck=true;Bike->Ride->Speed=0;
   S.Phase=1;S.Clock=0;return;
  }
  Finish(false,TEXT("no grass vantage found"));return;
 }
 if(S.Phase==1&&S.Clock>.4f){
  S.GrassSeen=Bike->Ride->bGrass?1.f:0.f;
  if(!Bike->Ride->bGrass){Finish(false,TEXT("rider is not on grass at the vantage"));return;}
  S.StartTime=Mode->TimeRemaining;S.Phase=2;S.Clock=0;Key(EKeys::W,true);return;
 }
 if(S.Phase==2){
  S.GrassSeen+=Bike->Ride->bGrass?Dt:0.f;
  if(S.Clock<6.f)return;
  Key(EKeys::W,false);
  const float Lost=S.StartTime-Mode->TimeRemaining;
  int32 Chases=0;for(TActorIterator<ABattleGrassWatch> It(PC->GetWorld());It;++It)Chases=It->Chases;
  S.Chases=Chases;
  const bool bTime=Lost>=S.GrassSeen*1.5f+2.f;   // clock plus the grass penalty
  const bool bPursuit=Chases>0;
  const bool bHint=S.bHintSeen;
  // The board asked for exists, is built right and stands beside the route on
  // the way to Monroe. Its board used to be a child of the scaled post, which
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
  if(!bTime||!bPursuit||!bHint)Finish(false,bTime?(bPursuit?(bHint?TEXT("grass rules live"):TEXT("no hint shown")):TEXT("no Trees ATL pursuit")):TEXT("grass time penalty too small"));
  else if(!bSign)Finish(false,*SignReason);
  else Finish(true,TEXT("grass penalty, Trees ATL pursuit, hints and the roadside sign all live"));
 }
#endif
}
