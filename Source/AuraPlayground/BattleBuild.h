#pragma once
#include "CoreMinimal.h"

namespace BattleBuild {
// 149, not 148: the Mac side had already packaged and shared 148 before this merge,
// and Windows is packaging its own 149 with the Ghost Rider unlock.
// Two different builds must not carry one number.
inline constexpr TCHAR Label[] = TEXT("BUILD 149");
inline constexpr TCHAR Version[] = TEXT("VERSION 1");
inline constexpr TCHAR VersionedLabel[] = TEXT("VERSION 1  •  BUILD 149");
}
