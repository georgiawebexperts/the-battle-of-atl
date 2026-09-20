#pragma once
#include "CoreMinimal.h"

namespace BattleBuild {
// 153: the playtest build Elliott and the testers get. 151 was the controller and
// 152 merged Windows' Ghost Rider; 153 adds the four asks from the first playtest
// (two minutes off every clock, the hurt yell and red flash, a dismount that faces
// down the route). It shipped as 153 everywhere except this label, which was left
// at 152 by mistake - so the on-screen build tag disagreed with the bundle. This
// is that correction, not a new build: the number stays 153.
inline constexpr TCHAR Label[] = TEXT("BUILD 153");
inline constexpr TCHAR Version[] = TEXT("VERSION 1");
inline constexpr TCHAR VersionedLabel[] = TEXT("VERSION 1  •  BUILD 153");
}
