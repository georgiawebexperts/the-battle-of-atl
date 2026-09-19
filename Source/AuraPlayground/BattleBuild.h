#pragma once
#include "CoreMinimal.h"

namespace BattleBuild {
// 151: the controller. A gamepad could not play this game at all before it -
// there were no gamepad mappings in DefaultInput.ini, only dead-zone entries -
// and BattleGamepadAudit now proves the pad pedals, steers by angle, brakes,
// fires, hops, shifts, boosts, sounds the horn and walks the rider on foot.
inline constexpr TCHAR Label[] = TEXT("BUILD 151");
inline constexpr TCHAR Version[] = TEXT("VERSION 1");
inline constexpr TCHAR VersionedLabel[] = TEXT("VERSION 1  •  BUILD 151");
}
