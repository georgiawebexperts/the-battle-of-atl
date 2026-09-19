#pragma once
#include "CoreMinimal.h"

namespace BattleBuild {
// 148, not 147: this side had already packaged and shared 147 (the bear's
// light-based look) before the merge, and Windows is packaging its own 147 with
// the 200 HP punk wall. Two different builds must not carry one number.
inline constexpr TCHAR Label[] = TEXT("BUILD 148");
inline constexpr TCHAR Version[] = TEXT("VERSION 1");
inline constexpr TCHAR VersionedLabel[] = TEXT("VERSION 1  •  BUILD 148");
}
