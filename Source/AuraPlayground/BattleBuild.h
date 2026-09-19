#pragma once
#include "CoreMinimal.h"

namespace BattleBuild {
// 150, not 149: this Mac already shared a 149 (the bear back on its authored
// material) and Windows is packaging its own 149 (round 15, the ghost rider).
// Three builds must not carry two numbers, so the merge takes the next one.
// This build carries Windows round 14 - the Kroger wall punks at 200 HP with a
// guaranteed weapon drop each - on top of the Mac's 149.
inline constexpr TCHAR Label[] = TEXT("BUILD 150");
inline constexpr TCHAR Version[] = TEXT("VERSION 1");
inline constexpr TCHAR VersionedLabel[] = TEXT("VERSION 1  •  BUILD 150");
}
