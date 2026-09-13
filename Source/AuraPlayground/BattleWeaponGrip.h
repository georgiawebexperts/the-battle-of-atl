#pragma once
#include "CoreMinimal.h"
// Wrist placement in the equipped model's local receiver frame (centimetres).
// These are art offsets; the world transform is always driven by the body pose.
namespace BattleWeaponGrip {
inline FVector Right(int32 Slot){return Slot==4?FVector(-12,3,-10):Slot==1?FVector(-39,4,-3):FVector(-3.5f,3,-8);}
inline FVector Left(int32 Slot){return Slot==4?FVector(8,-3,-2):Slot==1?FVector(4,-3,-4):FVector(8.4f,-3,-5);}
}
