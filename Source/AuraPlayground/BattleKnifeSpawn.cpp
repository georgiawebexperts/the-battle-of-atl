#include "BattleKnife.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattleQuest.h"
#include "BattleZombie.h"
#include "NavigationSystem.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
void ABattleLabMode::TickKnife(float Dt){
 if(bTutorialActive||bRunEnded||StartCountdown>0||UGameplayStatics::IsGamePaused(this))return;
 auto* M=Cast<ABattleParkMode>(this);auto* P=UGameplayStatics::GetPlayerPawn(this,0);if(!M||!P||(M->Enemies&&M->Enemies->bFreezeSpawns))return;
 auto* B=Cast<ABattleBike>(P);if(auto* Foot=Cast<ABattleRider>(P)){if(Foot->bSwimming)return;B=Foot->ParkedBike;}
 if(!B||B->RiderHealth<=0||B->RespawnRemaining>0)return;
 KnifeDelay-=Dt;if(KnifeDelay>0)return;KnifeDelay=FMath::FRandRange(90.f,150.f);
 for(TActorIterator<ABattleKnife> It(GetWorld());It;++It)if(!It->bDead)return;
 if(FMath::FRand()>=ABattleKnife::SpawnChance(M->Quest&&M->Quest->bCollected,Trouble))return;
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());if(!Nav)return;
 FVector Eye;FRotator View;if(auto* PC=Cast<APlayerController>(P->GetController()))PC->GetPlayerViewPoint(Eye,View);else return;
 for(int I=0;I<20;I++){
  FNavLocation Point;if(!Nav->GetRandomReachablePointInRadius(P->GetActorLocation(),2600,Point)||FVector::Dist2D(Point.Location,P->GetActorLocation())<1500)continue;
  const FVector Spot=Point.Location+FVector(0,0,90);bool Wet=false;for(TActorIterator<APiedmontWaterHazard> It(GetWorld());It;++It)if(It->ContainsBike(Spot)){Wet=true;break;}if(Wet)continue;
  FHitResult H;FCollisionQueryParams Q(SCENE_QUERY_STAT(KnifeSpawn),false,P);
  if(FVector::DotProduct((Spot-Eye).GetSafeNormal(),View.Vector())>.15f&&!GetWorld()->LineTraceSingleByChannel(H,Eye,Spot,ECC_Visibility,Q))continue;
  FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
  if(GetWorld()->SpawnActor<ABattleKnife>(Spot,(P->GetActorLocation()-Spot).Rotation(),Params)){KnifeSpawned++;return;}
 }
}
bool ABattleBike::ApplyKnifeStab(bool Lethal){
 const auto* M=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));
 if(!M||M->bTutorialActive||M->bRunEnded||M->StartCountdown>0||UGameplayStatics::IsGamePaused(this)||RiderHealth<=0||RespawnRemaining>0||DamageGrace>0||StunRemaining>0||Ride->Recovery>0)return false;
 if(Lethal)return ApplyRiderDamage(1000)>0;
 const float Damage=FMath::Min(25.f,RiderHealth-1);if(Damage>0&&ApplyRiderDamage(Damage)<=0)return false;
 if(!bParked){Ride->Wipeout(TEXT("Knife attack"));if(!bCrashActive)Dismount();}StunLabel=TEXT("STABBED");StunRemaining=.9f;HurtCooldown=5;Ride->BoostRemaining=0;Ride->Speed=Ride->Pedal=Ride->Steer=0;Ride->StopMovementImmediately();UpdateStun(0);RideImpact(.8f);return true;
}
