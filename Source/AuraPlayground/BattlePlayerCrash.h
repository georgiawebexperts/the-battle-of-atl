#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattlePlayerRecoveryBlend.h"
#include "BattlePlayerCrash.generated.h"
class ABattleBike;class ABattleFallenBike;class USkeletalMeshComponent;class UPoseableMeshComponent;class ACameraActor;
UCLASS()
class AURAPLAYGROUND_API ABattlePlayerCrash : public AActor {
 GENERATED_BODY()
public:
 ABattlePlayerCrash();
 bool Start(ABattleBike* Source,const FVector& Velocity);
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPoseableMeshComponent* GetRecoveryPose() const;
 float GetRecoveryTime() const;
 bool PrepareRemount(class ABattleRider* Person);
 UPROPERTY() TObjectPtr<ABattleBike> Bike;
 UPROPERTY() TObjectPtr<ABattleFallenBike> Fallen;
 UPROPERTY() TObjectPtr<USkeletalMeshComponent> Physics;
 UPROPERTY() TObjectPtr<UPoseableMeshComponent> Display;
 UPROPERTY() TObjectPtr<ACameraActor> Camera;
 bool bRecovered=false;
private:
 void MirrorPose();
 bool FinishRecovery(float Dt);
 FVector ExitTarget;
 bool bExitReposition=false;
 float NextExitSearch=0;
 FBattlePlayerRecoveryBlend Recovery;
 float Clock=0,Settled=0;
 FName HipBone=TEXT("Hips");
 bool bGettingUp=false;
};
