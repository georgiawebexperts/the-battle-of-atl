#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PiedmontTrafficDirector.generated.h"
class APiedmontPedestrian;
UCLASS()
class AURAPLAYGROUND_API APiedmontTrafficDirector : public AActor {
 GENERATED_BODY()
public:
 APiedmontTrafficDirector();
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 DesiredPopulation=24;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float JoggerShare=.3f;
 UPROPERTY(BlueprintReadOnly) int32 LivePopulation=0;
 UPROPERTY(BlueprintReadOnly) int32 TotalSpawned=0;
 UFUNCTION(BlueprintCallable) APiedmontPedestrian* SpawnVisitorForValidation(FVector Location,bool Jogger);
private:
 APiedmontPedestrian* SpawnVisitor(FVector Location,bool Jogger);
 TArray<FVector> CandidatePoints;
 TArray<TWeakObjectPtr<APiedmontPedestrian>> Visitors;
};
