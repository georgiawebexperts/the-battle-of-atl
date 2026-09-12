#pragma once
#include "CoreMinimal.h"
class AActor;class ACameraActor;
// Follow the rider, with a swept camera probe so walls can shorten the boom.
void UpdateBattleCrashCamera(ACameraActor* Camera,AActor* RiderOwner,AActor* FallenBike,const FVector& Focus,float Dt);
