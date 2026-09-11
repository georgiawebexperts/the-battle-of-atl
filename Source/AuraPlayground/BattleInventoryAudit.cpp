#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
void ABattleMacController::TickInventoryAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());auto* Mode=Cast<ABattleParkMode>(UGameplayStatics::GetGameMode(this));if(!Bike||!Mode||!Mode->Pickups)return;
 InventoryClock+=Dt;auto* Z=Cast<ABattleZombie>(InventoryTarget.Get());
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleInventoryAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"crates\":%d,\"desired\":%d,\"kills\":%d,\"deaths\":%d}"),Pass?TEXT("true"):TEXT("false"),InventoryPhase,Reason,Mode->Pickups->WeaponCrates,Mode->Difficulty.WeaponCrates,Bike->EnemyKills,Bike->Deaths);UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);};
#define CHECK_INVENTORY(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Key=[&](FKey K){for(bool Down:{true,false})InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Target=[&](){auto* T=GetWorld()->SpawnActor<ABattleZombie>(Person->GetActorLocation()+FVector(250,0,0),FRotator::ZeroRotator);if(T){T->SetActorTickEnabled(false);T->GetCharacterMovement()->DisableMovement();T->Emergence=0;}InventoryTarget=T;return T;};
 auto Aim=[&](){FVector Eye;FRotator View;GetPlayerViewPoint(Eye,View);SetControlRotation((Z->GetActorLocation()-Eye).Rotation());};
 if(InventoryPhase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  CHECK_INVENTORY(Mode->Pickups->WeaponCrates==Mode->Difficulty.WeaponCrates,"Incomplete actual crate layout");
  CHECK_INVENTORY(Bike->Inventory.Num()==4&&Bike->Inventory[0].Owned&&!Bike->Inventory[1].Owned&&!Bike->Inventory[2].Owned,"Initial ownership incorrect");
  ABattleWeaponCrate* Crate=nullptr;for(TActorIterator<ABattleWeaponCrate> It(GetWorld());It;++It)if(It->WeaponSlot==1){Crate=*It;break;}
  CHECK_INVENTORY(Crate,"Missing shotgun crate");Bike->SetActorLocation(Crate->GetActorLocation()+FVector(0,0,33),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;InventoryPhase=1;InventoryClock=0;
 }
 else if(InventoryPhase==1&&InventoryClock>.3f){
  CHECK_INVENTORY(Bike->Inventory[1].Owned&&Bike->Inventory[1].Magazine==6&&Bike->Inventory[1].Reserve==12,"Bike crate did not grant shotgun supply");Bike->SetActorTransform(Bike->CheckpointTransform,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;CHECK_INVENTORY(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_INVENTORY(Person&&!Person->SelectWeapon(2)&&!Person->SelectWeapon(3),"Unowned weapon selectable");Key(EKeys::Two);InventoryPhase=2;InventoryClock=0;
 }
 else if(InventoryPhase==2&&InventoryClock>.3f){CHECK_INVENTORY(Person->CurrentWeapon==1&&Person->Ammo==6,"2 key selection failed");Z=Target();CHECK_INVENTORY(Z,"Shotgun target failed");InventoryPhase=3;InventoryClock=0;}
 else if(InventoryPhase==3){Aim();if(InventoryClock>.2f){CHECK_INVENTORY(Person->Fire(),"Shotgun failed to fire");CHECK_INVENTORY(Z->bDead&&Bike->EnemyKills==1&&Person->Ammo==5,"Close shotgun did not kill with one shell");CHECK_INVENTORY(!Person->Fire(),"Shotgun cooldown failed");Key(EKeys::R);InventoryPhase=4;InventoryClock=0;}}
 else if(InventoryPhase==4&&InventoryClock>.1f){CHECK_INVENTORY(Person->ReloadRemaining>0&&!Person->Fire()&&!Person->Melee(),"Reload did not block fire/melee");InventoryPhase=5;}
 else if(InventoryPhase==5&&InventoryClock>2.4f){
  CHECK_INVENTORY(Person->Ammo==6&&Bike->Inventory[1].Reserve==11,"Timed reload did not conserve shells");Z->Destroy();
  auto* Crate=GetWorld()->SpawnActorDeferred<ABattleWeaponCrate>(ABattleWeaponCrate::StaticClass(),FTransform(Person->GetActorLocation()+FVector(70,0,-20)),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);CHECK_INVENTORY(Crate,"SMG crate fixture failed");Crate->WeaponSlot=2;Crate->FinishSpawning(FTransform(Person->GetActorLocation()+FVector(70,0,-20)));InventoryPhase=6;InventoryClock=0;
 }
 else if(InventoryPhase==6&&InventoryClock>.2f){CHECK_INVENTORY(Bike->Inventory[2].Owned&&Bike->Inventory[2].Magazine==30&&Bike->Inventory[2].Reserve==60,"Foot crate failed");Key(EKeys::Three);Z=Target();CHECK_INVENTORY(Z,"SMG target failed");InventoryPhase=7;InventoryClock=0;}
 else if(InventoryPhase==7){Aim();if(InventoryClock>.3f){CHECK_INVENTORY(Person->CurrentWeapon==2&&Person->Fire(),"3 key/SMG fire failed");InventoryShots++;InventoryPhase=8;InventoryClock=0;}}
 else if(InventoryPhase==8){Aim();if(InventoryClock>.1f){CHECK_INVENTORY(Person->Fire(),"SMG cadence failed");InventoryShots++;InventoryClock=0;if(InventoryShots==8){CHECK_INVENTORY(Z->bDead&&Bike->EnemyKills==2&&Person->Ammo==22,"SMG damage/ammo failed");CHECK_INVENTORY(Person->MountBike(),"Remount failed");CHECK_INVENTORY(Bike->PistolAmmo==12&&Bike->Inventory[2].Magazine==22,"Remount corrupted magazines");InventoryPhase=9;InventoryClock=0;}}}
 else if(InventoryPhase==9&&InventoryClock>.2f){CHECK_INVENTORY(Bike->Dismount(),"Second dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_INVENTORY(Person&&Person->CurrentWeapon==2&&Person->Ammo==22,"Loadout lost on dismount");Person->TakeDamage(1000,FDamageEvent(),this,this);InventoryPhase=10;InventoryClock=0;}
 else if(InventoryPhase==10&&InventoryClock>2.3f){CHECK_INVENTORY(!Person&&!Bike->bParked&&Bike->Deaths==1&&Bike->Inventory[2].Magazine==22&&Bike->Inventory[1].Reserve==11&&Bike->PistolAmmo==12,"Checkpoint respawn lost inventory");Finish(true,TEXT("Crates, keys, shotgun/SMG fire, reload conservation and possession/death persistence pass"));return;}
 if(InventoryClock>15)Finish(false,TEXT("Inventory phase timed out"));
#undef CHECK_INVENTORY
#endif
}
