#include "BattleZombie.h"
#include "BattleGunman.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleKnife.h"
#include "BattlePolice.h"
#include "PiedmontPedestrian.h"
#include "BattleCheckpoints.h"
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
void ABattleEnemyDirector::TickMurderK(float Dt){
 if(bMurderKActivated||!EncounterAllowed(this))return;
 auto* Pawn=UGameplayStatics::GetPlayerPawn(this,0);auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));
 if(!Pawn||!Mode)return;
 const auto& A=BattleCheckpoints::Anchors[0];const FVector Anchor(A.X,A.Y,A.Z);
 if(FVector::Dist2D(Pawn->GetActorLocation(),Anchor)>3000||FMath::Abs(Pawn->GetActorLocation().Z-Anchor.Z)>600)return;
 bMurderKActivated=true;
 const FQuat Frame=FRotator(0,A.Yaw,0).Quaternion();auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
 auto Grounded=[&](FVector Local){FVector Position=Anchor+Frame.RotateVector(Local);FNavLocation OnNav;if(Nav&&Nav->ProjectPointToNavigation(Position,OnNav,FVector(300,300,500)))Position=OnNav.Location;return Position+FVector(0,0,90);};
 // Murder K is a dense, hostile landmark even on Easy. Two pairs are already
 // fighting; the rest break toward Ellison when he rides into the plaza.
 TArray<ABattleZombie*> Brawlers;
 // Keep the middle of the widened plaza rideable. The crowd occupies both
 // edges and converges after activation instead of spawning as a solid wall.
 const FVector PunkOffsets[]={FVector(-1000,850,0),FVector(-720,-850,0),FVector(-300,920,0),FVector(40,-900,0),FVector(470,840,0),FVector(760,-920,0),FVector(1080,850,0),FVector(430,-1180,0),FVector(-470,-1160,0),FVector(-1450,-760,0),FVector(1480,780,0),FVector(830,1200,0)};
 // Kroger is the hardest stop on the route on every difficulty. Elliott rode
 // through and called it too easy, so the plaza is now packed and three brawls
 // are already going before the rider arrives.
 const int32 PunkCount=Mode->DifficultyName==TEXT("Easy")?7:(Mode->DifficultyName==TEXT("Medium")?9:11);
 for(int32 I=0;I<PunkCount;I++){
  const FVector Position=Grounded(PunkOffsets[I]);FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
  if(auto* Punk=GetWorld()->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),FTransform((Pawn->GetActorLocation()-Position).Rotation(),Position),this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)){
   Punk->VisualStyle=1;Punk->MoveSpeed=Mode->Difficulty.ZombieSpeed*1.08f;Punk->AttackDamage=Mode->Difficulty.ZombieDamage;Punk->WarningSeconds=Mode->Difficulty.ZombieWarningSeconds;Punk->Emergence=0;Punk->SetLifeSpan(65);Punk->FinishSpawning(FTransform((Pawn->GetActorLocation()-Position).Rotation(),Position));MurderKPunksSpawned++;if(I<6)Brawlers.Add(Punk);
  }
 }
 for(int32 I=0;I+1<Brawlers.Num();I+=2){Brawlers[I]->bMurderKBrawler=Brawlers[I+1]->bMurderKBrawler=true;Brawlers[I]->BrawlPartner=Brawlers[I+1];Brawlers[I+1]->BrawlPartner=Brawlers[I];MurderKFightSpots++;}
 const FVector BumOffsets[]={FVector(-1250,1180,0),FVector(1120,-1200,0),FVector(140,1280,0)};
 for(const FVector Offset:BumOffsets){const FVector Position=Grounded(Offset);auto* Bum=GetWorld()->SpawnActorDeferred<APiedmontPedestrian>(APiedmontPedestrian::StaticClass(),FTransform(FRotator(0,A.Yaw,0),Position),this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);if(Bum){Bum->bAmbientSleeper=true;Bum->AmbientWakeChance=.35f;Bum->Tags.Add(TEXT("MurderKBum"));Bum->FinishSpawning(FTransform(FRotator(0,A.Yaw,0),Position));Bum->SetLifeSpan(90);MurderKBumsSpawned++;}}
 const FVector CopOffsets[]={FVector(-1450,1380,0),FVector(1380,-1380,0),FVector(0,1720,0),FVector(-1720,-140,0)};const int32 CopCount=Mode->DifficultyName==TEXT("Easy")?2:4;
 for(int32 I=0;I<CopCount;I++){const FVector Position=Grounded(CopOffsets[I]);auto* Cop=GetWorld()->SpawnActorDeferred<ABattlePolice>(ABattlePolice::StaticClass(),FTransform(FRotator(0,A.Yaw-90,0),Position),this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);if(Cop){Cop->bAmbientMurderK=true;Cop->FinishSpawning(FTransform(FRotator(0,A.Yaw-90,0),Position));MurderKAmbientPolice++;}}
 // The store is never unguarded: the difficulty table sets the ceiling, this
 // sets the floor, so Kroger stays a violent stretch even on Easy.
 const FVector GunOffsets[]={FVector(-900,1150,0),FVector(80,-1220,0),FVector(980,1120,0)};
 const int32 KrogerShooters=FMath::Clamp(FMath::Max(Mode->Difficulty.KrogerShooters,Mode->DifficultyName==TEXT("Easy")?1:(Mode->DifficultyName==TEXT("Medium")?2:3)),0,3);
 for(int32 I=0;I<KrogerShooters;I++){
  FVector Position=Anchor+Frame.RotateVector(GunOffsets[I]);FNavLocation OnNav;if(Nav&&Nav->ProjectPointToNavigation(Position,OnNav,FVector(350,350,500)))Position=OnNav.Location;Position.Z+=90;
  FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
  if(auto* Gun=GetWorld()->SpawnActor<ABattleGunman>(Position,(Pawn->GetActorLocation()-Position).Rotation(),Params)){Gun->Cooldown=2.5f+I*.55f;Gun->SetLifeSpan(50);MurderKGunmen.Add(Gun);MurderKGunmenSpawned++;GunmenSpawned++;}
 }
 if(Mode->Difficulty.KnifeBehavior>0){
  FVector Position=Anchor+Frame.RotateVector(FVector(-420,-980,0));FNavLocation OnNav;if(Nav&&Nav->ProjectPointToNavigation(Position,OnNav,FVector(350,350,500)))Position=OnNav.Location;Position.Z+=90;
  FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
  if(auto* Knife=GetWorld()->SpawnActor<ABattleKnife>(Position,(Pawn->GetActorLocation()-Position).Rotation(),Params)){Knife->GetCharacterMovement()->MaxWalkSpeed=Mode->Difficulty.KnifeSpeed;Knife->bSingleLunge=Mode->Difficulty.KnifeBehavior==1;Knife->SetLifeSpan(50);bMurderKKnifeSpawned=true;}
 }
 // The punk wall: six punks in two staggered rows spanning the ride path
 // perpendicular to the ride line ahead of the plaza. The punks are the
 // wall, so the only way past is to shoot through. Own flag, so it can
 // never double-spawn.
 if(!Tags.Contains(TEXT("MurderKPunkWallActivated"))){
  Tags.Add(TEXT("MurderKPunkWallActivated"));
  Mode->PushHint(TEXT("murderkpunkwall"),TEXT("PUNK WALL AHEAD - SHOOT THROUGH - EVERY PUNK DROPS GEAR"),6.f);
  const FVector WallOffsets[]={FVector(-950,-350,0),FVector(-950,0,0),FVector(-950,350,0),FVector(-700,-150,0),FVector(-700,250,0),FVector(-700,550,0)};
  for(const FVector Offset:WallOffsets){
   const FVector Position=Grounded(Offset);FActorSpawnParameters Params;Params.Owner=this;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
   if(auto* Punk=GetWorld()->SpawnActorDeferred<ABattleZombie>(ABattleZombie::StaticClass(),FTransform((Pawn->GetActorLocation()-Position).Rotation(),Position),this,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)){
    Punk->VisualStyle=1;Punk->MoveSpeed=Mode->Difficulty.ZombieSpeed*1.08f;Punk->AttackDamage=Mode->Difficulty.ZombieDamage;Punk->WarningSeconds=Mode->Difficulty.ZombieWarningSeconds;Punk->Emergence=0;Punk->Health=200;Punk->WeaponDropChance=1.f;Punk->Tags.Add(TEXT("BattlePunkWall"));Punk->SetLifeSpan(65);Punk->FinishSpawning(FTransform((Pawn->GetActorLocation()-Position).Rotation(),Position));
   }
  }
 }
}
