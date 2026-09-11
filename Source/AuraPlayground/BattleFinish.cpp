#include "BattleBike.h"
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
bool BattleRecords::Record(FName Difficulty,float Elapsed){if(!FMath::IsFinite(Elapsed)||Elapsed<=0)return false;auto* R=Cast<UBattleRunRecords>(UGameplayStatics::LoadGameFromSlot(Slot(),0));if(!R)R=Cast<UBattleRunRecords>(UGameplayStatics::CreateSaveGameObject(UBattleRunRecords::StaticClass()));const float Old=R->BestElapsed.FindRef(Difficulty);if(Old>0&&Old<=Elapsed)return true;R->BestElapsed.Add(Difficulty,Elapsed);return UGameplayStatics::SaveGameToSlot(R,Slot(),0);}
FString BattleRecords::Format(float Seconds){const int T=FMath::Max(0,FMath::FloorToInt(Seconds));return FString::Printf(TEXT("%d:%02d"),T/60,T%60);}
bool ABattleParkMode::CompleteRun(ABattleBike* Bike){
 if(!Bike||Bike->RiderHealth<=0||bRunEnded||StartCountdown>0||TimeRemaining<=0||UGameplayStatics::IsGamePaused(this))return false;
 bWon=true;bRunEnded=true;FinishKills=Bike->EnemyKills;FinishWipeouts=Bike->Ride->Wipeouts;FinishNearMisses=Bike->NearMisses;
 const float Fraction=TimeRemaining/FMath::Max(1.f,Difficulty.TimeLimitSeconds);FinishGrade=Fraction>=.5f?TEXT("A"):Fraction>=.25f?TEXT("B"):Fraction>=.1f?TEXT("C"):TEXT("D");
 bRecordSaved=BattleRecords::Record(DifficultyName,RunElapsed);Bike->Ride->Speed=Bike->Ride->Pedal=Bike->Ride->Steer=0;Bike->Ride->StopMovementImmediately();
 UE_LOG(LogTemp,Display,TEXT("BattleFinish: won=1 elapsed=%.2f remaining=%.2f saved=%d"),RunElapsed,TimeRemaining,bRecordSaved);return true;
}
