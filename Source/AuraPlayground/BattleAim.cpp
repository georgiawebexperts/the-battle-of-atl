#include "BattleAim.h"
#include "Misc/ConfigCacheIni.h"

namespace BattleAim {
int32 Step(){
 int32 Value=3;
 GConfig->GetInt(TEXT("BattleControls"),TEXT("AimSensitivityStep"),Value,GGameUserSettingsIni);
 return FMath::Clamp(Value,0,StepCount-1);
}
float Scale(){return Percentages[Step()]/100.f;}
FString Label(){return FString::Printf(TEXT("AIM SENSITIVITY: %d%%  /  [ ]"),Percentages[Step()]);}
void Cycle(int32 Delta){
 const int32 Next=FMath::Clamp(Step()+Delta,0,StepCount-1);
 GConfig->SetInt(TEXT("BattleControls"),TEXT("AimSensitivityStep"),Next,GGameUserSettingsIni);
 GConfig->Flush(false,GGameUserSettingsIni);
 UE_LOG(LogTemp,Display,TEXT("BattleAim: step=%d percent=%d"),Next,Percentages[Next]);
}
}
