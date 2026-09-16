#pragma once
#include "CoreMinimal.h"
FName BattleDetailedBone(FName Name,bool Detailed);

// Normal player presentation; explicit legacy option remains for diagnostics.
bool BattleUseDetailedRider();
int32 BattleRiderStyle();
void BattleSetRiderStyle(int32 Style);
FString BattleRiderStyleName(int32 Style=-1);
