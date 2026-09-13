#pragma once
#include "CoreMinimal.h"
class AActor;
class ABattleRider;
class APlayerController;
namespace BattleSpareBikes {
 void SpawnStations(AActor* Context);
 void TickAudit(APlayerController* PC,float Dt);
 void Locations(const UObject* Context,TArray<FVector>& Out);
 AActor* Nearest(const ABattleRider* Person,float Range=240.f);
 bool Mount(ABattleRider* Person,AActor* Spare);
}
