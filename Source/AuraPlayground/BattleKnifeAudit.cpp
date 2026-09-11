#include "BattleMacController.h"
#include "BattleKnife.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
void ABattleMacController::TickKnifeAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(KnifeAuditStage<0||GetWorld()->GetTimeSeconds()<5)return;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* P=Cast<ABattleRider>(GetPawn());auto* B=Cast<ABattleBike>(GetPawn());if(P)B=P->ParkedBike;
 auto End=[&](bool Pass,const TCHAR* Why){KnifeAuditStage=-1;UE_LOG(LogTemp,Display,TEXT("BattleKnifeAudit: {\"passed\":%s,\"reason\":\"%s\"}"),Pass?TEXT("true"):TEXT("false"),Why);ConsoleCommand(TEXT("quit"));};
#define KCHECK(C,R) if(!(C)){End(false,TEXT(R));return;}
 KCHECK(M&&B&&M->Quest,"Missing running game");KnifeAuditClock+=Dt;
 auto Advance=[&](int N){KnifeAuditStage=N;KnifeAuditClock=0;};
 auto Spawn=[&](float Distance){
  FNavLocation Spot;auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());if(!Nav||!Nav->ProjectPointToNavigation(B->GetActorLocation()+B->GetActorForwardVector()*Distance,Spot,FVector(300,300,300)))return false;
  KnifeAuditActor=GetWorld()->SpawnActor<ABattleKnife>(Spot.Location+FVector(0,0,90),FRotator::ZeroRotator);if(!KnifeAuditActor)return false;KnifeAuditOrigin=KnifeAuditActor->GetActorLocation();KnifeAuditActor->Cooldown=.1f;return true;
 };
 if(KnifeAuditStage==0){
  KCHECK(!M->bTutorialActive&&M->StartCountdown<=0,"Not timed run");if(M->Enemies)M->Enemies->bFreezeSpawns=true;
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
  KCHECK(FMath::IsNearlyEqual(ABattleKnife::SpawnChance(false,0),.03f)&&FMath::IsNearlyEqual(ABattleKnife::SpawnChance(true,12),.25f)&&ABattleKnife::SpawnChance(true,6)>ABattleKnife::SpawnChance(true,0),"Rarity formula mismatch");
  KnifeAuditTime=M->TimeRemaining;KCHECK(Spawn(700),"Cannot spawn pursuit fixture");Advance(1);return;
 }
 if(KnifeAuditStage==1){
  KCHECK(KnifeAuditClock<15,"Initial pursuit or first stab timed out");
  if(KnifeAuditActor->Stabs==1){KCHECK(P&&B->RiderHealth>0&&B->RiderHealth<100&&B->StunRemaining>0&&B->RespawnRemaining==0&&!P->bWeaponDrawn,"First stab did not force hands-free nonfatal dismount");KCHECK(KnifeAuditActor->PathRequests>0&&FVector::Dist2D(KnifeAuditOrigin,KnifeAuditActor->GetActorLocation())>150,"Attacker did not actually navigate");KCHECK(M->TimeRemaining<KnifeAuditTime,"Clock stopped during attack");Advance(2);}
 }else if(KnifeAuditStage==2){
  KCHECK(KnifeAuditClock<2&&KnifeAuditActor->Stabs==1,"Second stab during recovery window");
  if(B->StunRemaining<=0){KCHECK(P&&P->MountBike(),"Could not remount after recovery");KCHECK(KnifeAuditActor->bEscaped&&!KnifeAuditActor->ResolveStrike(),"Remount did not end chase immediately");KnifeAuditActor->Destroy();KCHECK(Spawn(600),"Cannot spawn death-sequence fixture");KnifeAuditResets=M->Quest->SearchResets;Advance(3);}
 }else if(KnifeAuditStage==3){
  KCHECK(KnifeAuditClock<18,"Two-hit chase timed out");
  if(KnifeAuditActor->Stabs==2){KCHECK(B->RiderHealth<=0&&B->RespawnRemaining>0&&M->Quest->SearchResets>KnifeAuditResets&&KnifeAuditActor->bEscaped,"Second stab failed death/reset");Advance(4);}
 }else if(KnifeAuditStage==4){
  KCHECK(KnifeAuditClock<7,"Recovery did not complete");
  if(B->RespawnRemaining<=0&&B->DamageGrace<=0){KCHECK(Cast<ABattleBike>(GetPawn())&&B->RiderHealth>0,"Death did not return rider to bike");if(IsValid(KnifeAuditActor))KnifeAuditActor->Destroy();Advance(5);}
 }else if(KnifeAuditStage==5){
  const FVector Direction=B->GetActorForwardVector();KnifeAuditActor=GetWorld()->SpawnActor<ABattleKnife>(B->GetActorLocation()+Direction*125,(-Direction).Rotation());KCHECK(KnifeAuditActor,"Cannot spawn reach fixture");KnifeAuditActor->SetActorTickEnabled(false);
  auto* Wall=GetWorld()->SpawnActor<AStaticMeshActor>(B->GetActorLocation()+Direction*62,Direction.Rotation());KCHECK(Wall,"Cannot create obstruction");auto* Mesh=Wall->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Wall->SetActorScale3D(FVector(.12,4,4));Mesh->SetCollisionProfileName(TEXT("BlockAll"));
  const float Before=B->RiderHealth;KnifeAuditActor->bWindingUp=true;KnifeAuditActor->WindupRemaining=0;KCHECK(!KnifeAuditActor->ResolveStrike()&&B->RiderHealth==Before&&KnifeAuditActor->Stabs==0,"Stab passed through wall");Wall->Destroy();
  KnifeAuditActor->bWindingUp=true;KnifeAuditActor->WindupRemaining=.5f;KCHECK(!KnifeAuditActor->ResolveStrike(),"Stab before windup finished");KnifeAuditActor->WindupRemaining=0;
  M->bTutorialActive=true;KCHECK(!KnifeAuditActor->ResolveStrike(),"Tutorial stab allowed");M->bTutorialActive=false;
  KCHECK(KnifeAuditActor->ResolveStrike()&&KnifeAuditActor->Stabs==1&&B->RiderHealth>0,"Unobstructed control strike failed");
  KnifeAuditActor->bWindingUp=true;KnifeAuditActor->WindupRemaining=0;KCHECK(!KnifeAuditActor->ResolveStrike()&&B->RiderHealth>0,"Stab bypassed recovery protection");
  FDamageEvent Damage;const float ClockBefore=M->TimeRemaining;KCHECK(KnifeAuditActor->TakeDamage(200,Damage,this,GetPawn())==100&&KnifeAuditActor->bDead&&!KnifeAuditActor->bWindingUp&&!KnifeAuditActor->ResolveStrike(),"Defeated attacker continued attacking");M->RecordPlayerShotHit(KnifeAuditActor);KCHECK(M->TimeRemaining==ClockBefore,"Knife attacker incorrectly awarded zombie time");
  End(true,TEXT("Actual pursuit, first-hit dismount/recovery, immediate remount escape, second-hit death/phone reset, obstruction, windup, tutorial protection and defeated-attacker cancellation pass"));
 }
#undef KCHECK
#endif
}
