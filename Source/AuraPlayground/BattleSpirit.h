#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleSpirit.generated.h"
class ABattleBike;
class UStaticMeshComponent;
class UPointLightComponent;
class UTexture2D;
class UBillboardComponent;
enum class EBattleSpiritState : uint8 { Untried, Absent, Appearing, Active, Fading, Resolved };

// One actor per full run. Checkpoint recovery must never recreate or reset it.
UCLASS()
class AURAPLAYGROUND_API ABattleSpirit : public AActor {
 GENERATED_BODY()
public:
 ABattleSpirit();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 bool TryApproach(float EligibilityRoll);
 bool TryCatch();
 void Cancel();
 void AdvanceEncounter(float Dt);
 static void CancelForRider(const UObject* Context);
 EBattleSpiritState State=EBattleSpiritState::Untried;
 float Age=0,Fade=0;
 int32 Rewards=0;
 bool bPresentationReady=true;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> FigureParts;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Wisps;
 // The two cold eyes on the 3D body. Separate from FigureParts because they
 // belong to the imported mesh's frame, not the primitive figure's, and they
 // have to be visible when the body is (the figure's own eyes are not).
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BodyEyes;
 /** Recent body positions, newest first: the drifting trail the note asks for. */
 UPROPERTY() TArray<FVector> TrailPoints;
 float TrailClock=0;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> MoonGlow;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> SpiritCard;
 /** The 3D body. Replaces the flat card as the presentation the player sees. */
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> BearBody;
 UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> BearBodyMaterial;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UBillboardComponent> SpiritBillboard;
 FVector ApproachPoint=FVector::ZeroVector;
 TArray<FVector> ChaseRoute;
 float RouteDistance=0;
private:
 bool IsLiveRun() const;
 ABattleBike* MountedRider() const;
};
