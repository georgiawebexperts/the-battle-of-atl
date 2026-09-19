#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BattleRunRecords.generated.h"
USTRUCT()
struct FBattleGhostRoute{
 GENERATED_BODY()
 UPROPERTY(SaveGame) TArray<FVector> Points;
};
UCLASS()
class AURAPLAYGROUND_API UBattleRunRecords:public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY(SaveGame) TMap<FName,float> BestElapsed;
 UPROPERTY(SaveGame) TMap<FName,int32> WinCounts;
 UPROPERTY(SaveGame) TMap<FName,FBattleGhostRoute> BestRoute;
};
namespace BattleRecords {
 FString Slot();
 float Best(FName Difficulty);
 bool Record(FName Difficulty,float Elapsed,const TArray<FVector>& Route=TArray<FVector>());
 TArray<FVector> BestRoute(FName Difficulty);
 bool GhostUnlocked(FName Difficulty);
 int32 Wins(FName Difficulty);
 bool AddWin(FName Difficulty);
 FString Format(float Seconds);
}
