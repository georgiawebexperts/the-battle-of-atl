#include "BattleZombie.h"
#include "BattleGunman.h"
#include "BattleBike.h"
#include "BattleQuest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
void TickBattleGunmanDirectorAudit(ABattleEnemyDirector* D,float Dt){
#if !UE_BUILD_SHIPPING
 auto* W=D->GetWorld();if(W->GetTimeSeconds()<6)return;
 static bool Finished=false;if(Finished)return;Finished=true;
 auto End=[&](bool Pass,const TCHAR* Why){UE_LOG(LogTemp,Display,TEXT("GunmanDirectorAudit: {\"passed\":%s,\"reason\":\"%s\",\"spawned\":%d}"),Pass?TEXT("true"):TEXT("false"),Why,D->GunmenSpawned);FPlatformMisc::RequestExit(false);};
#define CHECK_GD(C,R) if(!(C)){End(false,TEXT(R));return;}
 auto* B=Cast<ABattleBike>(UGameplayStatics::GetPlayerPawn(W,0));auto* M=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(W));CHECK_GD(B&&M&&M->Quest,"Missing player or quest");
 B->DamageGrace=0;B->RespawnRemaining=0;B->Ride->StopMovementImmediately();M->Trouble=0;M->StartCountdown=0;M->Quest->bCollected=false;M->bTutorialActive=false;
 D->TickGunmen(1000);CHECK_GD(D->GunmenSpawned==0&&D->GunmanDelay==60&&!D->SpawnGunman(),"Quiet search permitted shooter");
 M->Quest->bCollected=true;M->bTutorialActive=true;D->TickGunmen(1000);CHECK_GD(D->GunmanDelay==60&&!D->SpawnGunman(),"Practice permitted shooter");M->bTutorialActive=false;
 M->StartCountdown=2;D->TickGunmen(1000);CHECK_GD(D->GunmanDelay==60&&!D->SpawnGunman(),"Countdown permitted shooter");M->StartCountdown=0;
 B->DamageGrace=2;D->TickGunmen(1000);CHECK_GD(D->GunmanDelay==60&&!D->SpawnGunman(),"Grace permitted shooter");B->DamageGrace=0;
 D->TickGunmen(59);CHECK_GD(D->GunmenSpawned==0&&D->GunmanDelay==1,"First quiet interval was not preserved");
 for(int Attempt=0;Attempt<40&&!D->ActiveGunman.IsValid();Attempt++)D->TickGunmen(76);
 CHECK_GD(D->ActiveGunman.IsValid()&&D->GunmenSpawned==1,"No navigable shooter placement found");auto* Gun=D->ActiveGunman.Get();
 const float Distance=FVector::Dist2D(Gun->GetActorLocation(),B->GetActorLocation());CHECK_GD(Distance>=1200&&Distance<=2000,"Spawn distance invalid");
 FVector Eye;FRotator View;UGameplayStatics::GetPlayerController(W,0)->GetPlayerViewPoint(Eye,View);CHECK_GD(FVector::DotProduct((Gun->GetActorLocation()-Eye).GetSafeNormal2D(),View.Vector().GetSafeNormal2D())<=-.15f,"Shooter popped into view");
 CHECK_GD(Gun->Cooldown==3&&Gun->GetLifeSpan()>34&&D->GunmanDelay>=150&&D->GunmanDelay<=210,"Arrival grace or encounter interval invalid");
 CHECK_GD(!D->SpawnGunman()&&D->GunmenSpawned==1,"Concurrency cap failed");
 Gun->SetActorLocation(B->GetActorLocation()+FVector(5000,0,0));D->TickGunmen(0);CHECK_GD(!D->ActiveGunman.IsValid()&&D->GunmanDelay>=90,"Escaped encounter did not retire");
 B->RiderHealth=0;D->TickGunmen(1000);CHECK_GD(!D->SpawnGunman()&&D->GunmenSpawned==1,"Dead player acquired new encounter");
 End(true,TEXT("Search, practice, countdown and grace protected; real nav spawn behind view; arrival grace, rarity cooldown, single-shooter cap and escape cleanup verified"));
#undef CHECK_GD
#endif
}
