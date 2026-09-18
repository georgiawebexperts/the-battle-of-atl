#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "AIController.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "Misc/CommandLine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void TickBattleGrassLinkAudit(APlayerController* PC,float Dt);
void TickBattleConstructionAudit(APlayerController* PC,float Dt);
void TickBattleSleeperChaseAudit(APlayerController* PC,float Dt){
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleConstructionAudit"))){TickBattleConstructionAudit(PC,Dt);return;}
 if(FParse::Param(FCommandLine::Get(),TEXT("BattleGrassLinkAudit"))){TickBattleGrassLinkAudit(PC,Dt);return;}
#if !UE_BUILD_SHIPPING
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> Person,Target;FVector Mark=FVector::ZeroVector;bool bHasMark=false,bChasing=false;int32 Phase=0;float Clock=0,Total=0,StartDistance=0,Closest=100000;bool Done=false;};
 static FState S;if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<6)return;S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleSleeperChaseAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"start_distance\":%.3f,\"closest_distance\":%.3f}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Reason,S.StartDistance,S.Closest);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define CHECK_CHASE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 const bool FullCycle=FParse::Param(FCommandLine::Get(),TEXT("BattleSleeperFullCycle"));
 CHECK_CHASE(S.Total<60,"Timed out");
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(PC->GetWorld());
 // WakeAndChase only fires when the target is inside 600 cm with clear line of
 // sight, so the fixture has to stand on a mark that satisfies both. This used
 // to be a handful of random nav samples taken once, which committed the audit
 // to whichever pedestrian came first in world iteration order even when that
 // pedestrian had nowhere to put a target. Walk rings of directions instead,
 // from where the sleeper is now, and report the counters if nothing fits.
 struct FMarkTally{int32 Tried=0,NoNav=0,Crowded=0,Blocked=0;};
 auto FindMark=[&](APiedmontPedestrian* Who,APiedmontPedestrian* Fixture,FVector& Out,FMarkTally& Tally)->bool{
  if(!Nav)return false;
  const FVector From=Who->GetActorLocation();
  for(const float Radius:{300.f,400.f,500.f}){
   for(int32 Step=0;Step<12;++Step){
    ++Tally.Tried;
    const float Angle=float(Step)*2.f*PI/12.f;
    const FVector Want=From+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);
    FNavLocation Point;
    if(!Nav->ProjectPointToNavigation(Want,Point,FVector(200,200,300))){++Tally.NoNav;continue;}
    if(FVector::Dist2D(Point.Location,From)<250){++Tally.Crowded;continue;}
    const FVector P=Point.Location+FVector(0,0,90);
    FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperChaseMark),false,Who);if(Fixture)Q.AddIgnoredActor(Fixture);
    FHitResult Hit;
    if(Who->GetWorld()->OverlapBlockingTestByChannel(P,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,86),Q)||
     Who->GetWorld()->LineTraceSingleByChannel(Hit,From,P,ECC_Visibility,Q)){++Tally.Blocked;continue;}
    Out=P;return true;
   }
  }
  return false;
 };
 if(S.Phase==0){
  CHECK_CHASE(Nav,"Navigation missing");
  for(TActorIterator<APiedmontTrafficDirector> It(PC->GetWorld());It;++It)It->SetActorTickEnabled(false);
  APiedmontPedestrian* Selected=nullptr;FVector Mark;
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It&&!Selected;++It){
   // The first step below is "put this person to sleep", so the candidate has
   // to be awake. Placed ambient sleepers are already down (bAmbientSleeper
   // calls BeginSleeping on spawn) and they used to get picked here, which made
   // BeginSleeping refuse and the audit report a sleep defect that never
   // happened. BattleSleeperTriggerAudit is the one that covers woken bums.
   if(It->bDead||It->SleepPhase||It->bAmbientSleeper||!It->bNativeCrowdRig||!It->Body||!It->Body->GetSkinnedAsset()||!It->Body->GetSkinnedAsset()->GetName().StartsWith(TEXT("m_tal_nrw")))continue;
   // WakeFromSleep refuses to stand up when something overlaps the standing
   // capsule, and a crowd on the Beltline puts plenty of bodies inside each
   // other. Picking one of those made this audit fail on a defect that was not
   // there, so the candidate has to have room to stand before it is chosen.
   FCollisionQueryParams StandQ(SCENE_QUERY_STAT(SleeperChaseStandRoom),false,*It);
   if(PC->GetWorld()->OverlapBlockingTestByChannel(It->GetActorLocation(),FQuat::Identity,ECC_Pawn,
    FCollisionShape::MakeCapsule(It->GetCapsuleComponent()->GetScaledCapsuleRadius(),It->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-2.f),StandQ))continue;
   // A candidate is only useful if a fixture can stand beside it.
   FMarkTally Probe;
   if(!FindMark(*It,nullptr,Mark,Probe))continue;
   Selected=*It;
  }
  CHECK_CHASE(Selected,"No awake crowd sleeper with room to stand and a clear mark");
  Selected->PauseRemaining=100;Selected->GroupLeader=nullptr;
  // The target is a fixture, not a pedestrian under test. Left to itself it
  // panics or yields to the bike and walks out of WakeAndChase's 600 cm gate,
  // which is how this audit reported "Chase wake rejected" with the target 725
  // to 745 cm away. It is held on its mark by the tick below instead of having
  // its movement disabled: a MOVE_None pawn cannot be a path destination, so
  // MoveTo failed on the first chase tick and the sleeper never moved.
  FMarkTally Pick;
  if(!FindMark(Selected,nullptr,Mark,Pick)){Finish(false,TEXT("No mark for the target fixture"));return;}
  auto* Target=PC->GetWorld()->SpawnActor<APiedmontPedestrian>(Mark,FRotator::ZeroRotator);
  CHECK_CHASE(Target,"Target spawn failed");
  Target->PauseRemaining=100;Target->GroupLeader=nullptr;
  S.Person=Selected;S.Target=Target;S.Mark=Mark;S.bHasMark=true;S.Phase=1;S.Clock=0;return;
 }
 auto* Person=S.Person.Get();CHECK_CHASE(Person&&!Person->bDead,"Sleeper lost");
 // Hold the fixture on its mark every tick. This is the same trick
 // BattleSleeperTriggerAudit uses to keep its target at a fixed distance, and it
 // leaves the pawn pathable, which freezing it did not.
 if(S.bHasMark&&S.Phase>=2&&S.Target.IsValid())S.Target->SetActorLocation(S.Mark,false,nullptr,ETeleportType::TeleportPhysics);
 if(S.Phase==1&&S.Clock>.5f){
  // BeginSleeping refuses for six different reasons and one message covered all
  // of them. Name the one that fired, with the mesh, rather than trying a fourth
  // guess at it - the same rewriting is what ended the drone diagnosis in one run.
  if(!Person->BeginSleeping()){
   const FString Why=FString::Printf(TEXT("Sleep entry failed: swimming=%d knockdown=%d stumble=%.2f sleep_phase=%d bench_reaching=%d incident_pose=%d dead=%d mesh=%s native=%d"),
    Person->bSwimming?1:0,Person->KnockdownPhase,Person->StumbleRemaining,Person->SleepPhase,
    Person->bBenchReaching?1:0,Person->bIncidentPosing?1:0,Person->bDead?1:0,
    *GetNameSafe(Person->Body?Person->Body->GetSkinnedAsset():nullptr),Person->bNativeCrowdRig?1:0);
   Finish(false,*Why);return;
  }
  S.Phase=2;S.Clock=0;return;
 }
 if(S.Phase==2&&S.Clock>.5f){
  auto* WakeTarget=S.Target.Get();CHECK_CHASE(WakeTarget,"Target vanished");
  // Choose the mark from where the sleeper is now. A body in a crowd gets
  // shuffled on its way down, so a mark measured at selection time can already
  // be past the 600 cm gate by the time the wake is attempted.
  FVector Mark;FMarkTally Tally;
  if(!FindMark(Person,WakeTarget,Mark,Tally)){
   // The crowd keeps moving, so a mark that worked a moment ago can be full of
   // bodies now. Keep re-reading it while the sleeper stays down.
   if(S.Clock<4.f)return;
   const FString Why=FString::Printf(TEXT("No clear mark beside the sleeper: tried=%d unreachable=%d too_close=%d blocked=%d"),
    Tally.Tried,Tally.NoNav,Tally.Crowded,Tally.Blocked);
   Finish(false,*Why);return;
  }
  WakeTarget->SetActorLocation(Mark,false,nullptr,ETeleportType::TeleportPhysics);
  S.Mark=Mark;S.bHasMark=true;
  if(!Person->WakeAndChase(WakeTarget)){
   // WakeAndChase refuses for four reasons and "Chase wake rejected" covered
   // all of them. Re-run the same read-only checks it does and name the one
   // that fired, rather than guessing at the fixture a third time.
   const float Dist=WakeTarget?float(FVector::Dist2D(WakeTarget->GetActorLocation(),Person->GetActorLocation())):-1.f;
   bool bLos=false;FString Blocker;
   if(WakeTarget){
    FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperChaseWakeLos),false,Person);Q.AddIgnoredActor(WakeTarget);
    FHitResult Hit;
    bLos=Person->GetWorld()->LineTraceSingleByChannel(Hit,Person->GetActorLocation(),WakeTarget->GetActorLocation(),ECC_Visibility,Q);
    if(bLos)Blocker=GetNameSafe(Hit.GetActor());
   }
   FCollisionQueryParams SQ(SCENE_QUERY_STAT(SleeperChaseWakeStand),false,Person);
   const bool bStand=Person->GetWorld()->OverlapBlockingTestByChannel(Person->GetActorLocation(),FQuat::Identity,ECC_Pawn,
    FCollisionShape::MakeCapsule(Person->GetCapsuleComponent()->GetScaledCapsuleRadius(),Person->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-2.f),SQ);
   const bool bAnim=LoadObject<UAnimSequence>(nullptr,TEXT("/Game/BattleRetarget/Mixamo/SleepCandidate/SleepToStand_R"))!=nullptr;
   const FString Why=FString::Printf(TEXT("Chase wake rejected: target_valid=%d dist_cm=%.1f los_blocked=%d blocker=%s stand_blocked=%d anim_ok=%d sleep_phase=%d"),
    WakeTarget?1:0,Dist,bLos?1:0,*Blocker,bStand?1:0,bAnim?1:0,Person->SleepPhase);
   Finish(false,*Why);return;
  }
  S.StartDistance=FVector::Dist2D(Person->GetActorLocation(),S.Target->GetActorLocation());S.Phase=3;S.Clock=0;return;
 }
 if(S.Phase==3){
  CHECK_CHASE(S.Target.IsValid(),"Target disappeared unexpectedly");
  // WakeAndChase leaves the pedestrian in SleepPhase 2 playing SleepToStand_R;
  // the chase proper starts when that clip finishes and SleepPhase becomes 3.
  // Grading anything before that scored a normal stand-up as "did not approach".
  if(!S.bChasing){
   CHECK_CHASE(Person->SleepPhase==2||Person->SleepPhase==3,"Sleeper never started to wake");
   CHECK_CHASE(S.Clock<8,"Sleeper never started the chase");
   if(Person->SleepPhase!=3)return;
   S.bChasing=true;S.Clock=0;S.Closest=100000;
   S.StartDistance=FVector::Dist2D(Person->GetActorLocation(),S.Target->GetActorLocation());
   return;
  }
  S.Closest=FMath::Min(S.Closest,float(FVector::Dist2D(Person->GetActorLocation(),S.Target->GetActorLocation())));
  // SleepPhase 3 is the chase itself, and it ends after six seconds, on a lost
  // target, or at the origin gate. Grade the approach when it ends or at 7.5 s,
  // then grade the hand-back the moment it happens. The old fixed 11.8 s sample
  // landed two seconds after the pedestrian had been released back into its
  // walk, so "started walking again" was scored as a failure to restore speed.
  // The chase runs for six seconds and then releases the pedestrian, so the
  // approach is graded when it ends. Sample a little late as a backstop for a
  // chase that keeps going.
  if(S.Clock>7.5f||Person->SleepPhase!=3){
   if(S.Closest>=S.StartDistance-70){
    auto* AI=Cast<AAIController>(Person->GetController());
    const FString Why=FString::Printf(TEXT("Chaser did not approach: start=%.1f closest=%.1f sleep_phase=%d ai=%d move_status=%d speed=%.1f max_walk=%.1f target_ok=%d pause=%.1f"),
     S.StartDistance,S.Closest,Person->SleepPhase,AI?1:0,AI?int32(AI->GetMoveStatus()):-1,
     Person->GetVelocity().Size(),Person->GetCharacterMovement()->MaxWalkSpeed,S.Target.IsValid()?1:0,Person->PauseRemaining);
    Finish(false,*Why);return;
   }
  }
  CHECK_CHASE(S.Clock<15,"Chase never ended");
  if(Person->SleepPhase!=3){
   CHECK_CHASE(Person->SleepPhase==0,"Chase did not release the pedestrian");
   CHECK_CHASE(Person->GetCharacterMovement()->MaxWalkSpeed==135,"Chase did not restore walking speed");
   CHECK_CHASE(Person->GetVelocity().Size()<1,"Chaser still drifting after the chase");
   CHECK_CHASE(Person->BeginSleeping(),"Repeat sleep failed");
   S.Phase=4;S.Clock=0;return;
  }
 }
 if(S.Phase==4&&S.Clock>.5f){Person->bReturnToSleepAfterChase=FullCycle;CHECK_CHASE(Person->WakeAndChase(S.Target.Get()),"Second chase wake rejected");S.Phase=5;S.Clock=0;return;}
 if(S.Phase==5&&S.Clock>5.7f){CHECK_CHASE(Person->SleepPhase==3,"Second chase did not start");S.Target->Destroy();S.Phase=6;S.Clock=0;return;}
 if(S.Phase==6&&FullCycle){
  if(Person->SleepPhase==1){Person->Body->RefreshBoneTransforms();Finish(true,TEXT("approach, timeout, target loss and return to sleep passed"));return;}
  CHECK_CHASE(S.Clock<17,"Full cycle failed to settle back to sleep");return;
 }
 if(S.Phase==6&&S.Clock>.2f){CHECK_CHASE(Person->SleepPhase==0&&Person->GetVelocity().Size()<1,"Lost target did not cancel pursuit");Finish(true,TEXT("approach, timeout, speed restoration and target loss passed"));}
#undef CHECK_CHASE
#endif
}
