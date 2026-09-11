#include "BattleZombie.h"
#include "BattlePickup.h"
#include "Engine/World.h"
void ABattleZombie::DropWeapon(){
 if(DroppedWeapon||FMath::FRand()>=FMath::Clamp(WeaponDropChance,0.f,1.f))return;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(EnemyDrop),false,this);FHitResult Floor;
 if(!GetWorld()->LineTraceSingleByChannel(Floor,GetActorLocation()+FVector(0,0,30),GetActorLocation()-FVector(0,0,220),ECC_Visibility,Q)||Floor.ImpactNormal.Z<.6f)return;
 const FVector Spot=Floor.ImpactPoint+FVector(0,0,45);
 if(GetWorld()->OverlapBlockingTestByChannel(Spot,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeSphere(20),Q))return;
 if(auto* Drop=GetWorld()->SpawnActorDeferred<ABattleWeaponCrate>(ABattleWeaponCrate::StaticClass(),FTransform(Spot),nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn)){Drop->WeaponSlot=1+FMath::RandHelper(4);Drop->FinishSpawning(FTransform(Spot));Drop->SetLifeSpan(60);DroppedWeapon=Drop;}
}
