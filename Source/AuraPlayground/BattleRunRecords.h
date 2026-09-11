#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BattleRunRecords.generated.h"
UCLASS()
class AURAPLAYGROUND_API UBattleRunRecords:public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY(SaveGame) TMap<FName,float> BestElapsed;
};
namespace BattleRecords {
 FString Slot();
 float Best(FName Difficulty);
 bool Record(FName Difficulty,float Elapsed);
 FString Format(float Seconds);
}
