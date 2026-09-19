#include "BattleInventory.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

bool ABattleBike::GiveWeapon(int32 Slot,int32 Rounds){
 if(Slot<0||Slot>=BattleWeapons::Count||Rounds<=0||Inventory.Num()!=BattleWeapons::Count||RiderHealth<=0||RespawnRemaining>0||UGameplayStatics::IsGamePaused(this))return false;
 const auto* Mode=Cast<ABattleLabMode>(UGameplayStatics::GetGameMode(this));if(Mode&&(Mode->bRunEnded||Mode->StartCountdown>0))return false;
 auto& Item=Inventory[Slot];const bool New=!Item.Owned;const int32 Old=Item.Reserve;
 if(New){Item.Owned=true;Item.Magazine=FMath::Min(Rounds,BattleWeapons::Capacity(Slot));Rounds-=Item.Magazine;}
 Item.Reserve=FMath::Min(BattleWeapons::ReserveLimit(Slot),Item.Reserve+Rounds);
 // Elliott: "the gun never changes even though i pick up a machine gun or a
 // rifle". A new weapon is now the one in the rider's hand; picking up more
 // ammunition for something already owned must not yank the gun away.
 if(New)ActiveWeapon=Slot;
 return New||Item.Reserve>Old;
}
bool ABattleBike::SelectRidingWeapon(int32 Slot){
 if(Slot<0||Slot>=BattleWeapons::Count||!Inventory.IsValidIndex(Slot)||!Inventory[Slot].Owned)return false;
 ActiveWeapon=Slot;UpdateRidingWeaponModel();return true;
}
void ABattleBike::UpdateRidingWeaponModel(){
 const bool Visible=!bParked&&GunHold>0;
 // Slot 2 is the SMG; it carried audio and inventory for two days before it had
 // a mesh of its own, and borrowed the rifle until the CC0 Quaternius SMG was
 // imported on 2026-09-19.
 Pistol->SetVisibility(Visible&&ActiveWeapon==0);
 RifleProp->SetVisibility(Visible&&ActiveWeapon==4);
 SMGProp->SetVisibility(Visible&&ActiveWeapon==2);
 ShotgunProp->SetVisibility(Visible&&ActiveWeapon==1);
}
void ABattleRider::SaveWeapon(){
 if(!ParkedBike||!ParkedBike->Inventory.IsValidIndex(CurrentWeapon))return;
 ParkedBike->Inventory[CurrentWeapon].Magazine=Ammo;ParkedBike->LastFootWeapon=CurrentWeapon;
 if(CurrentWeapon==0)ParkedBike->PistolAmmo=Ammo;
}
void ABattleRider::RestoreLoadout(){
 if(!ParkedBike)return;ParkedBike->Inventory[0].Magazine=ParkedBike->PistolAmmo;
 CurrentWeapon=ParkedBike->LastFootWeapon;if(!ParkedBike->Inventory.IsValidIndex(CurrentWeapon)||!ParkedBike->Inventory[CurrentWeapon].Owned)CurrentWeapon=0;
 Ammo=ParkedBike->Inventory[CurrentWeapon].Magazine;
}
bool ABattleRider::SelectWeapon(int32 Slot){
 if(!CanUseWeapon()||MeleeRemaining>0||!ParkedBike||Slot<0||Slot>=BattleWeapons::Count||!ParkedBike->Inventory[Slot].Owned)return false;
 if(Slot==CurrentWeapon){if(!bWeaponDrawn)return ToggleDrawWeapon();return true;}SaveWeapon();ReloadRemaining=0;CurrentWeapon=Slot;Ammo=ParkedBike->Inventory[Slot].Magazine;ParkedBike->LastFootWeapon=Slot;ShotCooldown=FMath::Max(ShotCooldown,.2f);bWeaponDrawn=true;DrawRemaining=.3f;UpdateWeaponModel();return true;
}
FString ABattleRider::ReserveLabel() const{return ParkedBike?FString::FromInt(ParkedBike->Inventory[CurrentWeapon].Reserve):TEXT("0");}
void ABattleRider::Reload(){
 if(!CanUseWeapon()||!bWeaponDrawn||DrawRemaining>0||MeleeRemaining>0||ReloadRemaining>0||Ammo>=BattleWeapons::Capacity(CurrentWeapon))return;
 if(!ParkedBike||ParkedBike->Inventory[CurrentWeapon].Reserve<=0)return;
 if(CurrentWeapon==1){StartShotgunReload();return;}
 ReloadRemaining=BattleWeapons::ReloadSeconds(CurrentWeapon);
}
void ABattleRider::FinishReload(){
 if(CurrentWeapon==1&&bShotgunReloading){FinishShotgunReload();return;}
 if(ParkedBike){auto& Item=ParkedBike->Inventory[CurrentWeapon];const int32 Add=FMath::Min(BattleWeapons::Capacity(CurrentWeapon)-Ammo,Item.Reserve);Ammo+=Add;Item.Reserve-=Add;}
 SaveWeapon();
}
void ABattleRider::BuildLongGun(){
 LongGun=CreateDefaultSubobject<USceneComponent>(TEXT("LongGun"));LongGun->SetupAttachment(Camera);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
 auto Part=[&](const TCHAR* Name,FVector P,FVector Scale,bool Barrel){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(Name);M->SetupAttachment(LongGun);M->SetStaticMesh(Barrel?Cylinder.Object:Cube.Object);M->SetRelativeLocation(P);M->SetRelativeScale3D(Scale);if(Barrel)M->SetRelativeRotation(FRotator(90,0,0));M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCanEverAffectNavigation(false);M->SetOnlyOwnerSee(true);M->SetCastShadow(false);M->SetVisibility(false);LongGunParts.Add(M);};
 Part(TEXT("Receiver"),FVector(0,0,0),FVector(.24,.065,.085),false);
 Part(TEXT("Barrel"),FVector(24,0,2),FVector(.035,.035,.35),true);
 Part(TEXT("Stock"),FVector(-22,0,-2),FVector(.22,.045,.085),false);
 Part(TEXT("Grip"),FVector(-5,0,-8),FVector(.045,.05,.13),false);
 Part(TEXT("MagazineOrPump"),FVector(12,0,-4),FVector(.17,.07,.065),false);
 Part(TEXT("DiscHopper"),FVector(8,0,7),FVector(.32,.32,.05),true);
 LongGunParts[5]->SetRelativeRotation(FRotator::ZeroRotator);
 ShotgunMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShotgunModel"));ShotgunMesh->SetupAttachment(LongGun);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Shotgun(TEXT("/Game/BattleForTheA/Weapons/ShotgunParts/ShotgunParts/StaticMeshes/ShotgunBody.ShotgunBody"));
 ShotgunMesh->SetStaticMesh(Shotgun.Object);ShotgunMesh->SetRelativeScale3D(FVector(.8f));ShotgunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);ShotgunMesh->SetCanEverAffectNavigation(false);ShotgunMesh->SetOnlyOwnerSee(true);ShotgunMesh->SetCastShadow(false);ShotgunMesh->SetVisibility(false);
 ShotgunPump=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShotgunPump"));ShotgunPump->SetupAttachment(LongGun);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Pump(TEXT("/Game/BattleForTheA/Weapons/ShotgunParts/ShotgunParts/StaticMeshes/ShotgunPump.ShotgunPump"));
 ShotgunPump->SetStaticMesh(Pump.Object);ShotgunPump->SetRelativeScale3D(FVector(.8f));ShotgunPump->SetCollisionEnabled(ECollisionEnabled::NoCollision);ShotgunPump->SetCanEverAffectNavigation(false);ShotgunPump->SetOnlyOwnerSee(true);ShotgunPump->SetCastShadow(false);ShotgunPump->SetVisibility(false);
 ShotgunShell=CreateDefaultSubobject<USceneComponent>(TEXT("ShotgunLoadingShell"));ShotgunShell->SetupAttachment(LongGun);
 auto ShellPart=[&](const TCHAR* Name,float X,float Length,const TCHAR* Material){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(Name);M->SetupAttachment(ShotgunShell);M->SetStaticMesh(Cylinder.Object);M->SetRelativeRotation(FRotator(90,0,0));M->SetRelativeLocation(FVector(X,0,0));M->SetRelativeScale3D(FVector(.018f,.018f,Length/100.f));M->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,Material));M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCanEverAffectNavigation(false);M->SetOnlyOwnerSee(true);M->SetCastShadow(false);M->SetVisibility(false);};
 ShellPart(TEXT("ShotgunShellHull"),0,4.8f,TEXT("/Game/BattleForTheA/Weapons/ShotgunParts/M_ShellRed.M_ShellRed"));
 ShellPart(TEXT("ShotgunShellHead"),-2.65f,.5f,TEXT("/Game/BattleForTheA/Weapons/ShotgunParts/M_ShellBrass.M_ShellBrass"));
 RifleMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RifleModel"));RifleMesh->SetupAttachment(LongGun);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Rifle(TEXT("/Game/BattleForTheA/Weapons/Rifle/Rifle/StaticMeshes/Rifle.Rifle"));RifleMesh->SetStaticMesh(Rifle.Object);RifleMesh->SetRelativeRotation(FRotator(0,-90,0));RifleMesh->SetRelativeScale3D(FVector(.5));RifleMesh->SetRelativeLocation(FVector(-10,0,-10));RifleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);RifleMesh->SetOnlyOwnerSee(true);RifleMesh->SetCastShadow(false);RifleMesh->SetVisibility(false);
 SMGMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SMGModel"));SMGMesh->SetupAttachment(LongGun);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> SMG(TEXT("/Game/BattleForTheA/Weapons/SMG/SMG/StaticMeshes/SMG.SMG"));SMGMesh->SetStaticMesh(SMG.Object);SMGMesh->SetRelativeRotation(FRotator(0,-90,0));SMGMesh->SetRelativeScale3D(FVector(.62));SMGMesh->SetRelativeLocation(FVector(-6,0,-10));SMGMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);SMGMesh->SetOnlyOwnerSee(true);SMGMesh->SetCastShadow(false);SMGMesh->SetVisibility(false);

}
void ABattleRider::UpdateWeaponModel(){
 const bool Visible=CurrentWeapon>0&&bWeaponDrawn&&MeleeRemaining<=0;
 // Firearms follow the body; camera lag must not move them after hand placement.
 USceneComponent* Parent=CurrentWeapon==3?static_cast<USceneComponent*>(Camera.Get()):GetRootComponent();
 if(LongGun->GetAttachParent()!=Parent)LongGun->AttachToComponent(Parent,FAttachmentTransformRules::KeepWorldTransform);
 LongGun->SetRelativeLocation(Weapon->GetRelativeLocation());LongGun->SetRelativeRotation(Weapon->GetRelativeRotation()-GunRestRotation);
 LongGun->SetRelativeScale3D(FVector(1));LongGun->SetVisibility(Visible,true);
 if(CurrentWeapon>0)Weapon->SetVisibility(false);
 RifleMesh->SetVisibility(Visible&&CurrentWeapon==4);
 SMGMesh->SetVisibility(Visible&&CurrentWeapon==2);
 ShotgunMesh->SetVisibility(Visible&&CurrentWeapon==1);
 UpdateShotgunVisual();
 if(LongGunParts.Num()==6){
  LongGunParts[5]->SetVisibility(Visible&&CurrentWeapon==3);LongGunParts[1]->SetRelativeScale3D(CurrentWeapon==3?FVector(.14,.035,.2):FVector(.035,.035,.35));LongGunParts[4]->SetRelativeLocation(CurrentWeapon==2?FVector(3,0,-10):FVector(12,0,-4));LongGunParts[4]->SetRelativeScale3D(CurrentWeapon==2?FVector(.06,.05,.2):FVector(.17,.07,.065));}
 // The three authored meshes stand in for the primitive long-gun parts.
 if(CurrentWeapon==4||CurrentWeapon==2||CurrentWeapon==1)for(auto Part:LongGunParts)Part->SetVisibility(false);
}
