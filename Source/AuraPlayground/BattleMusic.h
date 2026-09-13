#pragma once
class APlayerController;
namespace BattleMusic {
 void Initialize(APlayerController* PC);
 void Toggle(APlayerController* PC);
 bool Enabled();
 void TickAudit(APlayerController* PC,float Dt);
}
