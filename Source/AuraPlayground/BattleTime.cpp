#include "BattleBike.h"
#include "Kismet/GameplayStatics.h"
bool ABattleLabMode::AdjustRunTime(float Seconds,const FString& Reason){
 if(bRunEnded||StartCountdown>0||UGameplayStatics::IsGamePaused(this)||!FMath::IsFinite(Seconds)||Seconds==0)return false;
 TimeRemaining=FMath::Max(0.f,TimeRemaining+Seconds);
 LastTimeDelta=Seconds;TimeNotice=Reason;TimeNoticeRemaining=2.5f;
 if(TimeRemaining<=0)bRunEnded=true;
 UE_LOG(LogTemp,Display,TEXT("BattleTime: delta=%.0f reason=%s remaining=%.2f"),Seconds,*Reason,TimeRemaining);
 return true;
}
void ABattleLabMode::RecordPlayerShotHit(AActor* Victim){
 if(!IsValid(Victim))return;
 if(Victim->ActorHasTag(TEXT("PiedmontTraffic")))RecordAssault(Victim);
 if(Victim->ActorHasTag(TEXT("BattleZombie")))AdjustRunTime(10,TEXT("ZOMBIE HIT"));
 else if(Victim->ActorHasTag(TEXT("BattlePolice")))AdjustRunTime(-60,TEXT("POLICE HIT"));
 else if(Victim->ActorHasTag(TEXT("PiedmontTraffic")))AdjustRunTime(-10,TEXT("PEDESTRIAN HIT"));
 else if(Victim->ActorHasTag(TEXT("BattlePet"))||Victim->ActorHasTag(TEXT("BattleDuck")))AdjustRunTime(-10,TEXT("ANIMAL HIT"));
}
