#include "BattleBike.h"
#include "BattleHome.h"
#include "BattleRunRecords.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
FString BattleRecords::Slot(){
#if !UE_BUILD_SHIPPING
 TArray<FString> Args;FString(FCommandLine::Get()).ParseIntoArrayWS(Args);
 for(const FString& Arg:Args)if(Arg.StartsWith(TEXT("-Battle"))&&(Arg.EndsWith(TEXT("Audit"))||Arg.EndsWith(TEXT("Review"))))return TEXT("BattleForTheATL_AuditRecords");
#endif
 return TEXT("BattleForTheATL_Records_v1");
}
float BattleRecords::Best(FName Difficulty){auto* R=Cast<UBattleRunRecords>(UGameplayStatics::LoadGameFromSlot(Slot(),0));return R&&R->BestElapsed.Contains(Difficulty)?R->BestElapsed[Difficulty]:0;}
bool BattleRecords::Record(FName Difficulty,float Elapsed,const TArray<FVector>& Route){if(!FMath::IsFinite(Elapsed)||Elapsed<=0)return false;auto* R=Cast<UBattleRunRecords>(UGameplayStatics::LoadGameFromSlot(Slot(),0));if(!R)R=Cast<UBattleRunRecords>(UGameplayStatics::CreateSaveGameObject(UBattleRunRecords::StaticClass()));const float Old=R->BestElapsed.FindRef(Difficulty);if(Old>0&&Old<=Elapsed)return true;R->BestElapsed.Add(Difficulty,Elapsed);if(Route.Num()>1){FBattleGhostRoute G;G.Points=Route;R->BestRoute.Add(Difficulty,G);}return UGameplayStatics::SaveGameToSlot(R,Slot(),0);}
FString BattleRecords::Format(float Seconds){const int T=FMath::Max(0,FMath::FloorToInt(Seconds));return FString::Printf(TEXT("%d:%02d"),T/60,T%60);}
int32 BattleRecords::Wins(FName Difficulty){auto* R=Cast<UBattleRunRecords>(UGameplayStatics::LoadGameFromSlot(Slot(),0));return R&&R->WinCounts.Contains(Difficulty)?R->WinCounts[Difficulty]:0;}
TArray<FVector> BattleRecords::BestRoute(FName Difficulty){auto* R=Cast<UBattleRunRecords>(UGameplayStatics::LoadGameFromSlot(Slot(),0));return R&&R->BestRoute.Contains(Difficulty)?R->BestRoute[Difficulty].Points:TArray<FVector>();}
bool BattleRecords::GhostUnlocked(FName Difficulty){return Wins(Difficulty)>=5&&BestRoute(Difficulty).Num()>1;}
bool BattleRecords::AddWin(FName Difficulty){auto* R=Cast<UBattleRunRecords>(UGameplayStatics::LoadGameFromSlot(Slot(),0));if(!R)R=Cast<UBattleRunRecords>(UGameplayStatics::CreateSaveGameObject(UBattleRunRecords::StaticClass()));if(!R)return false;R->WinCounts.Add(Difficulty,R->WinCounts.FindRef(Difficulty)+1);return UGameplayStatics::SaveGameToSlot(R,Slot(),0);}
bool ABattleParkMode::CompleteRun(ABattleBike* Bike){
 if(!Bike||Bike->RiderHealth<=0||bRunEnded||StartCountdown>0||TimeRemaining<=0||UGameplayStatics::IsGamePaused(this))return false;
 bWon=true;bRunEnded=true;FinishKills=Bike->EnemyKills;FinishWipeouts=Bike->Ride->Wipeouts;FinishNearMisses=Bike->NearMisses;
 const float Fraction=TimeRemaining/FMath::Max(1.f,Difficulty.TimeLimitSeconds);FinishGrade=Fraction>=.5f?TEXT("A"):Fraction>=.25f?TEXT("B"):Fraction>=.1f?TEXT("C"):TEXT("D");
 const float OldBest=BattleRecords::Best(DifficultyName);const bool TimeSaved=BattleRecords::Record(DifficultyName,RunElapsed,RouteSamples);const bool WinSaved=BattleRecords::AddWin(DifficultyName);bRecordSaved=TimeSaved&&WinSaved;bGhostBeaten=bGhostRiding&&TimeSaved&&RunElapsed<OldBest;Bike->Ride->Speed=Bike->Ride->Pedal=Bike->Ride->Steer=0;Bike->Ride->StopMovementImmediately();
 ABattleHome::RefreshBestTimeBoard(DifficultyName,true);
 UE_LOG(LogTemp,Display,TEXT("BattleFinish: won=1 elapsed=%.2f remaining=%.2f saved=%d"),RunElapsed,TimeRemaining,bRecordSaved);return true;
}
