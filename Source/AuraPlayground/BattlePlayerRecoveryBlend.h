#pragma once
#include "CoreMinimal.h"
class USkeletalMeshComponent;class UPoseableMeshComponent;class UAnimSequence;class ABattleBike;
// Isolated review state; production ownership and interruption handling remain separate.
struct FBattlePlayerRecoveryBlend {
 TWeakObjectPtr<UPoseableMeshComponent> Pose;
 TWeakObjectPtr<UAnimSequence> Clip;
 TArray<FTransform> LandedLocal;
 float Clock=0,TransferError=0,FloorZ=0;
 int32 Choice=-1;
 bool Begin(ABattleBike* Bike,USkeletalMeshComponent* Physics);
 bool Tick(float Dt);
};
