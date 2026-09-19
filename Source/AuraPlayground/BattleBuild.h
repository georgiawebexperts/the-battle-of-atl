#pragma once
#include "CoreMinimal.h"

namespace BattleBuild {
// 149, not 148: 148 is the build on the Desktop shortcut and it carries the
// bear's light-based look. This one puts the bear back on its own authored
// material, which the 09:25 review capture shows draws - see the commit that
// made the change for the evidence and for what the "never drawn" reading got
// wrong.
inline constexpr TCHAR Label[] = TEXT("BUILD 149");
inline constexpr TCHAR Version[] = TEXT("VERSION 1");
inline constexpr TCHAR VersionedLabel[] = TEXT("VERSION 1  •  BUILD 149");
}
