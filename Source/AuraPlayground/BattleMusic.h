#pragma once
class APlayerController;
namespace BattleMusic {
 void Initialize(APlayerController* PC);
 void Toggle(APlayerController* PC);
 bool Enabled();
 int Selection();
 void TickAudit(APlayerController* PC,float Dt);
}
