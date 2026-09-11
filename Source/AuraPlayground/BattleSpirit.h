#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleSpirit.generated.h"
class ABattleBike;
enum class EBattleSpiritState : uint8 { Untried, Absent, Appearing, Active, Fading, Resolved };

// One actor per full run. Checkpoint recovery must never recreate or reset it.
UCLASS()
class AURAPLAYGROUND_API ABattleSpirit : public AActor {
 GENERATED_BODY()
public:
 ABattleSpirit();
 virtual void Tick(float Dt) override;
 bool TryApproach(float EligibilityRoll);
 bool TryCatch();
 void Cancel();
 void AdvanceEncounter(float Dt);
 static void CancelForRider(const UObject* Context);
 EBattleSpiritState State=EBattleSpiritState::Untried;
 float Age=0,Fade=0;
 int32 Rewards=0;
 // Presentation remains disabled until bear, flowers and rideable route pass visual review.
 bool bPresentationReady=false;
 FVector ApproachPoint=FVector::ZeroVector;
 TArray<FVector> ChaseRoute;
 float RouteDistance=0;
private:
 bool IsLiveRun() const;
 ABattleBike* MountedRider() const;
};
