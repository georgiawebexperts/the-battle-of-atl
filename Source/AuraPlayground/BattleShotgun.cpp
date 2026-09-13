#include "BattleRider.h"
#include "BattleBike.h"
#include "BattleInventory.h"
#include "Components/StaticMeshComponent.h"

namespace {
constexpr float ShellSeconds=.65f;
float Ease(float T){T=FMath::Clamp(T,0.f,1.f);return T*T*(3-2*T);}
}
void ABattleRider::StartShotgunReload(){
 ShotgunReloadShells=FMath::Min(BattleWeapons::Capacity(1)-Ammo,ParkedBike->Inventory[1].Reserve);
 if(ShotgunReloadShells<=0)return;
 ShotgunInsertedShells=0;ShotgunReloadLead=FMath::Max(.4f,ShotgunPumpRemaining+.25f);
 ShotgunReloadDuration=ShotgunReloadLead+ShotgunReloadShells*ShellSeconds+.35f;
 ReloadRemaining=ShotgunReloadDuration;bShotgunReloading=true;
}
void ABattleRider::TransferShotgunShells(int32 Count){
 if(!ParkedBike||CurrentWeapon!=1)return;
 auto& Item=ParkedBike->Inventory[1];
 while(ShotgunInsertedShells<Count&&Ammo<BattleWeapons::Capacity(1)&&Item.Reserve>0){++Ammo;--Item.Reserve;++ShotgunInsertedShells;}
 SaveWeapon();
}
void ABattleRider::FinishShotgunReload(){
 TransferShotgunShells(ShotgunReloadShells);bShotgunReloading=false;ShotgunReloadBlend=0;
}
void ABattleRider::UpdateShotgunMechanism(float Dt){
 ShotgunPumpRemaining=FMath::Max(0.f,ShotgunPumpRemaining-Dt);
 if(CurrentWeapon!=1||!bWeaponDrawn||!CanUseWeapon()){ShotgunPumpRemaining=0;bShotgunReloading=false;}
 const float PumpTime=.75f-ShotgunPumpRemaining;
 ShotgunPumpTravel=ShotgunPumpRemaining>0?8.f*Ease((PumpTime-.1f)/.22f)*(1-Ease((PumpTime-.43f)/.27f)):0;
 ShotgunReloadHand=FVector(4-ShotgunPumpTravel,-3,-4);ShotgunReloadBlend=0;ShotgunShellPhase=-1;
 if(!bShotgunReloading)return;
 // Cancellation never transfers the uninserted shells. Each completed insertion
 // has already been saved to the bike's persistent inventory.
 if(ReloadRemaining<=0){bShotgunReloading=false;return;}
 const float Elapsed=ShotgunReloadDuration-ReloadRemaining;
 ShotgunReloadBlend=Ease((Elapsed-ShotgunReloadLead)/.2f)*Ease(ReloadRemaining/.3f);
 const float Loading=Elapsed-ShotgunReloadLead;
 TransferShotgunShells(FMath::Clamp(FMath::FloorToInt(Loading/ShellSeconds),0,ShotgunReloadShells));
 // Reach toward the belt in the body-held weapon frame before loading the port.
 const FVector Fetch(-24,-14,-37),Port(-18,1.3f,1.2f),HandOffset(-10,-1,-3);
 if(Loading<0){ShotgunReloadHand=FMath::Lerp(ShotgunReloadHand,Fetch+HandOffset,Ease((Elapsed-(ShotgunReloadLead-.25f))/.25f));return;}
 if(Loading>=ShotgunReloadShells*ShellSeconds){ShotgunReloadHand=FMath::Lerp(Port+FVector(6,0,0)+HandOffset,FVector(4,-3,-4),Ease((Loading-ShotgunReloadShells*ShellSeconds)/.35f));return;}
 ShotgunShellPhase=FMath::Fmod(Loading,ShellSeconds)/ShellSeconds;
 // Return the empty hand to the next shell before presenting it. Keeping the
 // cycle endpoints aligned avoids snapping from the loading port to the belt.
 if(ShotgunShellPhase<.25f){
  const FVector Previous=Loading<ShellSeconds?Fetch+HandOffset:Port+FVector(6,0,0)+HandOffset;
  ShotgunReloadHand=FMath::Lerp(Previous,Fetch+HandOffset,Ease(ShotgunShellPhase/.25f));return;
 }
 const FVector ShellPosition=FMath::Lerp(Fetch,Port,Ease((ShotgunShellPhase-.25f)/.6f))+FVector(6*Ease((ShotgunShellPhase-.85f)/.15f),0,0);
 ShotgunShell->SetRelativeLocation(ShellPosition);
 ShotgunReloadHand=ShellPosition+HandOffset;
}
void ABattleRider::UpdateShotgunVisual(){
 const bool Visible=CurrentWeapon==1&&bWeaponDrawn&&MeleeRemaining<=0;
 ShotgunPump->SetVisibility(Visible);ShotgunPump->SetRelativeLocation(FVector(-ShotgunPumpTravel,0,0));
 ShotgunShell->SetVisibility(Visible&&bShotgunReloading&&ShotgunShellPhase>=.25f&&ShotgunShellPhase<.97f,true);
}
