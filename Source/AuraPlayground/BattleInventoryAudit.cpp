#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "MaterialShared.h"
#include "SceneInterface.h"
#include "Camera/CameraComponent.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
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
 static TSet<FString> Captured;
 static bool ShotgunAimPressed=false,ShotgunPumpMoved=false;FString ReviewDir;const bool Review=FParse::Value(FCommandLine::Get(),TEXT("BattleInventoryReviewDir="),ReviewDir);
 auto Capture=[&](const FString& Name){FString Dir;if(!Captured.Contains(Name)&&FParse::Value(FCommandLine::Get(),TEXT("BattleInventoryReviewDir="),Dir)){Captured.Add(Name);
  if(Person&&Person->ShotgunMesh&&Name==TEXT("shotgun")){
   UStaticMesh* Mesh=Person->ShotgunMesh->GetStaticMesh();UE_LOG(LogTemp,Display,TEXT("ShotgunMaterialAudit: mesh=%s nanite=%d triangles=%d"),*GetNameSafe(Mesh),Mesh?Mesh->GetNaniteSettings().bEnabled:0,Mesh?Mesh->GetNumTriangles(0):0);
   for(int32 I=0;I<Person->ShotgunMesh->GetNumMaterials();I++){
    auto* Interface=Person->ShotgunMesh->GetMaterial(I);auto* Material=Interface?Interface->GetMaterial():nullptr;auto* Resource=Material?Material->GetMaterialResource(GetWorld()->Scene->GetShaderPlatform()):nullptr;
    UE_LOG(LogTemp,Display,TEXT("ShotgunMaterialAudit: slot=%d interface=%s resource=%d shader_complete=%d"),I,*GetPathNameSafe(Interface),Resource!=nullptr,Resource&&Resource->IsGameThreadShaderMapComplete());
   }
  }
  FScreenshotRequest::RequestScreenshot(Dir/(Name+TEXT(".png")),false,false);}};
 if(InventoryPhase==3&&InventoryClock>.6f)Capture(TEXT("shotgun"));
 if(Review&&InventoryPhase==3&&InventoryClock>1.f&&!ShotgunAimPressed){ShotgunAimPressed=true;InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::RightMouseButton,IE_Pressed,1.f,false,0));}
 if(InventoryPhase==3&&InventoryClock>1.7f)Capture(TEXT("shotgun-aimed"));
 if(InventoryPhase==5&&Person&&Person->ShotgunPumpTravel>6.f){ShotgunPumpMoved=true;Capture(TEXT("shotgun-pump"));}
 if(InventoryPhase==5&&InventoryClock>1.15f)Capture(TEXT("shotgun-reload"));
 if(InventoryPhase==5&&InventoryClock>1.35f)Capture(TEXT("shotgun-insert"));
 if(InventoryPhase==17&&InventoryClock>.9f)Capture(TEXT("shotgun-next-shell"));
 if(InventoryPhase==7&&InventoryClock>.15f)Capture(TEXT("smg"));
 if(InventoryPhase==12&&InventoryClock>.15f)Capture(TEXT("rifle"));
 if(InventoryPhase==13&&InventoryClock>.7f)Capture(TEXT("rifle-aimed"));
 if(InventoryPhase==14&&InventoryClock>.6f)Capture(TEXT("rifle-reload"));
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
  CHECK_INVENTORY(Bike->Inventory.Num()==BattleWeapons::Count&&Bike->Inventory[0].Owned&&!Bike->Inventory[1].Owned&&!Bike->Inventory[2].Owned&&!Bike->Inventory[3].Owned&&!Bike->Inventory[4].Owned,"Initial ownership incorrect");
  ABattleWeaponCrate* Crate=nullptr;for(TActorIterator<ABattleWeaponCrate> It(GetWorld());It;++It)if(It->WeaponSlot==1){Crate=*It;break;}
  CHECK_INVENTORY(Crate,"Missing shotgun crate");Bike->SetActorLocation(Crate->GetActorLocation()+FVector(0,0,33),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;InventoryPhase=1;InventoryClock=0;
 }
 else if(InventoryPhase==1&&InventoryClock>.3f){
  CHECK_INVENTORY(Bike->Inventory[1].Owned&&Bike->Inventory[1].Magazine==6&&Bike->Inventory[1].Reserve==12,"Bike crate did not grant shotgun supply");Bike->SetActorTransform(Bike->CheckpointTransform,false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;CHECK_INVENTORY(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_INVENTORY(Person&&!Person->SelectWeapon(2)&&!Person->SelectWeapon(3),"Unowned weapon selectable");Key(EKeys::Two);InventoryPhase=2;InventoryClock=0;
 }
 else if(InventoryPhase==2&&InventoryClock>.3f){CHECK_INVENTORY(Person->CurrentWeapon==1&&Person->Ammo==6,"2 key selection failed");Z=Target();CHECK_INVENTORY(Z,"Shotgun target failed");InventoryPhase=3;InventoryClock=0;}
 else if(InventoryPhase==3){Aim();if(InventoryClock>(Review?2.f:.4f)){CHECK_INVENTORY(Person->Fire(),"Shotgun failed to fire");CHECK_INVENTORY(Z->bDead&&Bike->EnemyKills==1&&Person->Ammo==5,"Close shotgun did not kill with one shell");CHECK_INVENTORY(Bike->ShotNotice==TEXT("ZOMBIE DOWN")&&Bike->ShotNoticeRemaining>0,"Shotgun kill confirmation missing");CHECK_INVENTORY(!Person->Fire(),"Shotgun cooldown failed");if(ShotgunAimPressed)InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::RightMouseButton,IE_Released,0.f,false,0));Key(EKeys::R);InventoryPhase=4;InventoryClock=0;}}
 else if(InventoryPhase==4&&InventoryClock>.1f){CHECK_INVENTORY(Person->ReloadRemaining>0&&!Person->Fire()&&!Person->Melee(),"Reload did not block fire/melee");InventoryPhase=5;}
 else if(InventoryPhase==5&&InventoryClock>2.4f){
  CHECK_INVENTORY(Bike->ShotNoticeRemaining==0,"Shot confirmation did not expire");
  CHECK_INVENTORY(ShotgunPumpMoved&&Person->ShotgunPumpTravel==0,"Shotgun pump did not cycle and return");
  CHECK_INVENTORY(Person->Ammo==6&&Bike->Inventory[1].Reserve==11,"Timed reload did not conserve shells");Z->Destroy();
  auto* Crate=GetWorld()->SpawnActorDeferred<ABattleWeaponCrate>(ABattleWeaponCrate::StaticClass(),FTransform(Person->GetActorLocation()+FVector(70,0,-20)),this,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);CHECK_INVENTORY(Crate,"SMG crate fixture failed");Crate->WeaponSlot=2;Crate->FinishSpawning(FTransform(Person->GetActorLocation()+FVector(70,0,-20)));InventoryPhase=6;InventoryClock=0;
 }
 else if(InventoryPhase==6&&InventoryClock>.2f){CHECK_INVENTORY(Bike->Inventory[2].Owned&&Bike->Inventory[2].Magazine==30&&Bike->Inventory[2].Reserve==60,"Foot crate failed");Key(EKeys::Three);Z=Target();CHECK_INVENTORY(Z,"SMG target failed");InventoryPhase=7;InventoryClock=0;}
 else if(InventoryPhase==7){Aim();if(InventoryClock>.4f){CHECK_INVENTORY(Person->CurrentWeapon==2&&Person->Fire(),"3 key/SMG fire failed");CHECK_INVENTORY(Bike->ShotNotice==TEXT("ZOMBIE HIT")&&Bike->ShotNoticeRemaining>0,"SMG hit confirmation missing");InventoryShots++;InventoryPhase=8;InventoryClock=0;}}
 else if(InventoryPhase==8){Aim();if(InventoryClock>.1f){CHECK_INVENTORY(Person->Fire(),"SMG cadence failed");InventoryShots++;InventoryClock=0;if(InventoryShots==8){CHECK_INVENTORY(Z->bDead&&Bike->EnemyKills==2&&Person->Ammo==22,"SMG damage/ammo failed");CHECK_INVENTORY(Person->MountBike(),"Remount failed");CHECK_INVENTORY(Bike->PistolAmmo==10&&Bike->Inventory[2].Magazine==22,"Remount corrupted magazines");InventoryPhase=9;InventoryClock=0;}}}
 else if(InventoryPhase==9&&InventoryClock>.2f){CHECK_INVENTORY(Bike->Dismount(),"Second dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_INVENTORY(Person&&Person->CurrentWeapon==2&&Person->Ammo==22,"Loadout lost on dismount");Person->TakeDamage(1000,FDamageEvent(),this,this);InventoryPhase=10;InventoryClock=0;}
 else if(InventoryPhase==10&&InventoryClock>2.3f){CHECK_INVENTORY(!Person&&!Bike->bParked&&Bike->Deaths==1&&Bike->Inventory[2].Magazine==22&&Bike->Inventory[1].Reserve==11&&Bike->PistolAmmo==10,"Checkpoint respawn lost inventory");ABattleWeaponCrate* Crate=nullptr;for(TActorIterator<ABattleWeaponCrate> It(GetWorld());It;++It)if(It->WeaponSlot==4){Crate=*It;break;}CHECK_INVENTORY(Crate,"Missing findable rifle crate");Bike->SetActorLocation(Crate->GetActorLocation()+FVector(0,0,33),false,nullptr,ETeleportType::TeleportPhysics);InventoryPhase=11;InventoryClock=0;}
 else if(InventoryPhase==11&&InventoryClock>.4f){CHECK_INVENTORY(Bike->Inventory[4].Owned&&Bike->Inventory[4].Magazine==30&&Bike->Inventory[4].Reserve==30,"Rifle crate supply incorrect");Bike->SetActorTransform(Bike->CheckpointTransform,false,nullptr,ETeleportType::TeleportPhysics);CHECK_INVENTORY(Bike->Dismount(),"Rifle dismount failed");Person=Cast<ABattleRider>(GetPawn());Key(EKeys::Five);InventoryPhase=12;InventoryClock=0;}
 else if(InventoryPhase==12&&InventoryClock>.4f){CHECK_INVENTORY(Person->CurrentWeapon==4&&Person->Ammo==30,"5 key rifle selection failed");InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::RightMouseButton,IE_Pressed,1.f,false,0));InventoryPhase=13;InventoryClock=0;}
 else if(InventoryPhase==13&&InventoryClock>1){CHECK_INVENTORY(Person->bAiming&&FMath::IsNearlyEqual(Person->Camera->FieldOfView,35.f,.1f),"Rifle zoom not applied");CHECK_INVENTORY(Person->Fire()&&Person->Ammo==29,"Rifle shot ammo incorrect");Key(EKeys::R);InventoryPhase=14;InventoryClock=0;}
 else if(InventoryPhase==14&&InventoryClock>1){CHECK_INVENTORY(Person->ReloadRemaining>0&&!Person->bAiming&&Person->Camera->FieldOfView>84,"Reload did not exit zoom");CHECK_INVENTORY(!Person->Fire(),"Reload allowed rifle fire");InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),EKeys::RightMouseButton,IE_Released,0.f,false,0));InventoryPhase=15;}
 else if(InventoryPhase==15&&InventoryClock>2.7f){CHECK_INVENTORY(Person->Ammo==30&&Bike->Inventory[4].Reserve==29,"Rifle reload conservation failed");CHECK_INVENTORY(Person->MountBike()&&Bike->Dismount(),"Rifle possession cycle failed");Person=Cast<ABattleRider>(GetPawn());CHECK_INVENTORY(Person&&Person->CurrentWeapon==4&&Person->Ammo==30,"Rifle loadout not retained");Key(EKeys::Two);InventoryPhase=16;InventoryClock=0;}
 else if(InventoryPhase==16&&InventoryClock>.5f){
  CHECK_INVENTORY(Person->CurrentWeapon==1,"Shotgun reselect failed");
  // Explicit low-magazine fixture: retain the reserve from the earlier reload.
  Person->Ammo=2;Person->SaveWeapon();Key(EKeys::R);InventoryPhase=17;InventoryClock=0;
 }
 else if(InventoryPhase==17&&InventoryClock>1.85f){
  CHECK_INVENTORY(Person->Ammo==4&&Bike->Inventory[1].Reserve==9&&Person->ReloadRemaining>0,"Shotgun did not insert two individual shells");
  Key(EKeys::G);InventoryPhase=18;InventoryClock=0;
 }
 else if(InventoryPhase==18&&InventoryClock>.7f){
  CHECK_INVENTORY(!Person->bWeaponDrawn&&Person->ReloadRemaining==0&&Person->Ammo==4&&Bike->Inventory[1].Reserve==9,"Holster did not preserve only inserted shells");
  Key(EKeys::G);InventoryPhase=19;InventoryClock=0;
 }
 else if(InventoryPhase==19&&InventoryClock>.4f){Key(EKeys::R);InventoryPhase=20;InventoryClock=0;}
 else if(InventoryPhase==20&&InventoryClock>2.2f){
  CHECK_INVENTORY(Person->Ammo==6&&Bike->Inventory[1].Reserve==7&&Person->ReloadRemaining==0,"Resumed shell reload conservation failed");
  CHECK_INVENTORY(Person->MountBike()&&Bike->Dismount(),"Shotgun possession cycle failed");Person=Cast<ABattleRider>(GetPawn());
  CHECK_INVENTORY(Person&&Person->CurrentWeapon==1&&Person->Ammo==6&&Bike->Inventory[1].Reserve==7,"Shotgun partial reload inventory not retained");
  Finish(true,TEXT("Crates, combat, death persistence, rifle zoom and shotgun individual-shell interruption/remount pass"));return;
 }
 if(InventoryClock>15)Finish(false,TEXT("Inventory phase timed out"));
#undef CHECK_INVENTORY
#endif
}
