#include "BattleHints.h"
#include "BattleBike.h"

void ABattleLabMode::PushHint(FName Id,const FString& Text,float Seconds,bool bOnce){
 if(bOnce&&HintsSeen.Contains(Id))return;
 HintsSeen.Add(Id);HintText=Text;HintRemaining=Seconds;
}
void ABattleLabMode::TickHints(float Dt){
 HintRemaining=FMath::Max(0.f,HintRemaining-Dt);
}
