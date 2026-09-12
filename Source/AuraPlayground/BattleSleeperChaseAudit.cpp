#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
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
 struct FState{TWeakObjectPtr<UWorld> World;TWeakObjectPtr<APiedmontPedestrian> Person,Target;int32 Phase=0;float Clock=0,Total=0,StartDistance=0,Closest=100000;bool Done=false;};
 static FState S;if(S.World!=PC->GetWorld()){S=FState();S.World=PC->GetWorld();}
 if(S.Done||PC->GetWorld()->GetTimeSeconds()<6)return;S.Clock+=Dt;S.Total+=Dt;
 auto Finish=[&](bool Pass,const TCHAR* Reason){S.Done=true;UE_LOG(LogTemp,Display,TEXT("BattleSleeperChaseAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"start_distance\":%.3f,\"closest_distance\":%.3f}"),Pass?TEXT("true"):TEXT("false"),S.Phase,Reason,S.StartDistance,S.Closest);UKismetSystemLibrary::QuitGame(PC,PC,EQuitPreference::Quit,false);};
#define CHECK_CHASE(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 const bool FullCycle=FParse::Param(FCommandLine::Get(),TEXT("BattleSleeperFullCycle"));
 CHECK_CHASE(S.Total<60,"Timed out");
 if(S.Phase==0){
  auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(PC->GetWorld());CHECK_CHASE(Nav,"Navigation missing");
  for(TActorIterator<APiedmontTrafficDirector> It(PC->GetWorld());It;++It)It->SetActorTickEnabled(false);
  FVector Goal;APiedmontPedestrian* Selected=nullptr;
  for(TActorIterator<APiedmontPedestrian> It(PC->GetWorld());It&&!Selected;++It){
   if(It->bDead||!It->bNativeCrowdRig||!It->Body->GetSkinnedAsset()->GetName().StartsWith(TEXT("m_tal_nrw")))continue;
   for(int32 Try=0;Try<20;++Try){
    FNavLocation Point;if(!Nav->GetRandomReachablePointInRadius(It->GetActorLocation(),550,Point)||FVector::Dist2D(Point.Location,It->GetActorLocation())<300)continue;
    const FVector P=Point.Location+FVector(0,0,90);FCollisionQueryParams Q(SCENE_QUERY_STAT(SleeperChasePlacement),false,*It);FHitResult Hit;
    if(PC->GetWorld()->OverlapBlockingTestByChannel(P,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(32,86),Q)||PC->GetWorld()->LineTraceSingleByChannel(Hit,It->GetActorLocation(),P,ECC_Visibility,Q))continue;
    Selected=*It;Goal=P;break;
   }
  }
  CHECK_CHASE(Selected,"No reachable clear sleeper/target pair");
  Selected->PauseRemaining=100;Selected->GroupLeader=nullptr;
  auto* Target=PC->GetWorld()->SpawnActor<APiedmontPedestrian>(Goal,FRotator::ZeroRotator);CHECK_CHASE(Target,"Target spawn failed");
  Target->PauseRemaining=100;Target->GroupLeader=nullptr;
  S.Person=Selected;S.Target=Target;S.Phase=1;S.Clock=0;return;
 }
 auto* Person=S.Person.Get();CHECK_CHASE(Person&&!Person->bDead,"Sleeper lost");
 if(S.Phase==1&&S.Clock>.5f){CHECK_CHASE(Person->BeginSleeping(),"Sleep entry failed");S.Phase=2;S.Clock=0;return;}
 if(S.Phase==2&&S.Clock>.5f){CHECK_CHASE(Person->WakeAndChase(S.Target.Get()),"Chase wake rejected");S.StartDistance=FVector::Dist2D(Person->GetActorLocation(),S.Target->GetActorLocation());S.Phase=3;S.Clock=0;return;}
 if(S.Phase==3){
  CHECK_CHASE(S.Target.IsValid(),"Target disappeared unexpectedly");
  S.Closest=FMath::Min(S.Closest,float(FVector::Dist2D(Person->GetActorLocation(),S.Target->GetActorLocation())));
  if(S.Clock>7.5f)CHECK_CHASE(S.Closest<S.StartDistance-70,"Chaser did not approach");
  if(S.Clock>11.8f){CHECK_CHASE(Person->SleepPhase==0&&Person->GetVelocity().Size()<1&&Person->GetCharacterMovement()->MaxWalkSpeed==135,"Chase did not stop and restore speed");
   CHECK_CHASE(Person->BeginSleeping(),"Repeat sleep failed");S.Phase=4;S.Clock=0;return;}
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
