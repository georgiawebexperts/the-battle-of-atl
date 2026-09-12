#include "BattleZombie.h"
#include "BattleGunman.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

namespace {
ABattleBike* RiderBike(APawn* Pawn){
 if(auto* Bike=Cast<ABattleBike>(Pawn))return Bike;
 if(auto* Foot=Cast<ABattleRider>(Pawn))return Foot->ParkedBike;
 return nullptr;
}
bool EncounterAllowed(ABattleEnemyDirector* Director){
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(Director));
 auto* Bike=RiderBike(UGameplayStatics::GetPlayerPawn(Director,0));
 return Mode&&Bike&&!Director->bFreezeSpawns&&!Mode->bTutorialActive&&Mode->StartCountdown<=0&&!Mode->bRunEnded&&!UGameplayStatics::IsGamePaused(Director)&&Bike->RiderHealth>0&&Bike->RespawnRemaining<=0&&Bike->DamageGrace<=0&&((Mode->Quest&&Mode->Quest->bCollected)||Mode->Trouble>=3);
}
}
void ABattleEnemyDirector::TickGunmen(float Dt){
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
 auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 auto* Bike=RiderBike(Pawn);
 if(ActiveGunman.IsValid()&&(!Pawn||!Mode||Mode->bRunEnded||Mode->bTutorialActive||!Bike||Bike->RiderHealth<=0||Bike->RespawnRemaining>0||FVector::Dist2D(Pawn->GetActorLocation(),ActiveGunman->GetActorLocation())>4500)){
  ActiveGunman->Destroy();ActiveGunman.Reset();GunmanDelay=FMath::Max(GunmanDelay,90.f);
 }
 if(!EncounterAllowed(this))return;
 GunmanDelay=FMath::Max(0.f,GunmanDelay-Dt);
 if(ActiveGunman.IsValid()||GunmanDelay>0)return;
 // Rare by default; firing at people increases the chance, never the concurrency cap.
 GunmanDelay=FMath::FRandRange(45.f,75.f);
 if(FMath::FRand()<FMath::Clamp(.35f+Mode->Trouble*.025f,.35f,.65f)&&SpawnGunman())GunmanDelay=FMath::FRandRange(150.f,210.f);
}
bool ABattleEnemyDirector::SpawnGunman(){
 if(!EncounterAllowed(this)||ActiveGunman.IsValid())return false;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);
 auto* PC=UGameplayStatics::GetPlayerController(this,0);
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
 if(!Pawn||!PC||!Nav)return false;
 FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
 for(int Attempt=0;Attempt<24;Attempt++){
  FNavLocation Point;if(!Nav->GetRandomReachablePointInRadius(Pawn->GetActorLocation(),2000,Point))continue;
  const FVector Position=Point.Location+FVector(0,0,90);
  const float Distance=FVector::Dist2D(Position,Pawn->GetActorLocation());if(Distance<1200||Distance>2000)continue;
  // Keep the arrival behind the current view so it cannot pop into the crosshair.
  if(FVector::DotProduct((Position-Eye).GetSafeNormal2D(),View.Vector().GetSafeNormal2D())>-.15f)continue;
  bool Wet=false;for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Position)){Wet=true;break;}if(Wet)continue;
  FCollisionQueryParams Q(SCENE_QUERY_STAT(GunmanSpawn),false,Pawn);
  if(GetWorld()->OverlapBlockingTestByChannel(Position,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(35,88),Q))continue;
  const FRotator Facing(0,(Pawn->GetActorLocation()-Position).Rotation().Yaw,0);
  FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
  if(auto* Gun=GetWorld()->SpawnActor<ABattleGunman>(Position,Facing,Params)){
   Gun->Cooldown=3;Gun->SetLifeSpan(35);ActiveGunman=Gun;GunmenSpawned++;return true;
  }
 }
 return false;
}
