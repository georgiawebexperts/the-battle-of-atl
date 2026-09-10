#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "GameFramework/GameModeBase.h"
#include "GlideGame.generated.h"

UCLASS()
class AURAPLAYGROUND_API AGlideBike : public APawn {
 GENERATED_BODY()
public:
 AGlideBike();
 virtual void BeginPlay() override;
 virtual void Tick(float DeltaSeconds) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 float Speed=0,Distance=0; int32 Score=0,Hits=0;
private:
 void ThrottleInput(float V){Throttle=V;} void TurnInput(float V){Turn=V;}
 void BrakeOn(){Braking=true;} void BrakeOff(){Braking=false;} void ResetRide();
 float Throttle=0,Turn=0,Cooldown=0,Elapsed=0;bool Braking=false;
 FVector Start; FRotator StartRotation;
 struct FTraffic {TWeakObjectPtr<AActor> Actor;FVector Origin;float Phase;bool Scooter;};
 TArray<FTraffic> Traffic;
};
UCLASS()
class AURAPLAYGROUND_API AGlideHUD : public AHUD {
 GENERATED_BODY()
public: virtual void DrawHUD() override;
};
UCLASS()
class AURAPLAYGROUND_API AGlideGameMode : public AGameModeBase {
 GENERATED_BODY()
public: AGlideGameMode();
};
