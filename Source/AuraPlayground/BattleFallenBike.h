#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleFallenBike.generated.h"
class USceneComponent;class UBoxComponent;class UStaticMeshComponent;class ABattleBike;
UCLASS()
class AURAPLAYGROUND_API ABattleFallenBike : public AActor {
 GENERATED_BODY()
public:
 ABattleFallenBike();
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void RestoreSourceVisibility();
 bool InitializeFrom(ABattleBike* Bike,const FVector& Velocity);
 UPROPERTY() TObjectPtr<UBoxComponent> Frame;
 int32 PartCount=0;
private:
 TMap<TWeakObjectPtr<UStaticMeshComponent>,bool> SourceVisibility;
 struct FLightAttachment{TWeakObjectPtr<USceneComponent> Light,Parent;FTransform Relative;FName Socket;};
 TArray<FLightAttachment> LightAttachments;
 bool bInitialized=false;
};
