#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleMurderK.generated.h"
class UStaticMeshComponent;
class UTextRenderComponent;
class UAudioComponent;
class APiedmontPedestrian;

// Fictional trail-side grocery landmark and plaza at the real 725 Ponce checkpoint.
// The plaza is the loudest stretch on the ride: a DJ and a dance party on one
// side of the apron, a riot knot on the other, bodies on the pavement and
// brawls in between. It is deliberately the most chaotic place on the course.
UCLASS()
class AURAPLAYGROUND_API ABattleMurderK : public AActor {
 GENERATED_BODY()
public:
 ABattleMurderK();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> StoreSign;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> GraffitiSign;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> PartyLabel;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> StoreMass;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> OfficeTower;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> GlassWing;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> TrailApron;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Speaker;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> SpeakerCone;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UAudioComponent> PartyMusic;
 /** Live population of the plaza, for the audit and for review frames. */
 UPROPERTY(BlueprintReadOnly) int32 Dancers=0,Rioters=0,BodiesDown=0,Brawls=0;
 /** The rotating riot chant, shown by the HUD exactly like the Krog crash shouts. */
 UPROPERTY(BlueprintReadOnly) FString ShoutText;
 UPROPERTY(BlueprintReadOnly) float ShoutRemaining=0;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BrickParts;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> DarkParts;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> RedParts;
UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> GlassParts;
UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> ConcreteParts;
private:
 /** Drops a landmark-local point onto whatever ground is there now. */
 FVector GroundAt(const FVector& Local) const;
 void TopUp();
 void PollCrowd(float Dt);
 bool bPlazaLive=true;
 float ShoutClock=0,TopUpClock=0;
 int32 ShoutIndex=0;
 TArray<TWeakObjectPtr<APiedmontPedestrian>> DancerList;
 TArray<TWeakObjectPtr<APiedmontPedestrian>> RiotList;
 TArray<TWeakObjectPtr<APiedmontPedestrian>> BodyList;
};
