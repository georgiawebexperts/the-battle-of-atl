#pragma once
#include "CoreMinimal.h"

namespace BattleBuild {
// 152: this side shipped 151 (the controller) and Windows shipped their own 149
// (the Ghost Rider), so the merge takes the next number. This build carries both:
// a gamepad that can play the game, and the ghost of your best run riding it with
// you once you have five wins on a difficulty.
inline constexpr TCHAR Label[] = TEXT("BUILD 152");
inline constexpr TCHAR Version[] = TEXT("VERSION 1");
inline constexpr TCHAR VersionedLabel[] = TEXT("VERSION 1  •  BUILD 152");
}
