#include "BattleMacController.h"
#include "BattleBike.h"
#include "BattleRider.h"
#include "BattlePickup.h"
#include "BattleDisc.h"
#include "BattleZombie.h"
#include "PiedmontTrafficDirector.h"
#include "PiedmontPedestrian.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
void ABattleMacController::TickDiscAudit(float Dt){
#if !UE_BUILD_SHIPPING
 if(GetWorld()->GetTimeSeconds()<5)return;
 auto* Person=Cast<ABattleRider>(GetPawn());auto* Bike=Person?Person->ParkedBike.Get():Cast<ABattleBike>(GetPawn());if(!Bike)return;
 DiscClock+=Dt;auto* First=Cast<ABattleZombie>(DiscFirst.Get());auto* Second=Cast<ABattleZombie>(DiscSecond.Get());auto* Projectile=Cast<ABattleDisc>(DiscProjectile.Get());
 auto Finish=[&](bool Pass,const TCHAR* Reason){UE_LOG(LogTemp,Display,TEXT("BattleDiscAudit: {\"passed\":%s,\"phase\":%d,\"reason\":\"%s\",\"kills\":%d,\"ammo\":%d}"),Pass?TEXT("true"):TEXT("false"),DiscPhase,Reason,Bike->EnemyKills,Person?Person->Ammo:0);UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);};
#define CHECK_DISC(C,R) if(!(C)){Finish(false,TEXT(R));return;}
 auto Key=[&](FKey K){for(bool Down:{true,false})InputKey(FInputKeyEventArgs(nullptr,IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),K,Down?IE_Pressed:IE_Released,Down?1.f:0.f,false,0));};
 auto Target=[&](FVector P){auto* Z=GetWorld()->SpawnActor<ABattleZombie>(P,FRotator::ZeroRotator);if(Z){Z->SetActorTickEnabled(false);Z->GetCharacterMovement()->DisableMovement();Z->WeaponDropChance=0;}return Z;};
 auto Wall=[&](FVector P){auto* A=GetWorld()->SpawnActor<AStaticMeshActor>(P,FRotator::ZeroRotator);if(A){auto* M=A->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));A->SetActorScale3D(FVector(.1,20,20));M->SetCollisionProfileName(TEXT("BlockAll"));}return A;};
 if(DiscPhase==0){
  for(TActorIterator<APiedmontTrafficDirector> It(GetWorld());It;++It)It->SetActorTickEnabled(false);
  for(TActorIterator<APiedmontPedestrian> It(GetWorld());It;++It)It->Destroy();
  ABattleWeaponCrate* Crate=nullptr;for(TActorIterator<ABattleWeaponCrate> It(GetWorld());It;++It)if(It->WeaponSlot==3){Crate=*It;break;}
  CHECK_DISC(Crate,"No real launcher crate");Bike->SetActorLocation(Crate->GetActorLocation()+FVector(0,0,33),false,nullptr,ETeleportType::TeleportPhysics);Bike->Ride->bForceNextFloorCheck=true;DiscPhase=1;DiscClock=0;
 }
 else if(DiscPhase==1&&DiscClock>.3f){
  CHECK_DISC(Bike->Inventory[3].Owned&&Bike->Inventory[3].Magazine==8&&Bike->Inventory[3].Reserve==8,"Launcher crate supply failed");Bike->SetActorTransform(Bike->CheckpointTransform,false,nullptr,ETeleportType::TeleportPhysics);CHECK_DISC(Bike->Dismount(),"Dismount failed");Person=Cast<ABattleRider>(GetPawn());CHECK_DISC(Person,"Missing FPS rider");
  // Floating controlled arena isolates projectile collision from terrain/crowds.
  Person->SetActorLocation(Person->GetActorLocation()+FVector(0,0,4000),false,nullptr,ETeleportType::TeleportPhysics);Person->GetCharacterMovement()->DisableMovement();SetControlRotation(FRotator::ZeroRotator);Key(EKeys::Four);
  DiscFirst=Target(Person->GetActorLocation()+FVector(400,12,40));DiscSecond=Target(Person->GetActorLocation()+FVector(700,12,40));DiscWall=Wall(Person->GetActorLocation()+FVector(1000,0,40));CHECK_DISC(DiscFirst.IsValid()&&DiscSecond.IsValid()&&DiscWall.IsValid(),"Arena setup failed");DiscPhase=2;DiscClock=0;
 }
 else if(DiscPhase==2&&DiscClock>.3f){
  CHECK_DISC(Person->CurrentWeapon==3&&Person->Fire()&&Person->Ammo==7,"4 key/launcher fire failed");TActorIterator<ABattleDisc> It(GetWorld());CHECK_DISC(It,"No moving disc");DiscProjectile=*It;DiscPhase=3;DiscClock=0;
 }
 else if(DiscPhase==3&&DiscClock>.65f){
  CHECK_DISC(Projectile&&Projectile->Hits==2&&Projectile->Bounces==1&&Projectile->Velocity.X<0,"Multi-target flight or wall reflection failed");CHECK_DISC(First->Health==35&&Second->Health==35&&Bike->EnemyKills==0,"Disc damage or duplicate-hit guard failed");SetControlRotation(FRotator::ZeroRotator);CHECK_DISC(Person->Fire()&&Person->Ammo==6,"Second disc failed");DiscPhase=4;DiscClock=0;
 }
 else if(DiscPhase==4&&DiscClock>.65f){
  CHECK_DISC(First->bDead&&Second->bDead&&Bike->EnemyKills==2&&Bike->Nitro==50,"Second disc kills/reward failed");CHECK_DISC(Projectile&&Projectile->Hits==2,"Returning disc hit a target twice");Key(EKeys::R);DiscPhase=5;DiscClock=0;
 }
 else if(DiscPhase==5&&DiscClock>1.7f){CHECK_DISC(Person->Ammo==8&&Bike->Inventory[3].Reserve==6,"Disc reload did not conserve ammo");DiscPhase=6;}
 else if(DiscPhase==6&&DiscClock>8.2f){
  TActorIterator<ABattleDisc> Remaining(GetWorld());CHECK_DISC(!Remaining,"Disc lifetime cleanup failed");
  if(First)First->Destroy();if(Second)Second->Destroy();if(DiscWall.IsValid())DiscWall->Destroy();
  Person->SetActorLocation(Bike->GetActorLocation()+FVector(0,145,0),false,nullptr,ETeleportType::TeleportPhysics);
  // Grounded zombie makes the normal drop placement code inspect real ground.
  FVector Spot=Bike->GetActorLocation()+FVector(400,0,0);FHitResult Floor;FCollisionQueryParams Q(SCENE_QUERY_STAT(DropFixture),false,Person);Q.AddIgnoredActor(Bike);
  CHECK_DISC(GetWorld()->LineTraceSingleByChannel(Floor,Spot+FVector(0,0,200),Spot-FVector(0,0,500),ECC_Visibility,Q),"No drop fixture floor");
  First=Target(Floor.ImpactPoint+FVector(0,0,88));CHECK_DISC(First,"Drop target failed");First->WeaponDropChance=1;First->TakeDamage(100,FDamageEvent(),this,Person);
  CHECK_DISC(First->bDead&&IsValid(First->DroppedWeapon)&&First->DroppedWeapon->GetLifeSpan()>59,"Enemy drop placement/expiry failed");auto* Drop=First->DroppedWeapon.Get();const int32 Slot=Drop->WeaponSlot;const int32 Before=Bike->Inventory[Slot].Reserve;
  Person->SetActorLocation(Drop->GetActorLocation()+FVector(0,0,30),false,nullptr,ETeleportType::TeleportPhysics);CHECK_DISC(Drop->TryCollect(Person)&&Bike->Inventory[Slot].Owned&&Bike->Inventory[Slot].Reserve>Before&&!Drop->TryCollect(Person),"Enemy drop pickup/duplicate guard failed");
  Finish(true,TEXT("Launcher crate/key, two-target flight, ricochet, one hit per target, reload, cleanup and enemy drop pass"));return;
 }
 if(DiscClock>15)Finish(false,TEXT("Disc phase timed out"));
#undef CHECK_DISC
#endif
}
