#pragma once
#include "CoreMinimal.h"
#include "BattleInventory.generated.h"
USTRUCT(BlueprintType)
struct AURAPLAYGROUND_API FBattleWeaponState {
 GENERATED_BODY()
 UPROPERTY(BlueprintReadOnly) bool Owned=false;
 UPROPERTY(BlueprintReadOnly) int32 Magazine=0;
 UPROPERTY(BlueprintReadOnly) int32 Reserve=0;
};
namespace BattleWeapons {
 inline constexpr int32 Count=5;
 inline int32 Capacity(int32 Slot){const int32 Values[]={17,6,30,8,30};return Slot>=0&&Slot<Count?Values[Slot]:0;}
 inline int32 ReserveLimit(int32 Slot){return Slot==0?60:Slot==1?60:Slot==2?300:Slot==4?120:32;}
 inline float ReloadSeconds(int32 Slot){return Slot==4?2.4f:Slot==1?2.2f:Slot==2?1.8f:1.5f;}
 inline const TCHAR* Name(int32 Slot){const TCHAR* Values[]={TEXT("PISTOL"),TEXT("SHOTGUN"),TEXT("SMG"),TEXT("FRISBEE"),TEXT("RIFLE")};return Slot>=0&&Slot<Count?Values[Slot]:TEXT("UNKNOWN");}
}
