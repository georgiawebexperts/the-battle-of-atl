#pragma once
#include "CoreMinimal.h"

/**
 * Player-facing aim sensitivity. The authored look rate stays the baseline that
 * was tuned in play; this scales it from 25% to 200% and persists the choice, so
 * a player never has to live with someone else's mouse feel.
 */
namespace BattleAim {
inline constexpr int32 Percentages[] = {25,50,75,100,125,150,200};
inline constexpr int32 StepCount = UE_ARRAY_COUNT(Percentages);
int32 Step();
float Scale();
FString Label();
void Cycle(int32 Delta);
}
