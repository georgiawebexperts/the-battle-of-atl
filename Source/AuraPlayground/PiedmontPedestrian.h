#pragma once
#include "CoreMinimal.h"
#include "PiedmontExplorer.h"
#include "PiedmontPedestrian.generated.h"
class APiedmontBike;
UENUM(BlueprintType)
enum class EPiedmontPedestrianKind : uint8 { Walker, Jogger };

/** Moving, collidable park visitors driven by an AIController on the park navmesh. */
UCLASS()
class AURAPLAYGROUND_API APiedmontPedestrian : public APiedmontExplorer {
 GENERATED_BODY()
public:
 APiedmontPedestrian();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual float TakeDamage(float Amount,const FDamageEvent& Event,AController* Instigator,AActor* Causer) override;
 UPROPERTY(BlueprintReadOnly) EPiedmontPedestrianKind Kind=EPiedmontPedestrianKind::Walker;
 UPROPERTY(BlueprintReadOnly) FVector Destination;
 UPROPERTY(BlueprintReadOnly) bool bHasDestination=false;
 UPROPERTY(BlueprintReadOnly) float PauseRemaining=0;
 UPROPERTY(BlueprintReadOnly) float StumbleRemaining=0;
 UPROPERTY(BlueprintReadOnly) int32 HornReactions=0;
 UPROPERTY(BlueprintReadOnly) int32 BikeContacts=0;
 UPROPERTY(BlueprintReadOnly) int32 CompletedWalks=0;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<APiedmontPedestrian> GroupLeader;
 float GroupSide=1;
 void Configure(EPiedmontPedestrianKind NewKind);
 void HearHorn(APawn* Source);
 void BikeImpact(float Speed,FVector Direction);
 UFUNCTION(BlueprintCallable) bool SetDestinationForValidation(FVector Goal);
protected:
 float YieldCooldown=0;
 virtual bool CanUseWeapon() const override {return false;}
private:
 void InitializeCityAppearance();
 bool MoveTo(FVector Goal);
 void ChooseDestination();
 void YieldTo(APawn* Source,bool Horn);
 float ThinkRemaining=0,YieldRemaining=0;
};
