#include "BattleMacController.h"
#include "BattleSpirit.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "PiedmontPedestrian.h"
#include "PiedmontTrafficDirector.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
void ABattleMacController::TickSpiritAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(bSpiritAuditDone||GetWorld()->GetTimeSeconds()<5)return;bSpiritAuditDone=true;
 auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));auto* B=Cast<ABattleBike>(GetPawn());int Checks=0;
 auto End=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleSpiritAudit: {\"passed\":%s,\"checks\":%d,\"reason\":\"%s\",\"presentation_enabled\":false}"),Pass?TEXT("true"):TEXT("false"),Checks,Reason);ConsoleCommand(TEXT("quit"));};
#define SPIRITCHECK(C,R) if(!(C)){End(false,TEXT(R));return;}else{Checks++;}
 SPIRITCHECK(M&&B&&M->Quest&&!M->bTutorialActive&&M->StartCountdown<=0,"Missing timed game");
 if(M->Enemies)M->Enemies->bFreezeSpawns=true;
 for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
 for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();for(TActorIterator<ABattleZombie> It(GetWorld());It;++It)It->Destroy();
 auto New=[&](){auto* S=GetWorld()->SpawnActor<ABattleSpirit>(B->GetActorLocation(),FRotator::ZeroRotator);S->SetActorTickEnabled(false);return S;};
 auto* S=New();SPIRITCHECK(S&&!S->bPresentationReady&&!S->GetActorEnableCollision()&&!S->CanBeDamaged(),"Spirit must be inert until art is ready");
 M->bTutorialActive=true;SPIRITCHECK(!S->TryApproach(0)&&S->State==EBattleSpiritState::Untried,"Tutorial consumed opportunity");M->bTutorialActive=false;
 M->StartCountdown=1;SPIRITCHECK(!S->TryApproach(0),"Countdown spawned spirit");M->StartCountdown=0;
 SPIRITCHECK(!S->TryApproach(-1)&&!S->TryApproach(1)&&S->State==EBattleSpiritState::Untried,"Invalid roll consumed opportunity");
 SPIRITCHECK(!S->TryApproach(.15f)&&S->State==EBattleSpiritState::Absent&&!S->TryApproach(0),"Rarity cutoff or repeated visit failed");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(.1499f)&&S->State==EBattleSpiritState::Appearing&&!S->TryCatch(),"Reveal must precede reward");
 UGameplayStatics::SetGamePaused(this,true);S->AdvanceEncounter(3);SPIRITCHECK(S->Age==0&&!S->TryCatch(),"Paused encounter advanced");UGameplayStatics::SetGamePaused(this,false);
 S->AdvanceEncounter(1.6f);SPIRITCHECK(S->State==EBattleSpiritState::Active,"Reveal never became active");
 auto* Mesh=NewObject<UStaticMeshComponent>(S);Mesh->SetupAttachment(S->GetRootComponent());Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere")));Mesh->SetCollisionProfileName(TEXT("BlockAll"));Mesh->RegisterComponent();
 FCollisionQueryParams Q(SCENE_QUERY_STAT(SpiritBullet),false,B);FHitResult H;const FVector From=S->GetActorLocation()+FVector(0,-100,0),To=S->GetActorLocation()+FVector(0,100,0);
 SPIRITCHECK(!GetWorld()->LineTraceSingleByChannel(H,From,To,ECC_Visibility,Q)||H.GetActor()!=S,"Projectile trace hit spirit mesh");
 S->SetActorEnableCollision(true);SPIRITCHECK(GetWorld()->LineTraceSingleByChannel(H,From,To,ECC_Visibility,Q)&&H.GetActor()==S,"Projectile test fixture was not blocking");S->SetActorEnableCollision(false);
 const FVector Phone=M->Quest->ArtifactLocation;const int Checkpoint=M->Quest->NextCheckpoint;const bool Collected=M->Quest->bCollected;const float Elapsed=M->RunElapsed;B->RiderHealth=35;M->TimeRemaining=50;
 SPIRITCHECK(S->TryCatch()&&B->RiderHealth==100&&FMath::IsNearlyEqual(M->TimeRemaining,M->Difficulty.TimeLimitSeconds)&&S->Rewards==1,"Mounted catch failed restore");
 SPIRITCHECK(M->Quest->ArtifactLocation==Phone&&M->Quest->NextCheckpoint==Checkpoint&&M->Quest->bCollected==Collected&&M->RunElapsed==Elapsed,"Catch reset quest or elapsed record");
 SPIRITCHECK(!S->TryCatch()&&!S->TryApproach(0),"Repeated reward or opportunity");S->AdvanceEncounter(2);SPIRITCHECK(S->State==EBattleSpiritState::Resolved,"Fade never resolved");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Fresh run fixture failed");S->AdvanceEncounter(2);M->TimeRemaining=M->Difficulty.TimeLimitSeconds+30;SPIRITCHECK(S->TryCatch()&&M->TimeRemaining==M->Difficulty.TimeLimitSeconds+30,"Catch removed earned time");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0)&&B->Dismount(),"Dismount fixture failed");auto* P=Cast<ABattleRider>(GetPawn());
 SPIRITCHECK(P&&S->State==EBattleSpiritState::Fading&&!S->TryCatch(),"Dismount did not synchronously cancel");
 auto* FootOpportunity=New();SPIRITCHECK(!FootOpportunity->TryApproach(0)&&FootOpportunity->State==EBattleSpiritState::Absent,"Foot approach should consume chance");
 SPIRITCHECK(P->MountBike()&&!S->TryApproach(0)&&!S->TryCatch()&&!FootOpportunity->TryApproach(0),"Same-frame remount restored chance");S->Destroy();FootOpportunity->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Expiry fixture failed");S->AdvanceEncounter(12);SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryCatch(),"Expired opportunity rewarded");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Stun fixture failed");B->StunRemaining=2;S->AdvanceEncounter(.1f);B->StunRemaining=0;SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryApproach(0),"Knockoff/stun failed cancellation");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"End fixture failed");M->bRunEnded=true;S->AdvanceEncounter(.1f);M->bRunEnded=false;SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryCatch(),"Run end restored opportunity");S->Destroy();
 S=New();SPIRITCHECK(S->TryApproach(0),"Death fixture failed");B->DamageGrace=0;B->ApplyRiderDamage(1000);SPIRITCHECK(S->State==EBattleSpiritState::Fading&&!S->TryCatch()&&!S->TryApproach(0),"Death failed immediate cancellation");
 End(true,TEXT("Native lifecycle, mounted restore, quest preservation, rarity boundary, pause, expiry, dismount/remount, stun, death and projectile pass-through pass; presentation pending"));
#undef SPIRITCHECK
#endif
}
