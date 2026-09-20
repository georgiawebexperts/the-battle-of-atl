#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleQuest.h"
#include "BattleHome.h"
#include "BattleHomeData.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "BattleZombie.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

/**
 * Elliott rode to the party on packaged 128 and nothing happened: "I got to the
 * end and no end scene or credits were triggered."
 *
 * ABattleHome only accepts the finish once the phone is collected, two
 * checkpoints are down, the tunnel entry AND exit have been passed, and the
 * rider is inside 650 cm of the patio gate. Every one of those fails silently,
 * so the ride can end with the player parked at the party and no screen at all.
 *
 * This walks the authored route - BattleHomeData::Route, then Approach, the
 * same points the game drives - and reports which condition was still false
 * when the rider arrived, plus the closest the route ever comes to each gate.
 *
 *   -BattlePatioAudit
 */
void TickBattlePatioAudit(APlayerController* PC,float Dt){
#if !UE_BUILD_SHIPPING
 struct FState{int Phase=0,Index=0,Passes=0;float Clock=0,BestEntry=BIG_NUMBER,BestExit=BIG_NUMBER,BestGate=BIG_NUMBER;bool Done=false;};
 static FState S;
 if(S.Done||!PC||!PC->GetWorld())return;
 if(PC->GetWorld()->GetTimeSeconds()<5)return;
 S.Clock+=Dt;
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(PC));
 auto* Bike=Cast<ABattleBike>(PC->GetPawn());
 ABattleHome* Home=nullptr;for(TActorIterator<ABattleHome> It(PC->GetWorld());It;++It)Home=*It;
 if(!Mode||!Mode->Quest||!Bike||!Home)return;
 auto Report=[&](const TCHAR* Why){
  UE_LOG(LogTemp,Display,TEXT("BattlePatioAudit: {\"passed\":%s,\"reason\":\"%s\",\"collected\":%s,\"checkpoint\":%d,\"tunnel_entered\":%s,\"tunnel_exited\":%s,\"won\":%s,\"closest_entry_cm\":%.1f,\"closest_exit_cm\":%.1f,\"closest_gate_cm\":%.1f,\"points\":%d}"),
   Mode->bWon?TEXT("true"):TEXT("false"),Why,Mode->Quest->bCollected?TEXT("true"):TEXT("false"),Mode->Quest->NextCheckpoint,
   Home->bTunnelEntered?TEXT("true"):TEXT("false"),Home->bTunnelExited?TEXT("true"):TEXT("false"),Mode->bWon?TEXT("true"):TEXT("false"),
   S.BestEntry,S.BestExit,S.BestGate,S.Index);
  S.Done=true;PC->ConsoleCommand(TEXT("quit"));
 };
 auto Place=[&](const FVector& P){Bike->Ride->StopMovementImmediately();Bike->Ride->Speed=0;Bike->SetActorLocation(P,false,nullptr,ETeleportType::TeleportPhysics);};
 if(S.Phase==0){
  // Clear the world the same way the finish audit does: a pedestrian walking
  // past must not be able to knock the rider off mid-route.
  for(TActorIterator<APiedmontTrafficDirector> It(PC->GetWorld());It;++It)It->SetActorTickEnabled(false);
  if(Mode->Enemies)Mode->Enemies->bFreezeSpawns=true;
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It;++It)It->Destroy();
  for(TActorIterator<ABattleZombie> It(PC->GetWorld());It;++It)It->Destroy();
  Bike->Ride->Recovery=0;Bike->RiderHealth=100;
  // Collect the phone FIRST, then the checkpoints: the quest will not count a
  // checkpoint before the artifact is in hand, and doing it the other way round
  // made this walk report checkpoint=0 against a perfectly good course.
  Place(Mode->Quest->ArtifactLocation+FVector(0,0,53));Mode->Quest->Tick(.016f);
  for(const FVector& C:Mode->Quest->CheckpointLocations){Place(C+FVector(0,0,98));Mode->Quest->Tick(.016f);}
  Place(Mode->Quest->ArtifactLocation+FVector(0,0,53));Mode->Quest->Tick(.016f);
  if(!Mode->Quest->bCollected){Report(TEXT("the artifact could not be collected at its own location"));return;}
  S.Phase=1;S.Clock=0;return;
  }
 if(S.Phase==1){
  const int32 RoadCount=UE_ARRAY_COUNT(BattleHomeData::Road);
  const int32 RouteCount=UE_ARRAY_COUNT(BattleHomeData::Route);
  const int32 ApproachCount=UE_ARRAY_COUNT(BattleHomeData::Approach);
  // The whole authored course: the road in, the home drive, then the final
  // approach. Walking only the home drive cannot see whether a rider coming from
  // Krog Market ever passes the tunnel entry marker.
  const FVector P=S.Index<RoadCount?BattleHomeData::Road[S.Index]
   :S.Index<RoadCount+RouteCount?BattleHomeData::Route[S.Index-RoadCount]
   :BattleHomeData::Approach[FMath::Min(S.Index-RoadCount-RouteCount,ApproachCount-1)];
  Place(P+FVector(0,0,98));
  S.BestEntry=FMath::Min(S.BestEntry,float(FVector::Dist(Bike->GetActorLocation(),BattleHomeData::TunnelEntry)));
  S.BestExit=FMath::Min(S.BestExit,float(FVector::Dist(Bike->GetActorLocation(),BattleHomeData::TunnelExit)));
  S.BestGate=FMath::Min(S.BestGate,float(FVector::Dist2D(Bike->GetActorLocation(),BattleHomeData::Gate)));
  Home->Tick(Dt);Mode->Quest->Tick(Dt);
  if(Mode->bWon){Report(TEXT("the authored route reaches the patio and commits the win"));return;}
  ++S.Index;
  if(S.Index>RoadCount+RouteCount+ApproachCount){
   // Walk it again rather than calling the walk a failure. The course is driven
   // by placing the bike on each authored point and letting one frame go by, so
   // how much of the checkpoint state machine advances per point depends on the
   // frame - and on 2026-09-19 this went red inside the 31-audit sweep after 157
   // points with the tunnel never entered, then passed alone on the same binary.
   // The promise is that the authored route reaches the patio, not that it does
   // so in exactly one pass.
   if(++S.Passes<3){S.Index=0;return;}
   Report(TEXT("route exhausted without a win after three passes"));
   return;
  }
  return;
 }
 if(S.Clock>60.f)Report(TEXT("timed out"));
#endif
}
