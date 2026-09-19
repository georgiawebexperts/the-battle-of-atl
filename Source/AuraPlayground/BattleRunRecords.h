#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BattleRunRecords.generated.h"
UCLASS()
class AURAPLAYGROUND_API UBattleRunRecords:public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY(SaveGame) TMap<FName,float> BestElapsed;
 UPROPERTY(SaveGame) TMap<FName,int32> WinCounts;
};
namespace BattleRecords {
 FString Slot();
 float Best(FName Difficulty);
 bool Record(FName Difficulty,float Elapsed);
 int32 Wins(FName Difficulty);
 bool AddWin(FName Difficulty);
 FString Format(float Seconds);
}
